#include "dc_ble.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include "dc_can.h"
#include "dc_main.h"
//---------------------------------------------------------------------------------------------------------------------
extern UART_HandleTypeDef huart3;
//---------------------------------------------------------------------------------------------------------------------
#define RX_BUFFER_LEN 255
//---------------------------------------------------------------------------------------------------------------------
static TaskHandle_t xTaskToNotify = NULL;
static uint8_t rxBuffer[RX_BUFFER_LEN], rxPtr;
static bool bleDfuIsActive = false;
//---------------------------------------------------------------------------------------------------------------------
const uint8_t DFU_INIT_ARRAY[] = {0x09, 0x01, 0xC0};
const uint8_t DFU_END_ARRAY[] = "DipCan Started\r\n";
uint8_t dfuEndArrayPtr = 0;
uint8_t START_NRF_BL_CMD[] = "NRF_BL\r";
//---------------------------------------------------------------------------------------------------------------------
bool dc_ble_dfu_is_active(void){
	return bleDfuIsActive;
}

void dc_ble_init(void){
	rxPtr = 0;
	HAL_UART_Receive_IT(&huart3, &rxBuffer[rxPtr], 1);
}

void dc_ble_task(void * pvParameters){
	xTaskToNotify = xTaskGetCurrentTaskHandle();
	dc_usb_packet_t txPacket;

	dc_ble_init();

	for(;;){
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		while (ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(5))) {}

		txPacket.len = rxPtr;
		memcpy(txPacket.data, rxBuffer, rxPtr);
		HAL_UART_AbortReceive_IT(&huart3);
		dc_ble_init();
		dc_usb_send_data(&txPacket);
	}
}

void dc_ble_check_dfu_end(uint8_t lastByte){
	dfuEndArrayPtr = lastByte == DFU_END_ARRAY[dfuEndArrayPtr] ? dfuEndArrayPtr+1 : 0;
	if ( dfuEndArrayPtr == strlen((char*)DFU_END_ARRAY) ){
		dfuEndArrayPtr = 0;
		bleDfuIsActive = false;
		dc_main_led_off();
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if (huart == &huart3){
		rxPtr++;

		if (rxPtr >= RX_BUFFER_LEN){
			rxPtr = 0;
		}
		dc_ble_check_dfu_end(rxBuffer[rxPtr-1]);

		HAL_UART_Receive_IT(&huart3, &rxBuffer[rxPtr], 1);

		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);
		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
}

HAL_StatusTypeDef dc_ble_send(uint8_t * pData, uint8_t len){
	return HAL_UART_Transmit(&huart3, pData, len, 10);
}

void dc_ble_usb_rx(dc_usb_packet_t *rxPacket){
	if (!bleDfuIsActive) {
		if (	rxPacket->len == sizeof(DFU_INIT_ARRAY) &&
				!memcmp( rxPacket->data, DFU_INIT_ARRAY, sizeof(DFU_INIT_ARRAY) ) &&
				!dc_can_is_active(CAN_MAX_CH) )
		{
			dc_ble_send(START_NRF_BL_CMD, strlen((char*)START_NRF_BL_CMD));
			vTaskDelay(pdMS_TO_TICKS(100));
			bleDfuIsActive = true;
			dc_main_led_on();
		} else {
			return;
		}
	}


	uint8_t maxRetries = 10;
	while (maxRetries && HAL_OK != dc_ble_send(rxPacket->data, rxPacket->len)){
		maxRetries--;
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart){
}
