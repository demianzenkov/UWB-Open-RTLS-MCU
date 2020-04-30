#include "une_twr.h"

#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_spi.h"
#include "deca_port.h"

#include "tsk_usb.hpp"
#include "tsk_udp_client.hpp"

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 µs and 1 µs = 499.2 * 128 dtu. */
#define UUS_TO_DWT_TIME 65536
/* Delay between frames, in UWB microseconds. See NOTE 4. */
/* This is the delay from Frame RX timestamp to TX reply timestamp used for calculating/setting the DW1000's delayed TX function. This includes the
* frame length of approximately 2.46 ms with above configuration. */
#define POLL_RX_TO_RESP_TX_DLY_UUS 2750
/* This is the delay from the end of the frame transmission to the enable of the receiver, as programmed for the DW1000's wait for response feature. */
#define RESP_TX_TO_FINAL_RX_DLY_UUS 500		//500
/* Receive final timeout. See NOTE 5 below. */
#define FINAL_RX_TIMEOUT_UUS 3700

/* Speed of light in air, in metres per second. */
#define SPEED_OF_LIGHT 299702547


extern TskUdpClient tskUdpClient;
extern TskUSB tskUSB;


UNE_TWR::UNE_TWR(DWM1000 * dwm)
{
  this->dwm = dwm;
}


