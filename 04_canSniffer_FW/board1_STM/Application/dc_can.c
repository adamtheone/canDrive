#include "dc_can.h"
#include <string.h>
#include "main.h"
#include "dc_main.h"
#include "dc_forwarder_module.h"
#include "dc_infocollector_module.h"
//---------------------------------------------------------------------------------------------------------------------
extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;
extern CAN_HandleTypeDef hcan3;
//---------------------------------------------------------------------------------------------------------------------
static QueueHandle_t txBufferQueue;
static QueueHandle_t rxBufferQueue;
//---------------------------------------------------------------------------------------------------------------------
dc_can_handle_t canHandle[CAN_MAX_CH];
//---------------------------------------------------------------------------------------------------------------------
bool dc_can_is_active(dc_can_ch_t can){
	if (can >= CAN_MAX_CH){
		return canHandle[CAN_L].isActive || canHandle[CAN_M].isActive || canHandle[CAN_H].isActive;
	}
	return canHandle[can].isActive;
}
//---------------------------------------------------------------------------------------------------------------------
bool dc_can_start(dc_can_ch_t can){
	if ( HAL_CAN_Start(canHandle[can].handle) != HAL_OK ) { return false; }
	//if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_TX_MAILBOX_EMPTY) != HAL_OK) { Error_Handler(); }
	if ( HAL_CAN_ActivateNotification(canHandle[can].handle, CAN_IT_RX_FIFO0_MSG_PENDING ) != HAL_OK) { return false; }
	if ( HAL_CAN_ActivateNotification(canHandle[can].handle, CAN_IT_RX_FIFO1_MSG_PENDING ) != HAL_OK) { return false; }
	if ( HAL_CAN_ActivateNotification(canHandle[can].handle, CAN_IT_ERROR) != HAL_OK ) { return false; }

	canHandle[can].isActive = true;
	canHandle[can].lastPacket = 0;

	return true;
}

bool dc_can_hal_init(dc_can_ch_t can){
	if (canHandle[can].isActive) { return false; }

	CAN_FilterTypeDef  sFilterConfig = filterConfigDefaults;
	switch (can){
		case CAN_H:
			canHandle[can].handle->Instance = CAN1;
			sFilterConfig.FilterBank = 0;
			sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;
			sFilterConfig.SlaveStartFilterBank = 14;
			break;
		case CAN_M:
			canHandle[can].handle->Instance = CAN2;
			sFilterConfig.FilterBank = 14;
			sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
			sFilterConfig.SlaveStartFilterBank = 14;
			break;
		case CAN_L:
			canHandle[can].handle->Instance = CAN3;
			sFilterConfig.FilterBank = 0;
			sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
			sFilterConfig.SlaveStartFilterBank = 14;
			break;
		default: return false;
	}

	canHandle[can].handle->Init.Prescaler 		= canHandle[can].prescaler;
	canHandle[can].handle->Init.SyncJumpWidth 	= canHandle[can].syncJumpWidth;
	canHandle[can].handle->Init.TimeSeg1 		= canHandle[can].timeSeg1;
	canHandle[can].handle->Init.TimeSeg2 		= canHandle[can].timeSeg2;

	if ( HAL_CAN_Init(canHandle[can].handle) != HAL_OK ) { return false; }
	if ( HAL_CAN_ConfigFilter(canHandle[can].handle, &sFilterConfig) != HAL_OK ) { return false; }

	canHandle[can].isInited = true;

	return true;
}

