#ifndef _UNE_TWR
#define _UNE_TWR

#include "prj_defs.h"
#include "dwm1000.h"

/* UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
 * 1 uus = 512 / 499.2 µs and 1 µs = 499.2 * 128 dtu. */
#define UUS_TO_DWT_TIME 65536

/* Delay between frames, in UWB microseconds. See NOTE 4. */
/* Delays for ANCHOR(initiator) */
/* This is the delay from Frame RX timestamp to TX reply timestamp used for calculating/setting the DW1000's delayed TX function. This includes the
* frame length of approximately 2.46 ms with above configuration. */
#define POLL_RX_TO_RESP_TX_DLY_UUS 2750
/* This is the delay from the end of the frame transmission to the enable of the receiver, as programmed for the DW1000's wait for response feature. */
#define RESP_TX_TO_FINAL_RX_DLY_UUS 300		//500
/* Receive final timeout. See NOTE 5 below. */
#define FINAL_RX_TIMEOUT_UUS 3700

/* Delays for TAG(initiator) */
/* Delay between frames, in UWB microseconds. See NOTE 4 below. */
/* This is the delay from the end of the frame transmission to the enable of the receiver, 
 * as programmed for the DW1000's wait for response feature. */
#define POLL_TX_TO_RESP_RX_DLY_UUS 1000 //300 // 1000 - critical minimum value
/* This is the delay from Frame RX timestamp to TX reply timestamp used for calculating/setting 
 * the DW1000's delayed TX function. This includes the frame length of approximately 2.66 ms with above configuration. */
#define RESP_RX_TIMEOUT_UUS 3700 //2700 	// Receive response timeout. NOTE 5
#define RESP_RX_TO_FINAL_TX_DLY_UUS 3000
#define PRE_TIMEOUT 8 			// Preamble tomeout. NOTE 6


#define POLL_MSG_SIZE	10
#define RESP_MSG_SIZE	12
#define FINAL_MSG_SIZE	29

#define DW_MSG_PREAMBLE_LEN	4

#define DEFAULT_TAG_ID		0x01
#define DEFAULT_ANCHOR_ID	0x01

#define START_POLL_N_H	0
#define START_POLL_N_L	0
#define START_RESP_N_H	0
#define START_RESP_N_L	0
#define START_FINAL_N_H	0
#define START_FINAL_N_L	0

#define MSG_TS_LEN 	5

#define FINAL_MSG_POLL_TX_TS_IDX	12
#define FINAL_MSG_RESP_RX_TS_IDX	17
#define FINAL_MSG_FINAL_TX_TS_IDX	22

#define MSG_TAG_IDX		4
#define MSG_AN_IDX		5
#define MSG_POLL_H_IDX		6
#define MSG_POLL_L_IDX		7
#define MSG_RESP_H_IDX		8
#define MSG_RESP_L_IDX		9
#define MSG_FINAL_H_IDX		10
#define MSG_FINAL_L_IDX		11

#define SPEED_OF_LIGHT 299702547	// metres per second

   
class UNE_TWR {
public:
  UNE_TWR(DWM1000 * dwm);
  ~UNE_TWR();
  
  static void initDWM();
  S08 twrResponderLoop();
  S08 twrInitiatorLoop(U08 an_id);	// use anchor in tag mode for calibration
  static void final_msg_get_ts(uint8 *ts_field, uint64 *ts);
  static void final_msg_set_ts(uint8 *ts_field, uint64 ts);
  
private:
  DWM1000 * dwm;
  
  U16 poll_frame_seq_nb;	// Tag
  U16 resp_frame_seq_nb;	// Anchor
  U16 final_frame_seq_nb;	// Tag
  
  U64 poll_rx_ts; 	// anchor(responder)
  U64 resp_tx_ts;	// anchor(responder)
  U64 final_rx_ts;	// anchor(responder)
  
  U64 poll_tx_ts;	// tag(initiator)
  U64 resp_rx_ts;	// tag(initiator)
  U64 final_tx_time;	// tag(initiator)
  U64 final_tx_ts;	// tag(initiator)
  
  U08 tag_id;		// anchor(initiator)
  double tof;		// anchor(initiator)
  float distance;	// anchor(initiator)
  
  U08 poll_msg[POLL_MSG_SIZE] = {'T', 'W', 'R', 'P',
			DEFAULT_TAG_ID,
			DEFAULT_ANCHOR_ID,
			START_POLL_N_H,
			START_POLL_N_L,
  			0, 0};		// CRC
  
  U08 resp_msg[RESP_MSG_SIZE] = {'T', 'W', 'R', 'R', 
			DEFAULT_TAG_ID,
			DEFAULT_ANCHOR_ID,
			START_POLL_N_H,
			START_POLL_N_L,
			START_RESP_N_H,
			START_RESP_N_L,
  			0, 0};		// CRC
  
  U08 final_msg[FINAL_MSG_SIZE] = {'T', 'W', 'R', 'F', 
			DEFAULT_TAG_ID,
			DEFAULT_ANCHOR_ID,
			START_POLL_N_H,
			START_POLL_N_L,
			START_RESP_N_H,
			START_RESP_N_L,
			START_FINAL_N_H,
			START_FINAL_N_L,
			0,0,0,0,0,	// POLL TX
			0,0,0,0,0,	// RESP RX
			0,0,0,0,0,	// FINAL TX	
  			0, 0};		// 
};

#endif