S08 UNE_TWR::twrResponderLoop()
{
  /* Clear reception timeout to start next ranging process. */
  dwt_setrxtimeout(0);
  
  //dbg_PutString("Trying to receive Poll message...\n\r");
  /* Activate reception immediately. */
  dwt_rxenable(DWT_START_RX_IMMEDIATE);
  
  /* CHECK IF POLL RECEIVED */
  /* Poll for reception of a frame or error/timeout. See NOTE 8 below. */
  while (!((dwm->status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
  { };
  
  if (dwm->status_reg & SYS_STATUS_RXFCG)
  {
    //dbg_PutString("Received Poll.\n\r");
 
    /* Clear good RX frame event in the DW1000 status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);
    
    /* READ POLL MESSAGE */
    /* A frame has been received, read it into the local buffer. */
    dwm->frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
    if (dwm->frame_len <= FRAME_LEN_MAX)
    {
      dwt_readrxdata(dwm->rx_buffer, dwm->frame_len, 0);
    }
    
    /* Check that the frame is a poll sent by tag in "TWR-Initiator" mode.
    * As the sequence number field of the frame is not relevant, it is cleared to simplify the validation of the frame. */
    if (memcmp(dwm->rx_buffer, poll_msg, DW_MSG_PREAMBLE_LEN) == 0)
    {
      //dbg_PutString("Correct Poll frame.\n\r");
      /* POLL MESSAGE IS CORRECT */
      uint32 resp_tx_time;
      int ret;
      
      /* Retrieve poll reception timestamp. */
      poll_rx_ts = DWM1000::getTimestampU64(DWM1000::RX_TS);
      
      /* Set send time for response. See NOTE 9 below. */
      resp_tx_time = (poll_rx_ts + (POLL_RX_TO_RESP_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
      dwt_setdelayedtrxtime(resp_tx_time);
      
      /* Set expected delay and timeout for final message reception. See NOTE 4 and 5 below. */
      dwt_setrxaftertxdelay(RESP_TX_TO_FINAL_RX_DLY_UUS);
      dwt_setrxtimeout(FINAL_RX_TIMEOUT_UUS);
      
      /* WRITE AND SEND RESPONSE MESSAGE. See NOTE 10 below.*/
//      resp_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
      dwt_writetxdata(sizeof(resp_msg), resp_msg, 0); /* Zero offset in TX buffer. */
      dwt_writetxfctrl(sizeof(resp_msg), 0, 1); /* Zero offset in TX buffer, ranging. */
      ret = dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);
      
      /* If dwt_starttx() returns an error, abandon this ranging exchange and proceed to the next one. See NOTE 11 below. */
      if (ret == DWT_ERROR)
      {
	return RC_ERR_HW;
      }
      
      //dbg_PutString("Response message sent.\n\r");
      //dbg_PutString("Waiting for Final message.\n\r");
      /* Poll for reception of expected "final" frame or error/timeout. See NOTE 8 below. */
      while (!((dwm->status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
      { };
      
      /* Increment frame sequence number after transmission of the response message (modulo 256). */
      resp_frame_seq_nb++;
      
      if (dwm->status_reg & SYS_STATUS_RXFCG)
      {
	/* FINAL MESSAGE RECEIVED */
	//dbg_PutString("Final message received.\n\r");
	
	/* Clear good RX frame event and TX frame sent in the DW1000 status register. */
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);
	
	/* A frame has been received, read it into the local buffer. */
	dwm->frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
	if (dwm->frame_len <= FRAME_LEN_MAX)
	{
	  dwt_readrxdata(dwm->rx_buffer, dwm->frame_len, 0);
	}
	
	/* Check that the frame is a final message sent by "DS TWR initiator" example.
	* As the sequence number field of the frame is not used in this example, it can be zeroed to ease the validation of the frame. */
//	rx_buffer[ALL_MSG_SN_IDX] = 0;
	if (memcmp(dwm->rx_buffer, final_msg, DW_MSG_PREAMBLE_LEN) == 0)
	{
	  
	  //dbg_PutString("Final message correct! Computing distance...\n\r");
	  
	  U64 poll_tx_ts=0, resp_rx_ts=0, final_tx_ts=0;
	  double Ra, Rb, Da, Db;
	  S64 tof_dtu;
	  
	  /* Retrieve response transmission and final reception timestamps. */
	  resp_tx_ts = DWM1000::getTimestampU64(DWM1000::TX_TS);
	  final_rx_ts =  DWM1000::getTimestampU64(DWM1000::RX_TS);
	  
	  /* Get timestamps embedded in the final message. */
	  final_msg_get_ts(&dwm->rx_buffer[FINAL_MSG_POLL_TX_TS_IDX], &poll_tx_ts);
	  final_msg_get_ts(&dwm->rx_buffer[FINAL_MSG_RESP_RX_TS_IDX], &resp_rx_ts);
	  final_msg_get_ts(&dwm->rx_buffer[FINAL_MSG_FINAL_TX_TS_IDX], &final_tx_ts);  
	  /* Compute time of flight. 32-bit subtractions give correct answers even if clock has wrapped. See NOTE 12 below. */
	  
	  Ra = (double)(resp_rx_ts - poll_tx_ts);
	  Rb = (double)(final_rx_ts - resp_tx_ts);
	  Da = (double)(final_tx_ts - resp_rx_ts);
	  Db = (double)(resp_tx_ts - poll_rx_ts);
	  tof_dtu = (S64)((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db));
	  
	  tof = tof_dtu * DWT_TIME_UNITS;
	  distance = tof * SPEED_OF_LIGHT;
	  
	  memcpy(&upd_ranging_msg[6], &distance, sizeof(distance));
	  memcpy(tskUSB.tx_queue.data, upd_ranging_msg, sizeof(upd_ranging_msg));
	  tskUSB.tx_queue.len = sizeof(upd_ranging_msg);
	  xQueueSend(tskUSB.xQueueUSBTx, (void *)&tskUSB.tx_queue, (TickType_t)0);
	  xQueueSend(tskUdpClient.xQueueUdpTx, (void *)&tskUSB.tx_queue, (TickType_t)0);
	  
	  return RC_ERR_NONE;
	  //sprintf(dist_str, "Anchor ID=%i, Lat=%2.6f, Lon=%2.6f, Distance=%3.2f m \n\n\r", anchor_id, anchor_lat, anchor_lon, distance);
	  //sprintf(dist_str, "Distance to initiator: %3.2f m \n\r", distance);
	  //dbg_PutString(dist_str);
	}
      }
      else
      {
	
//	dbg_PutString("Final message RX error/timeout event\n\n\r");
	/* Clear RX error/timeout events in the DW1000 status register. */
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
	/* Reset RX to properly reinitialise LDE operation. */
	dwt_rxreset();
	memset(dwm->rx_buffer, 0, sizeof(dwm->rx_buffer));
	return RC_ERR_TIMEOUT;
      }
    }
  }
  else	/* Error or Timeout of RX */
  {
    //dbg_PutString("Poll message RX error/timeout event\n\r");
    /* Clear RX error/timeout events in the DW1000 status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
    
    /* Reset RX to properly reinitialise LDE operation. */
    dwt_rxreset();
    memset(dwm->rx_buffer, 0, sizeof(dwm->rx_buffer));
    return RC_ERR_TIMEOUT;
  }
}

/*! ------------------------------------------------------------------------------------------------------------------
* @fn final_msg_get_ts()
*
* @brief Read a given timestamp value from the final message. In the timestamp fields of the final message, the least
*        significant byte is at the lower address.
*
* @param  ts_field  pointer on the first byte of the timestamp field to read
*         ts  timestamp value
*
* @return none
*/
void UNE_TWR::final_msg_get_ts(const uint8 *ts_field, uint64 *ts)
{
  int i;
  *ts = 0;
  for (i = 0; i < MSG_TS_LEN; i++)
  {
    *ts += (U64)ts_field[i] << (i * 8);
  }
}
