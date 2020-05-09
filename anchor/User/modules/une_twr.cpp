#include "une_twr.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_spi.h"
#include "deca_port.h"
#include "tsk_usb.h"
#include "tsk_udp_client.h"
#include "monitoring_pb.h"
#include "settings.h"
#include "wake.h"
#include "tsk_une.h"

extern TskUdpClient tskUdpClient;
extern TskUNE tskUNE;
extern TskUSB tskUSB;
extern MonitoringPB pb_monitoring;
extern DeviceSettings settings;

UNE_TWR::UNE_TWR(DWM1000 * dwm)
{
  this->dwm = dwm;
}

void UNE_TWR::initDWM()
{
   /* Set expected response's delay and timeout. See NOTE 4, 5 and 6 below.
  * As this example only handles one incoming frame with always the same delay and timeout, those values can be set here once for all. */
  dwt_setrxaftertxdelay(POLL_TX_TO_RESP_RX_DLY_UUS);
  dwt_setrxtimeout(RESP_RX_TIMEOUT_UUS);
  dwt_setpreambledetecttimeout(PRE_TIMEOUT);
  
}


/* Loop for anchor in two-way-ranging mode */
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
    if ((memcmp(dwm->rx_buffer, poll_msg, DW_MSG_PREAMBLE_LEN) == 0) &&
	(dwm->rx_buffer[MSG_AN_IDX] == settings.pb_settings.message.NodeID))
    {
      /* POLL MESSAGE IS CORRECT */
      tag_id =  dwm->rx_buffer[MSG_TAG_IDX];
	
      resp_msg[MSG_TAG_IDX] = tag_id;
      resp_msg[MSG_AN_IDX] = dwm->rx_buffer[MSG_AN_IDX];
      
      resp_msg[MSG_POLL_H_IDX] = dwm->rx_buffer[MSG_POLL_H_IDX];
      resp_msg[MSG_POLL_L_IDX] = dwm->rx_buffer[MSG_POLL_L_IDX];
      
      resp_msg[MSG_RESP_H_IDX] = (resp_frame_seq_nb >> 8) & 0xFF;
      resp_msg[MSG_RESP_L_IDX] = resp_frame_seq_nb & 0xFF;
	
      resp_frame_seq_nb++;
       
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
      dwt_writetxdata(RESP_MSG_SIZE, resp_msg, 0); /* Zero offset in TX buffer. */
      dwt_writetxfctrl(RESP_MSG_SIZE, 0, 1); /* Zero offset in TX buffer, ranging. */
      ret = dwt_starttx(DWT_START_TX_DELAYED | DWT_RESPONSE_EXPECTED);
      
      /* If dwt_starttx() returns an error, abandon this ranging exchange and proceed to the next one. See NOTE 11 below. */
      if (ret == DWT_ERROR)
      {
	return RC_ERR_HW;
      }
      
      /* Poll for reception of expected "final" frame or error/timeout. See NOTE 8 below. */
      while (!((dwm->status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
      { };
      
      /* Increment frame sequence number after transmission of the response message (modulo 256). */
      
      if (dwm->status_reg & SYS_STATUS_RXFCG)
      {
	/* FINAL RECEIVED */
	/* Clear good RX frame event and TX frame sent in the DW1000 status register. */
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);
	
	/* A frame has been received, read it into the local buffer. */
	dwm->frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
	if (dwm->frame_len <= FRAME_LEN_MAX)
	{
	  dwt_readrxdata(dwm->rx_buffer, dwm->frame_len, 0);
	}
	
	/* Check FINAL frame is valid */
	if ((memcmp(dwm->rx_buffer, final_msg, DW_MSG_PREAMBLE_LEN) == 0) &&
	    (dwm->rx_buffer[MSG_TAG_IDX] == resp_msg[MSG_TAG_IDX]) &&
	      (dwm->rx_buffer[MSG_AN_IDX] == resp_msg[MSG_AN_IDX]) &&
	    	(dwm->rx_buffer[MSG_RESP_H_IDX] == resp_msg[MSG_RESP_H_IDX]) && 
	      	(dwm->rx_buffer[MSG_RESP_L_IDX] == resp_msg[MSG_RESP_L_IDX]) )
	{
	  /* FINAL IS CORRECT */
	  U64 poll_tx_ts=0, resp_rx_ts=0, final_tx_ts=0;
	  double Ra, Rb, Da, Db;
	  S64 tof_dtu;
	  
	  poll_frame_seq_nb = (dwm->rx_buffer[MSG_POLL_H_IDX] << 8) + dwm->rx_buffer[MSG_POLL_L_IDX];
	  final_frame_seq_nb = (dwm->rx_buffer[MSG_FINAL_H_IDX] << 8) + dwm->rx_buffer[MSG_FINAL_L_IDX];
	  
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
	  
	  pb_monitoring.clearMessage();
	  
	  pb_monitoring.message.has_TWR = true;
	  pb_monitoring.message.TWR.NodeID = settings.pb_settings.message.NodeID;
	  pb_monitoring.message.TWR.InitiatorID = dwm->rx_buffer[MSG_TAG_IDX];
	  pb_monitoring.message.TWR.Distance = distance;
	  pb_monitoring.message.TWR.NodeTicks = xTaskGetTickCount();
	  pb_monitoring.message.TWR.PollNN = poll_frame_seq_nb;
	  pb_monitoring.message.TWR.ResponseNN = resp_frame_seq_nb;
	  pb_monitoring.message.TWR.FinalNN = final_frame_seq_nb;
	  
	  U16 msg_len;
	  U16 wake_buf_len;
	  /* Encode message to wake.dbuf for encoding to wake */
	  pb_monitoring.encode(&pb_monitoring.message, pb_monitoring.temp_buf, &msg_len);
	  
	  tskUNE.wake.prepareBuf(pb_monitoring.temp_buf, 
				 msg_len, 
				 CMD_TWR_RANGING, 
				 pb_monitoring.temp_buf,
				 &wake_buf_len);
	  
	  tskUdpClient.transmit(pb_monitoring.temp_buf, wake_buf_len-1);
	  tskUSB.transmit(pb_monitoring.temp_buf, wake_buf_len-1);
	  	  
	  return RC_ERR_NONE;
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
  return RC_ERR_TIMEOUT;
}

/* Loop for tag in two-way-ranging mode */
S08 UNE_TWR::twrInitiatorLoop(U08 an_id)
{
  U08 node_id = settings.pb_settings.message.NodeID;
  /* Write frame data to DW1000 and prepare transmission. See NOTE 8 below. */
  poll_msg[MSG_AN_IDX] = an_id;
  poll_msg[MSG_TAG_IDX] = node_id;
  poll_msg[MSG_POLL_H_IDX] = (poll_frame_seq_nb >> 8) & 0xFF;
  poll_msg[MSG_POLL_L_IDX] = poll_frame_seq_nb & 0xFF;
  
  dwt_writetxdata(POLL_MSG_SIZE, poll_msg, 0); /* Zero offset in TX buffer. */
  dwt_writetxfctrl(POLL_MSG_SIZE, 0, 1); /* Zero offset in TX buffer, ranging. */
  
  /* Start transmission, indicating that a response is expected so that reception is enabled automatically after the frame is sent and the delay
  * set by dwt_setrxaftertxdelay() has elapsed. */
  dwt_starttx(DWT_START_TX_IMMEDIATE | DWT_RESPONSE_EXPECTED);
  
  /* WAIT FOR ANSWER (REQUEST FROM RESPONDER) AFTER POLL */
  /* We assume that the transmission is achieved correctly, poll for reception of a frame or error/timeout. See NOTE 9 below. */
  while (!((dwm->status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
  { };
  
  /* Increment frame sequence number after transmission of the poll message (modulo 256). */
  poll_frame_seq_nb++;
  
  if (dwm->status_reg & SYS_STATUS_RXFCG)
  {
    /* RESPONSE RECEIVED */
    /* Clear good RX frame event and TX frame sent in the DW1000 status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);
    
    /* A frame has been received, read it into the local buffer. */
    dwm->frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
    if (dwm->frame_len <= FRAME_LEN_MAX)
    {
      dwt_readrxdata(dwm->rx_buffer, dwm->frame_len, 0);
    }
    
    /* Check that the frame is valid */
    if ((memcmp(dwm->rx_buffer, resp_msg, DW_MSG_PREAMBLE_LEN) == 0) &&
	(dwm->rx_buffer[MSG_POLL_H_IDX] == poll_msg[MSG_POLL_H_IDX]) && 
	 (dwm->rx_buffer[MSG_POLL_L_IDX] == poll_msg[MSG_POLL_L_IDX]) )
    {
      /* RESPONSE IS CORRECT */   
      uint32 final_tx_time;
      int ret;
      
      final_msg[MSG_TAG_IDX] = node_id;
      final_msg[MSG_AN_IDX] = poll_msg[MSG_AN_IDX];
      
      final_msg[MSG_POLL_H_IDX] = dwm->rx_buffer[MSG_POLL_H_IDX];
      final_msg[MSG_POLL_L_IDX] = dwm->rx_buffer[MSG_POLL_L_IDX];
      
      final_msg[MSG_RESP_H_IDX] = dwm->rx_buffer[MSG_RESP_H_IDX];
      final_msg[MSG_RESP_L_IDX] = dwm->rx_buffer[MSG_RESP_L_IDX];
      
      final_msg[MSG_FINAL_H_IDX] = (final_frame_seq_nb >> 8) & 0xFF;
      final_msg[MSG_FINAL_L_IDX] = final_frame_seq_nb & 0xFF;
      
      final_frame_seq_nb++;
      
      /* Retrieve poll transmission and response reception timestamp. */
      poll_tx_ts = DWM1000::getTimestampU64(DWM1000::TX_TS);
      resp_rx_ts = DWM1000::getTimestampU64(DWM1000::RX_TS);
      
      /* Compute final message transmission time. See NOTE 10 below. */
      final_tx_time = (resp_rx_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
      dwt_setdelayedtrxtime(final_tx_time);
      
      /* Final TX timestamp is the transmission time we programmed plus the TX antenna delay. */
      final_tx_ts = (((uint64)(final_tx_time & 0xFFFFFFFEUL)) << 8) + settings.pb_settings.message.DWTxAntDelay;
      
      /* Write all timestamps in the final message. See NOTE 11 below. */
      
      final_msg_set_ts(&final_msg[FINAL_MSG_POLL_TX_TS_IDX], poll_tx_ts);
      final_msg_set_ts(&final_msg[FINAL_MSG_RESP_RX_TS_IDX], resp_rx_ts);
      final_msg_set_ts(&final_msg[FINAL_MSG_FINAL_TX_TS_IDX], final_tx_ts);
      
      /* Write and send final message. See NOTE 8 below. */
      dwt_writetxdata(FINAL_MSG_SIZE, final_msg, 0); /* Zero offset in TX buffer. */
      dwt_writetxfctrl(FINAL_MSG_SIZE, 0, 1); /* Zero offset in TX buffer, ranging. */
      
      ret = dwt_starttx(DWT_START_TX_DELAYED);
      
      /* If dwt_starttx() returns an error, abandon this ranging exchange and proceed to the next one. See NOTE 12 below. */
      if (ret == DWT_SUCCESS)
      {
	/* Poll DW1000 until TX frame sent event set. See NOTE 9 below. */
	while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))
	{ };
	
	/* Clear TXFRS event. */
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
	
	return RC_ERR_NONE;
      }
    }
  }
  else
  {
    //      dbg_PutString("Poll message RX error/timeout event \n\r");
    /* Clear RX error/timeout events in the DW1000 status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR);
    
    /* Reset RX to properly reinitialise LDE operation. */
    dwt_rxreset();
  }
  return RC_ERR_HW;
}

/*! ------------------------------------------------------------------------------------------------------------------
* @fn final_msg_set_ts()
*
* @brief Fill a given timestamp field in the final message with the given value. In the timestamp fields of the final
*        message, the least significant byte is at the lower address.
*
* @param  ts_field  pointer on the first byte of the timestamp field to fill
*         ts  timestamp value
*
* @return none
*/
void UNE_TWR::final_msg_set_ts(uint8 *ts_field, uint64 ts)
{
  int i;
  for (i = 0; i < MSG_TS_LEN; i++)
  {
    ts_field[i] = (uint8) ts;
    ts >>= 8;
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
void UNE_TWR::final_msg_get_ts(uint8 *ts_field, uint64 *ts)
{
  int i;
  *ts = 0;
  for (i = 0; i < MSG_TS_LEN; i++)
  {
    *ts += ts_field[i] << (i * 8);
  }
}