#include "une_twr.h"

#include "deca_device_api.h"
#include "deca_regs.h"
#include "deca_spi.h"
#include "deca_port.h"


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

/*! ----------------------------------------------------------------------------
*  @brief   Double-sided two-way ranging (DS TWR) initiator example code 
*	    -> Code for INITIATOR, Send Poll, Waits Response, Send Final
*
*	This is a simple code example which acts as the initiator in a DS TWR distance measurement exchange. 
*	This application sends a "poll" frame (recording the TX time-stamp of the poll), and then waits for a "response" 
*	message expected from the "DS TWR responder" example code (companion to this application). 
*	When the response is received its RX time-stamp is recorded and we send a "final" message to
*       complete the exchange. The final message contains all the time-stamps recorded by this application, including 
*	the calculated/predicted TX time-stamp for the final message itself. 
*	The companion "DS TWR responder" example application works out the time-of-flight over-the-air and, thus, the estimated distance between the two devices.
* */

err_te UNE_TWR::twrInitiatorLoop()
{
  
  /* Write frame data to DW1000 and prepare transmission. See NOTE 8 below. */
  //    poll_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
  dwt_writetxdata(sizeof(poll_msg), poll_msg, 0); /* Zero offset in TX buffer. */
  dwt_writetxfctrl(sizeof(poll_msg), 0, 1); /* Zero offset in TX buffer, ranging. */
  
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
    //dbg_PutString("Received answer for poll request.\n\r");
    
    /* Clear good RX frame event and TX frame sent in the DW1000 status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG | SYS_STATUS_TXFRS);
    
    /* A frame has been received, read it into the local buffer. */
    dwm->frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFLEN_MASK;
    if (dwm->frame_len <= FRAME_LEN_MAX)
    {
      dwt_readrxdata(dwm->rx_buffer, dwm->frame_len, 0);
    }
    
    /* Check that the frame is the expected response from the companion "DS TWR responder" example.
    * As the sequence number field of the frame is not relevant, it is cleared to simplify the validation of the frame. */
    //      dwm->rx_buffer[ALL_MSG_SN_IDX] = 0;
    if (memcmp(dwm->rx_buffer, resp_msg, DW_MSG_PREAMBLE_LEN) == 0)
    {
      //	dbg_PutString("Received response frame is correct.\n\r");
      
      uint32 final_tx_time;
      int ret;
      
      /* Retrieve poll transmission and response reception timestamp. */
      poll_tx_ts = DWM1000::getTimestampU64(DWM1000::TX_TS);
      resp_rx_ts = DWM1000::getTimestampU64(DWM1000::RX_TS);
      
      /* Compute final message transmission time. See NOTE 10 below. */
      final_tx_time = (resp_rx_ts + (RESP_RX_TO_FINAL_TX_DLY_UUS * UUS_TO_DWT_TIME)) >> 8;
      dwt_setdelayedtrxtime(final_tx_time);
      
      /* Final TX timestamp is the transmission time we programmed plus the TX antenna delay. */
      final_tx_ts = (((uint64)(final_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DLY;
      
      /* Write all timestamps in the final message. See NOTE 11 below. */
      
      final_msg_set_ts(&final_msg[FINAL_MSG_POLL_TX_TS_IDX], poll_tx_ts);
      final_msg_set_ts(&final_msg[FINAL_MSG_RESP_RX_TS_IDX], resp_rx_ts);
      final_msg_set_ts(&final_msg[FINAL_MSG_FINAL_TX_TS_IDX], final_tx_ts);
      
      /* Write and send final message. See NOTE 8 below. */
      //	tx_final_msg[ALL_MSG_SN_IDX] = frame_seq_nb;
      dwt_writetxdata(sizeof(final_msg), final_msg, 0); /* Zero offset in TX buffer. */
      dwt_writetxfctrl(sizeof(final_msg), 0, 1); /* Zero offset in TX buffer, ranging. */
      ret = dwt_starttx(DWT_START_TX_DELAYED);
      
      /* If dwt_starttx() returns an error, abandon this ranging exchange and proceed to the next one. See NOTE 12 below. */
      if (ret == DWT_SUCCESS)
      {
	/* Poll DW1000 until TX frame sent event set. See NOTE 9 below. */
	while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))
	{ };
	
	/* Clear TXFRS event. */
	dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
	
	/* Increment frame sequence number after transmission of the final message (modulo 256). */
	poll_frame_seq_nb++;
	
	return NO_ERR;
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
  return ERR_DWM;
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
  for (i = 0; i < FINAL_MSG_TS_LEN; i++)
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
void UNE_TWR::final_msg_get_ts(const uint8 *ts_field, uint32 *ts)
{
  int i;
  *ts = 0;
  for (i = 0; i < FINAL_MSG_TS_LEN; i++)
  {
    *ts += ts_field[i] << (i * 8);
  }
}
