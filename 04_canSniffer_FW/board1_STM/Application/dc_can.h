#ifndef DC_CAN_H_
#define DC_CAN_H_
//--------------------------------------------------------------------------------------------------------------------
#include "main.h"
#include "dc_usb.h"
//---------------------------------------------------------------------------------------------------------------------
extern CAN_HandleTypeDef  * canHandles[];
//---------------------------------------------------------------------------------------------------------------------
#define DC_CAN_TX_BUFFER_SIZE	50
#define DC_CAN_RX_BUFFER_SIZE	50

#define DC_CAN_MAX_DATA_LEN		8
//---------------------------------------------------------------------------------------------------------------------
#define filterConfigDefaults { 					\
		  .FilterMode = CAN_FILTERMODE_IDMASK,	\
		  .FilterIdHigh = 0x0000,				\
		  .FilterIdLow = 0x0000,				\
		  .FilterMaskIdHigh = 0x0000,			\
		  .FilterMaskIdLow = 0x0000,			\
		  .FilterFIFOAssignment = CAN_RX_FIFO0,	\
		  .FilterActivation = ENABLE			\
}


//---------------------------------------------------------------------------------------------------------------------
#define CHECK_CAN_CH(x) (x < CAN_MAX_CH)

typedef enum{
	CAN_H = 0,
	CAN_M,
	CAN_L,
	CAN_MAX_CH,
} dc_can_ch_t;
//---------------------------------------------------------------------------------------------------------------------
typedef struct{
	dc_can_ch_t canCh;
	uint32_t 	id;
	uint8_t		dlc;
	bool		rtr;
	bool		ide;
	uint8_t		data[DC_CAN_MAX_DATA_LEN];
} dc_can_packet_t;
//---------------------------------------------------------------------------------------------------------------------
#define CHECK_CAN_PRESCALER(x) 		(x >= 1 && x <= 1024)
#define CHECK_CAN_SYNCJUMPWIDTH(x)	(x >= 1 && x <= 4)
#define CHECK_CAN_BS1(x)			(x >= 1 && x <= 16)
#define CHECK_CAN_BS2(x)			(x >= 1 && x <= 8)


typedef struct{
	uint32_t			prescaler, syncJumpWidth, timeSeg1, timeSeg2;
	CAN_HandleTypeDef	*handle;
	bool 				isInited, isActive;
	TickType_t 			lastPacket;
} dc_can_handle_t;
//---------------------------------------------------------------------------------------------------------------------
#define GET_CAN_CH(x) (x->data[1])
#define GET_PRESCALER(x) (x->data[2])
#define GET_JUMPWIDTH(x) (x->data[3])
#define GET_TIMESEG1(x) (x->data[4])
#define GET_TIMESEG2(x) (x->data[5])

//---------------------------------------------------------------------------------------------------------------------
void dc_can_init(void);
void dc_can_rx_task(void * pvParameters);
void dc_can_tx_task(void * pvParameters);
void dc_can_rx_handler(CAN_HandleTypeDef *hcan);
void dc_can_send_packet(dc_can_ch_t _canChannel, uint32_t _id, bool _rtr, bool _ide, uint8_t _dlc, uint8_t * _pData);
void dc_can_controller(dc_usb_packet_t * packet);
bool dc_can_is_active(dc_can_ch_t can);
//---------------------------------------------------------------------------------------------------------------------
#endif /* DC_CAN_H_ */
