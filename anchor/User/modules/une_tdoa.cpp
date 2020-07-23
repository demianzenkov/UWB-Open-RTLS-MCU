#include "une_tdoa.h"
#include "tsk_usb.h"
#include "tsk_udp_client.h"
#include "monitoring_pb.h"
#include "settings.h"
#include "tsk_network.h"

extern TskUdpClient tskUdpClient;
extern TskUSB tskUSB;
extern MonitoringPB pb_monitoring;
extern DeviceSettings settings;
extern TskNetwork tskNetwork;

UNE_TDOA::UNE_TDOA(DWM1000 * dwm)
{
  this->dwm = dwm;
}

/* TDOA anchor node routine */
S08 UNE_TDOA::tdoaAnchorRoutine()
{
  U08 packet_len=0;
  
  UNE_TDOA::packet_type_te packet_type = receivePacket(&packet_len);
  
  if (packet_type == UNE_TDOA::SYNC)
  {
  
      pb_monitoring.clearMessage();
      pb_monitoring.message.has_TDOA = true;
      pb_monitoring.message.TDOA.NodeID = settings.pb_settings.message.NodeID;
      pb_monitoring.message.TDOA.SyncID = sync_id;
      pb_monitoring.message.TDOA.SyncNN = sync_n;
      pb_monitoring.message.TDOA.SyncTxTS = (U64)sync_tx_ts << 8;
      pb_monitoring.message.TDOA.SyncRxTS = sync_rx_ts;
      U16 msg_len;
      U16 wake_buf_len;
      pb_monitoring.encode(&pb_monitoring.message, pb_monitoring.temp_buf, &msg_len);
      tskNetwork.wake.prepareBuf(pb_monitoring.temp_buf, 
			     msg_len, 
			     CMD_TDOA_SYNC, 
			     pb_monitoring.temp_buf,
			     &wake_buf_len);
      pb_monitoring.temp_buf[wake_buf_len++] = '\n';
      
      tskUdpClient.transmit(pb_monitoring.temp_buf, wake_buf_len-1);
      return RC_ERR_NONE;
  }
  else if (packet_type == UNE_TDOA::BLINK){
    
    pb_monitoring.clearMessage();
    pb_monitoring.message.has_TDOA = true;
    pb_monitoring.message.TDOA.NodeID = settings.pb_settings.message.NodeID;
    pb_monitoring.message.TDOA.BlinkID = blink_id;
    pb_monitoring.message.TDOA.BlinkTS = blink_ts;
    pb_monitoring.message.TDOA.BlinkNN = blink_n;
    U16 msg_len;
    U16 wake_buf_len;
    pb_monitoring.encode(&pb_monitoring.message, pb_monitoring.temp_buf, &msg_len);
    tskNetwork.wake.prepareBuf(pb_monitoring.temp_buf, 
			   msg_len, 
			   CMD_TDOA_BLINK, 
			   pb_monitoring.temp_buf,
			   &wake_buf_len);
    pb_monitoring.temp_buf[wake_buf_len++] = '\n';
    
    tskUdpClient.transmit(pb_monitoring.temp_buf, wake_buf_len-1);
    
    return RC_ERR_NONE;
  }
  
  return RC_ERR_DATA;
}

/* Synchronisational node routine */
S08 UNE_TDOA::tdoaSyncNodeRoutine()
{
  sendSyncWithDelay();
  return RC_ERR_NONE;
}

/* Blink node routine */
S08 UNE_TDOA::tdoaBlinkNodeRoutine()
{
  sendBlinkWithDelay();
  return RC_ERR_NONE;
}

UNE_TDOA::packet_type_te UNE_TDOA::receivePacket(uint8_t * data_len) 
{
  packet_type_te packet_type = NO_DATA;
  
  memset(dwm->rx_buffer, 0, FRAME_LEN_MAX);
  
   /* Clear reception timeout to start next ranging process. */
  dwt_setrxtimeout(0);
  
  //dbg_PutString("Trying to receive Poll message...\n\r");
  /* Activate reception immediately. */
  dwt_rxenable(DWT_START_RX_IMMEDIATE);
  
  /* Poll until a frame is properly received or an error/timeout occurs. See NOTE 4 below.
  * STATUS register is 5 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
  * function to access it. */
  while (!((status_reg = dwt_read32bitreg(SYS_STATUS_ID)) & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_TO | SYS_STATUS_ALL_RX_ERR)))
  { };
  
  if (status_reg & SYS_STATUS_RXFCG)
  {
    /* A frame has been received, copy it to our local buffer. */
    frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
    if (frame_len <= FRAME_LEN_MAX)
    {
      dwt_readrxdata(dwm->rx_buffer, frame_len, 0);
      
      if(memcmp(dwm->rx_buffer, sync_msg, 4) == 0) 
      {
	sync_id = dwm->rx_buffer[TDOA_MSG_SYNC_ID];
	sync_n = ((dwm->rx_buffer[TDOA_MSG_SYNC_L] << 8) + 
		  dwm->rx_buffer[TDOA_MSG_SYNC_H]);
	
	sync_rx_ts = DWM1000::getTimestampU64(DWM1000::RX_TS);
	sync_tx_ts = 0;
	for (int i = 4; i >= 0; i--)
	{
	  sync_tx_ts <<= 8;
	  sync_tx_ts |=  dwm->rx_buffer[TDOA_SYNC_TS+i];
	}
	packet_type = SYNC;
      }
      
      else if(memcmp(dwm->rx_buffer, blink_msg, 4) == 0) 
      {
	blink_id = dwm->rx_buffer[TDOA_MSG_BLINK_ID];
	blink_n = ((dwm->rx_buffer[TDOA_MSG_BLINK_L] << 8) + 
		   dwm->rx_buffer[TDOA_MSG_BLINK_H]);
	blink_ts =  DWM1000::getTimestampU64(DWM1000::RX_TS);
	packet_type = BLINK;
      }
      
      else {
	packet_type = UNKNOWN;
      }
      if (data_len != NULL) {
	*data_len = frame_len;
      }
    }
    /* Clear good RX frame event in the DW1000 status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);
  }
  else
  {
    /* Clear RX error events in the DW1000 status register. */
    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
    return packet_type;
  }
  return packet_type;
}