/*****************************************************************************************************************************************************
*  DECAWAVE TWR Example NOTES:
*
* 1. The sum of the values is the TX to RX antenna delay, experimentally determined by a calibration process. Here we use a hard coded typical value
*    but, in a real application, each device should have its own antenna delay properly calibrated to get the best possible precision when performing
*    range measurements.
* 2. The messages here are similar to those used in the DecaRanging ARM application (shipped with EVK1000 kit). They comply with the IEEE
*    802.15.4 standard MAC data frame encoding and they are following the ISO/IEC:24730-62:2013 standard. The messages used are:
*     - a poll message sent by the initiator to trigger the ranging exchange.
*     - a response message sent by the responder allowing the initiator to go on with the process
*     - a final message sent by the initiator to complete the exchange and provide all information needed by the responder to compute the
*       time-of-flight (distance) estimate.
*    The first 10 bytes of those frame are common and are composed of the following fields:
*     - byte 0/1: frame control (0x8841 to indicate a data frame using 16-bit addressing).
*     - byte 2: sequence number, incremented for each new frame.
*     - byte 3/4: PAN ID (0xDECA).
*     - byte 5/6: destination address, see NOTE 3 below.
*     - byte 7/8: source address, see NOTE 3 below.
*     - byte 9: function code (specific values to indicate which message it is in the ranging process).
*    The remaining bytes are specific to each message as follows:
*    Poll message:
*     - no more data
*    Response message:
*     - byte 10: activity code (0x02 to tell the initiator to go on with the ranging exchange).
*     - byte 11/12: activity parameter, not used for activity code 0x02.
*    Final message:
*     - byte 10 -> 13: poll message transmission timestamp.
*     - byte 14 -> 17: response message reception timestamp.
*     - byte 18 -> 21: final message transmission timestamp.
*    All messages end with a 2-byte checksum automatically set by DW1000.
* 3. Source and destination addresses are hard coded constants in this example to keep it simple but for a real product every device should have a
*    unique ID. Here, 16-bit addressing is used to keep the messages as short as possible but, in an actual application, this should be done only
*    after an exchange of specific messages used to define those short addresses for each device participating to the ranging exchange.
* 4. Delays between frames have been chosen here to ensure proper synchronisation of transmission and reception of the frames between the initiator
*    and the responder and to ensure a correct accuracy of the computed distance. The user is referred to DecaRanging ARM Source Code Guide for more
*    details about the timings involved in the ranging process.
* 5. This timeout is for complete reception of a frame, i.e. timeout duration must take into account the length of the expected frame. Here the value
*    is arbitrary but chosen large enough to make sure that there is enough time to receive the complete final frame sent by the responder at the
*    110k data rate used (around 3.5 ms).
* 6. The preamble timeout allows the receiver to stop listening in situations where preamble is not starting (which might be because the responder is
*    out of range or did not receive the message to respond to). This saves the power waste of listening for a message that is not coming. We
*    recommend a minimum preamble timeout of 5 PACs for short range applications and a larger value (e.g. in the range of 50% to 80% of the preamble
*    length) for more challenging longer range, NLOS or noisy environments.
* 7. In a real application, for optimum performance within regulatory limits, it may be necessary to set TX pulse bandwidth and TX power, (using
*    the dwt_configuretxrf API call) to per device calibrated values saved in the target system or the DW1000 OTP memory.
* 8. We use polled mode of operation here to keep the example as simple as possible but all status events can be used to generate interrupts. Please
*    refer to DW1000 User Manual for more details on "interrupts". It is also to be noted that STATUS register is 5 bytes long but, as the event we
*    use are all in the first bytes of the register, we can use the simple dwt_read32bitreg() API call to access it instead of reading the whole 5
*    bytes.
* 9. Timestamps and delayed transmission time are both expressed in device time units so we just have to add the desired response delay to poll RX
*    timestamp to get response transmission time. The delayed transmission time resolution is 512 device time units which means that the lower 9 bits
*    of the obtained value must be zeroed. This also allows to encode the 40-bit value in a 32-bit words by shifting the all-zero lower 8 bits.
* 10. dwt_writetxdata() takes the full size of the message as a parameter but only copies (size - 2) bytes as the check-sum at the end of the frame is
*     automatically appended by the DW1000. This means that our variable could be two bytes shorter without losing any data (but the sizeof would not
*     work anymore then as we would still have to indicate the full length of the frame to dwt_writetxdata()).
* 11. When running this example on the EVB1000 platform with the POLL_RX_TO_RESP_TX_DLY response delay provided, the dwt_starttx() is always
*     successful. However, in cases where the delay is too short (or something else interrupts the code flow), then the dwt_starttx() might be issued
*     too late for the configured start time. The code below provides an example of how to handle this condition: In this case it abandons the
*     ranging exchange and simply goes back to awaiting another poll message. If this error handling code was not here, a late dwt_starttx() would
*     result in the code flow getting stuck waiting subsequent RX event that will will never come. The companion "initiator" example (ex_05a) should
*     timeout from awaiting the "response" and proceed to send another poll in due course to initiate another ranging exchange.
* 12. The high order byte of each 40-bit time-stamps is discarded here. This is acceptable as, on each device, those time-stamps are not separated by
*     more than 2**32 device time units (which is around 67 ms) which means that the calculation of the round-trip delays can be handled by a 32-bit
*     subtraction.
* 13. The user is referred to DecaRanging ARM application (distributed with EVK1000 product) for additional practical example of usage, and to the
*     DW1000 API Guide for more details on the DW1000 driver functions.
****************************************************************************************************************************************************/