bool dc_can_hal_deinit(dc_can_ch_t can){
	if (HAL_CAN_DeactivateNotification(canHandle[can].handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) { }
	if (HAL_CAN_DeactivateNotification(canHandle[can].handle, CAN_IT_RX_FIFO1_MSG_PENDING) != HAL_OK) { }
	if (HAL_CAN_DeactivateNotification(canHandle[can].handle, CAN_IT_ERROR) != HAL_OK) { }
	if (HAL_CAN_Stop(canHandle[can].handle) != HAL_OK) { }
	if (HAL_CAN_DeInit(canHandle[can].handle) != HAL_OK) { }
	canHandle[can].isInited = false;
	canHandle[can].isActive = false;
	canHandle[can].lastPacket = 0;
	return true;
}
//---------------------------------------------------------------------------------------------------------------------


void dc_can_init(void){
	canHandle[CAN_L].handle 		= &hcan3;
	canHandle[CAN_L].prescaler 		= 135;
	canHandle[CAN_L].syncJumpWidth 	= CAN_SJW_1TQ;
	canHandle[CAN_L].timeSeg1 		= CAN_BS1_6TQ;
	canHandle[CAN_L].timeSeg2 		= CAN_BS2_1TQ;
	canHandle[CAN_L].lastPacket 	= 0;
	canHandle[CAN_L].isInited 		= false;
	canHandle[CAN_L].isActive 		= false;


	canHandle[CAN_M].handle 		= &hcan2;
	canHandle[CAN_M].prescaler 		= 21;
	canHandle[CAN_M].syncJumpWidth 	= CAN_SJW_1TQ;
	canHandle[CAN_M].timeSeg1 		= CAN_BS1_15TQ;
	canHandle[CAN_M].timeSeg2 		= CAN_BS2_2TQ;
	canHandle[CAN_M].lastPacket 	= 0;
	canHandle[CAN_M].isInited 		= false;
	canHandle[CAN_M].isActive 		= false;

	canHandle[CAN_H].handle 		= &hcan1;
	canHandle[CAN_H].prescaler 		= 4;
	canHandle[CAN_H].syncJumpWidth 	= CAN_SJW_1TQ;
	canHandle[CAN_H].timeSeg1 		= CAN_BS1_15TQ;
	canHandle[CAN_H].timeSeg2 		= CAN_BS2_2TQ;
	canHandle[CAN_H].lastPacket 	= 0;
	canHandle[CAN_H].isInited 		= false;
	canHandle[CAN_H].isActive 		= false;

	HAL_GPIO_WritePin(LS_CAN_RS_GPIO_Port, LS_CAN_RS_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MS_CAN_RS_GPIO_Port, MS_CAN_RS_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(HS_CAN_RS_GPIO_Port, HS_CAN_RS_Pin, GPIO_PIN_RESET);

	txBufferQueue = xQueueCreate( DC_CAN_TX_BUFFER_SIZE, sizeof(dc_can_packet_t));
	rxBufferQueue = xQueueCreate( DC_CAN_RX_BUFFER_SIZE, sizeof(dc_can_packet_t));
}
//---------------------------------------------------------------------------------------------------------------------
// Sending
//---------------------------------------------------------------------------------------------------------------------
void dc_can_send_packet(dc_can_ch_t _canChannel, uint32_t _id, bool _rtr, bool _ide, uint8_t _dlc, uint8_t * _pData){
	dc_can_packet_t 	txPacket = {
			.canCh 		= _canChannel,
			.id 		= _id,
			.rtr 		= _rtr,
			.ide		= _ide,
			.dlc		= _dlc > DC_CAN_MAX_DATA_LEN ? DC_CAN_MAX_DATA_LEN : _dlc,
	};
	memcpy(txPacket.data, _pData, txPacket.dlc);

	if (is_handler_mode()){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if ( xQueueSendFromISR(txBufferQueue, &txPacket, &xHigherPriorityTaskWoken) != pdTRUE ){
			Error_Handler();
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	} else {
		if ( xQueueSend(txBufferQueue, &txPacket, 10) != pdTRUE ){
			Error_Handler();
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------
void dc_can_tx_task(void * pvParameters){
	uint32_t           	txMailbox;
	dc_can_packet_t 	txPacket;
	CAN_TxHeaderTypeDef	txHeader = {
			.TransmitGlobalTime = DISABLE
	};

	for(;;){
		if ( xQueueReceive(txBufferQueue, &txPacket, portMAX_DELAY) == pdTRUE ){
			txHeader.DLC = txPacket.dlc;
			txHeader.RTR = txPacket.rtr ? CAN_RTR_REMOTE : CAN_RTR_DATA;
			txHeader.IDE = txPacket.ide ? CAN_ID_EXT : CAN_ID_STD;
			if (txPacket.ide){
				txHeader.ExtId = txPacket.id;
			} else {
				txHeader.StdId = txPacket.id;
			}
			if (!canHandle[txPacket.canCh].isActive){ continue; }

			if (HAL_CAN_AddTxMessage(canHandle[txPacket.canCh].handle, &txHeader, txPacket.data, &txMailbox) == HAL_OK){
				//dc_usb_send_str("sending...\r\n");
			} else {
				Error_Handler();
				// packet could be put at the end of the queue for resending
			}
		}
	}
}

uint32_t decode_jumpWidth(uint8_t jumpWidth){
	switch (jumpWidth){
		case 1: 	return CAN_SJW_1TQ;
		case 2: 	return CAN_SJW_2TQ;
		case 3: 	return CAN_SJW_3TQ;
		case 4: 	return CAN_SJW_4TQ;
	}
	return 0;
}

uint32_t decode_timeSeg1(uint8_t timeSeg1){
	switch (timeSeg1){
		case 1: 	return CAN_BS1_1TQ;
		case 2: 	return CAN_BS1_2TQ;
		case 3: 	return CAN_BS1_3TQ;
		case 4: 	return CAN_BS1_4TQ;
		case 5: 	return CAN_BS1_5TQ;
		case 6: 	return CAN_BS1_6TQ;
		case 7: 	return CAN_BS1_7TQ;
		case 8: 	return CAN_BS1_8TQ;
		case 9: 	return CAN_BS1_9TQ;
		case 10: 	return CAN_BS1_10TQ;
		case 11: 	return CAN_BS1_11TQ;
		case 12: 	return CAN_BS1_12TQ;
		case 13: 	return CAN_BS1_13TQ;
		case 14: 	return CAN_BS1_14TQ;
		case 15: 	return CAN_BS1_15TQ;
		case 16: 	return CAN_BS1_16TQ;
	}
	return 0;
}

uint32_t decode_timeSeg2(uint8_t timeSeg2){
	switch (timeSeg2){
		case 1: 	return CAN_BS2_1TQ;
		case 2: 	return CAN_BS2_2TQ;
		case 3: 	return CAN_BS2_3TQ;
		case 4: 	return CAN_BS2_4TQ;
		case 5: 	return CAN_BS2_5TQ;
		case 6: 	return CAN_BS2_6TQ;
		case 7: 	return CAN_BS2_7TQ;
		case 8: 	return CAN_BS2_8TQ;
	}
	return 0;
}

void dc_can_controller(dc_usb_packet_t * packet){
	bool CAN_M_reinit = false;
	if ( !IS_TARGET_IN_GROUP(GET_TARGET(packet), T_CAN_CTRL_GROUP)) { return; }

	switch (GET_TARGET(packet)){
		case T_CAN_CTRL_SET_TIMING:
			if ( !CHECK_CAN_CH( 			GET_CAN_CH(packet) 		) ) { return; }
			if ( !CHECK_CAN_PRESCALER( 		GET_PRESCALER(packet) 	) ) { return; }
			if ( !CHECK_CAN_SYNCJUMPWIDTH( 	GET_JUMPWIDTH(packet) 	) ) { return; }
			if ( !CHECK_CAN_BS1(			GET_TIMESEG1(packet) 	) ) { return; }
			if ( !CHECK_CAN_BS2( 			GET_TIMESEG2(packet)	) ) { return; }
			canHandle[GET_CAN_CH(packet)].prescaler = GET_PRESCALER(packet);
			canHandle[GET_CAN_CH(packet)].syncJumpWidth = decode_jumpWidth(GET_JUMPWIDTH(packet));
			canHandle[GET_CAN_CH(packet)].timeSeg1 = decode_timeSeg1(GET_TIMESEG1(packet));
			canHandle[GET_CAN_CH(packet)].timeSeg2 = decode_timeSeg2(GET_TIMESEG2(packet));
			break;

		case T_CAN_CTRL_START:
			if (!CHECK_CAN_CH( GET_CAN_CH(packet) )) { return; }

			if (GET_CAN_CH(packet) == CAN_M && !canHandle[CAN_H].isInited){
				if (dc_can_hal_init(CAN_H)){
					dc_usb_send_str("CAN_H inited\r\n");
				}
			}
			if (GET_CAN_CH(packet) == CAN_H && canHandle[CAN_M].isInited){
				CAN_M_reinit = true;
				if (dc_can_hal_deinit(CAN_M)){
					dc_usb_send_str("CAN_M de-inited\r\n");
				}
			}

			if (dc_can_hal_init(GET_CAN_CH(packet))) {
				dc_usb_send_str("CAN hal inited successfully\r\n");
			} else {
				dc_usb_send_str("CAN hal init failed\r\n");
			}
			if (dc_can_start(GET_CAN_CH(packet))) {
				dc_usb_send_str("CAN started successfully\r\n");
			} else {
				dc_usb_send_str("CAN start failed\r\n");
			}

			if (CAN_M_reinit){
				if (!dc_can_hal_init(CAN_M)) { dc_usb_send_str("CAN_M hal re-init failed\r\n"); }
				if (!dc_can_start(CAN_M)) { dc_usb_send_str("CAN_M re-start failed\r\n"); }
			}

			break;

		case T_CAN_CTRL_STOP:
			if (!dc_can_hal_deinit(CAN_L)){
				dc_usb_send_str("CAN L deinit failed\r\n");
			}
			if (!dc_can_hal_deinit(CAN_M)){
				dc_usb_send_str("CAN M deinit failed\r\n");
			}
			if (!dc_can_hal_deinit(CAN_H)){
				dc_usb_send_str("CAN H deinit failed\r\n");
			}
			dc_usb_send_str("CAN Deinited\r\n");
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------
// Receiving
//---------------------------------------------------------------------------------------------------------------------
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan){
	CAN_RxHeaderTypeDef rxHeader;
	dc_can_packet_t		rxPacket;

	HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

	HAL_CAN_DeactivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, rxPacket.data) == HAL_OK ) {
		BaseType_t		xHigherPriorityTaskWoken = pdFALSE;
		rxPacket.ide	= rxHeader.IDE == CAN_ID_EXT;
		rxPacket.rtr	= rxHeader.RTR == CAN_RTR_REMOTE;
		rxPacket.dlc 	= rxHeader.DLC;
		rxPacket.id 	= rxPacket.ide ? rxHeader.ExtId : rxHeader.StdId;
		rxPacket.canCh	= hcan == canHandle[CAN_H].handle ? CAN_H : hcan == canHandle[CAN_M].handle ? CAN_M : CAN_L;

		if ( xQueueSendFromISR(rxBufferQueue, &rxPacket, &xHigherPriorityTaskWoken) != pdTRUE ){
			Error_Handler();
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}

	HAL_CAN_ActivateNotification(hcan, CAN_IT_RX_FIFO0_MSG_PENDING );
}
//---------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------------------------------------------
void dc_can_rx_task( void * pvParameters){
	dc_can_packet_t		rxPacket;

	for(;;){
		if ( xQueueReceive(rxBufferQueue, &rxPacket, portMAX_DELAY) == pdTRUE ){
			// TODO after 1500ms of inactivity on LSCAN, it could be de-inited?

			// Sharing the incoming packet with the modules
			dc_forwarder_can_rx(&rxPacket);
			dc_infocollector_can_rx(&rxPacket);
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan){ Error_Handler(); }
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan){ Error_Handler(); } // TODO delete this after making sure it never happens!
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan){}
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan){}
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan){}