void UNE_TDOA::sendImmediate(U08 * packet, U08 size)
{
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
  
  dwt_writetxdata(size, packet, 0); /* Zero offset in TX buffer. */
  dwt_writetxfctrl(size, 0, 1); /* Zero offset in TX buffer, ranging. */
  
  /* Start transmission. */
  dwt_starttx(DWT_START_TX_IMMEDIATE);
  
  /* Poll DW1000 until TX frame sent event set. See NOTE 5 below.
  * STATUS register is 5 bytes long but, as the event we are looking at is in the first byte of the register, we can use this simplest API
  * function to access it.*/
  while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))
  { };
  
  /* Clear TX frame sent event. */
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
}


S08 UNE_TDOA::sendSyncWithDelay()
{
  if (!sync_started) 
  {
    sendImmediate(sync_msg, sizeof(sync_msg));
    sync_started = 1;
    return RC_ERR_NONE;
  }
 
  sync_msg[TDOA_MSG_SYNC_ID] = settings.pb_settings.message.NodeID;
  sync_msg[TDOA_MSG_SYNC_L] = (sync_n >> 8) & 0xFF;
  sync_msg[TDOA_MSG_SYNC_H] = sync_n & 0xFF;
  sync_n++;
    
  tx_time = dwt_readtxtimestamphi32();
  
  tx_time64 = tx_time;
  tx_time64 = tx_time64 << 8;
  
  tx_time64 += (U64)(((float)(settings.pb_settings.message.TdoaSyncPeriod)/1000)/DWT_TIME_UNITS);
  tx_time64 &= 0xFFFFFFFE00UL;
  tx_time = tx_time64 >> 8;
  
  for (int i=0; i<4; i++) {
    sync_msg[TDOA_SYNC_TS+i] = (tx_time >> (i*8)) & 0xFF;
  }
  
  dwt_setdelayedtrxtime(tx_time);
  dwt_writetxdata(sizeof(sync_msg), sync_msg, 0); /* Zero offset in TX buffer. */
  dwt_writetxfctrl(sizeof(sync_msg), 0, 1); /* Zero offset in TX buffer, ranging. */
  S08 ret = dwt_starttx(DWT_START_TX_DELAYED);
  if (ret == DWT_ERROR)
  {
    return RC_ERR_HW;
  }
  
  /* Poll DW1000 until TX frame sent event set. See NOTE 9 below. */
  while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))
  { };
  /* Clear TXFRS event. */
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
  
  return RC_ERR_NONE;
}


S08 UNE_TDOA::sendBlinkWithDelay()
{
  if (!blink_started) 
  {
    sendImmediate(blink_msg, sizeof(blink_msg));
    blink_started = 1;
    return RC_ERR_NONE;
  }
 
  blink_msg[TDOA_MSG_BLINK_ID] = settings.pb_settings.message.NodeID;
  blink_msg[TDOA_MSG_BLINK_L] = (blink_n >> 8) & 0xFF;
  blink_msg[TDOA_MSG_BLINK_H] = blink_n & 0xFF;
  blink_n++;
    
  tx_time = dwt_readtxtimestamphi32();
  
  tx_time64 = tx_time;
  tx_time64 = tx_time64 << 8;
  
  tx_time64 += (U64)(((float)(settings.pb_settings.message.TdoaPollPeriod)/1000)/DWT_TIME_UNITS);
  tx_time64 &= 0xFFFFFFFE00UL;
  tx_time = tx_time64 >> 8;
  
  for (int i=0; i<4; i++) {
    blink_msg[TDOA_BLINK_TS+i] = (tx_time >> (i*8)) & 0xFF;
  }
  
  dwt_setdelayedtrxtime(tx_time);
  /* Zero offset in TX buffer. */
  dwt_writetxdata(sizeof(blink_msg), blink_msg, 0);
  /* Zero offset in TX buffer, ranging. */
  dwt_writetxfctrl(sizeof(blink_msg), 0, 1);	
  S08 ret = dwt_starttx(DWT_START_TX_DELAYED);
  if (ret == DWT_ERROR) {
    return RC_ERR_HW;
  }
  
  /* Poll DW1000 until TX frame sent event set. See NOTE 9 below. */
  while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS))
  { };
  /* Clear TXFRS event. */
  dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
  
  return RC_ERR_NONE;
}


