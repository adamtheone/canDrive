#include "dc_wifi.h"
#include "dc_main.h"
#include "stm32f4xx.h"
#include "main.h"
#include "dc_usb.h"
#include "string.h"
#include "dc_wifi_packet_descriptor.h"
#include "dc_usb.h"
#include "usbd_def.h"
#include "usbd_cdc_if.h"
#include "dc_controller.h"
//---------------------------------------------------------------------------------------------------------------------
extern UART_HandleTypeDef huart3;
extern xTaskHandle	dc_wifi_txTaskHandle;
//---------------------------------------------------------------------------------------------------------------------
QueueHandle_t dc_wifi_txQueue;
#define DC_WIFI_TX_QUEUE_SIZE	10

QueueHandle_t dc_wifi_rxQueue;
#define DC_WIFI_RX_QUEUE_SIZE	10
//---------------------------------------------------------------------------------------------------------------------
static dc_usb_packet_t wifi_rx_usb_tx_packet;
static uint8_t rxTempByte;
//---------------------------------------------------------------------------------------------------------------------
static bool dc_wifi_isBootloaderModeActive = false;
const char machine[] = "started";
uint8_t machinePtr = 0;
//---------------------------------------------------------------------------------------------------------------------
bool dc_wifi_isBootloaderMode(void){
	return dc_wifi_isBootloaderModeActive;
}
//---------------------------------------------------------------------------------------------------------------------
void dc_wifi_init(void){
	dc_wifi_normalStart();
	//dc_wifi_triggerBootloader();

	dc_wifi_txQueue = xQueueCreate( DC_WIFI_TX_QUEUE_SIZE, sizeof(uint8_t) * INFO_PACKET_LEN);
	dc_wifi_rxQueue = xQueueCreate( DC_WIFI_RX_QUEUE_SIZE, sizeof(uint8_t) * CTRL_PACKET_LEN);

	wifi_rx_usb_tx_packet.len = 0;
	HAL_UART_Receive_DMA(&huart3, &rxTempByte, 1);
}
//----------------------------------------------------------------------------------------------------------------------
void dc_wifi_triggerBootloader(void){

}
//----------------------------------------------------------------------------------------------------------------------
void dc_wifi_normalStart(void){

}
//----------------------------------------------------------------------------------------------------------------------
void dc_wifi_update_finished_observer(uint8_t chr){
	machinePtr = machine[machinePtr] == (char)chr ? machinePtr + 1 : 0;
	if (machinePtr == strlen(machine)){
		dc_wifi_isBootloaderModeActive = false;
		machinePtr = 0;
		NVIC_SystemReset();
	}
}
//----------------------------------------------------------------------------------------------------------------------
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	static uint8_t startIndex = INVALID_INDEX;
	static uint8_t endIndex = INVALID_INDEX;

	if (huart == &huart3){
		wifi_rx_usb_tx_packet.data[wifi_rx_usb_tx_packet.len] = rxTempByte;
		wifi_rx_usb_tx_packet.len++;

		if (!dc_wifi_isBootloaderModeActive){
			if (rxTempByte == CTRL_PACKET_START_CHAR){
				startIndex = wifi_rx_usb_tx_packet.len - 1;
			}
			if (rxTempByte == CTRL_PACKET_END_CHAR){
				endIndex = wifi_rx_usb_tx_packet.len - 1;
				if (startIndex != INVALID_INDEX && endIndex - startIndex + 1 == CTRL_PACKET_LEN){
					BaseType_t xHigherPriorityTaskWoken = pdFALSE;
					if ( xQueueSendFromISR( dc_wifi_rxQueue, &(wifi_rx_usb_tx_packet.data[startIndex]), &xHigherPriorityTaskWoken) != pdTRUE ){
						// error
					}
					portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
				}
				wifi_rx_usb_tx_packet.len = 0;
				startIndex = INVALID_INDEX;
				endIndex = INVALID_INDEX;
			}
		} else {
			dc_wifi_update_finished_observer(rxTempByte);
		}

		if ( wifi_rx_usb_tx_packet.len == DC_USB_PACKET_SIZE){
			Error_Handler(); // overflow
		}

		HAL_UART_Receive_DMA(huart, &rxTempByte, 1);
	}
}
//----------------------------------------------------------------------------------------------------------------------
void dc_wifi_rxTask(void * pvParameters){
	uint8_t rxData[CTRL_PACKET_LEN];

	for(;;){
		if (dc_wifi_isBootloaderModeActive){
			TickType_t xLastWakeTime = xTaskGetTickCount();
			vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(10) );
			if ( wifi_rx_usb_tx_packet.len ){
				taskENTER_CRITICAL();
				dc_usb_packet_t tempPacket = { .len = wifi_rx_usb_tx_packet.len };
				memcpy(tempPacket.data, wifi_rx_usb_tx_packet.data, tempPacket.len);
				wifi_rx_usb_tx_packet.len = 0;
				taskEXIT_CRITICAL();
				dc_usb_send_data(&tempPacket);
			}
		} else {
			if ( xQueueReceive(dc_wifi_rxQueue, rxData, 500) == pdTRUE ) {
				dc_controller_processControlPacket(rxData, CTRL_PACKET_DATA_LEN, CTRL_SRC_WIFI);
			}
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------
void dc_wifi_txTask(void * pvParameters){
	uint8_t txData[INFO_PACKET_LEN];
	for (;;){
		if ( xQueueReceive(dc_wifi_txQueue, txData, portMAX_DELAY) == pdTRUE ) {
			if (!dc_wifi_isBootloaderModeActive){
				HAL_UART_Transmit_DMA(&huart3, txData, INFO_PACKET_LEN);
			}
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------
void dc_wifi_directTx(uint8_t * pData, uint8_t len){
	static uint8_t txBuffer[255];
	if (len > 255) {
		Error_Handler();
	}
	memcpy(txBuffer, pData, len);
	HAL_UART_Transmit_DMA(&huart3, pData, len);
}

