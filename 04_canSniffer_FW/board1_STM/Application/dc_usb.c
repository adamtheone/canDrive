#include "dc_usb.h"
#include "dc_can.h"
#include "dc_main.h"
#include "main.h"
#include "usbd_def.h"
#include "usbd_cdc_if.h"
#include "dc_forwarder_module.h"
#include "dc_infocollector_module.h"
#include "dc_transmitter_module.h"
#include "dc_ble.h"
//---------------------------------------------------------------------------------------------------------------------
static QueueHandle_t txBufferQueue;
static QueueHandle_t rxBufferQueue;
//---------------------------------------------------------------------------------------------------------------------
void dc_usb_init(void){
	HAL_GPIO_WritePin(USB_EN_GPIO_Port, USB_EN_Pin, GPIO_PIN_SET);

	txBufferQueue = xQueueCreate( DC_USB_TX_QUEUE_SIZE, sizeof(dc_usb_packet_t));
	rxBufferQueue = xQueueCreate( DC_USB_RX_QUEUE_SIZE, sizeof(dc_usb_packet_t));
}
//---------------------------------------------------------------------------------------------------------------------
// Sending
//---------------------------------------------------------------------------------------------------------------------
void dc_usb_send_str(char * cPtr){
	dc_usb_packet_t txPacket;
	txPacket.len = strlen(cPtr) > DC_USB_PACKET_SIZE ? DC_USB_PACKET_SIZE : strlen(cPtr);
	memcpy(txPacket.data, (uint8_t*)cPtr, txPacket.len);

	dc_usb_send_data(&txPacket);
}
//---------------------------------------------------------------------------------------------------------------------
void dc_usb_send_data(const dc_usb_packet_t * txPacket){
	if (is_handler_mode()){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if ( xQueueSendFromISR( txBufferQueue, txPacket, &xHigherPriorityTaskWoken) != pdTRUE ){
			// error
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	} else {
		if ( xQueueSend(txBufferQueue, txPacket, 50) != pdTRUE ){
			// error
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------
void dc_usb_tx_task(void * pvParameters){
	dc_usb_packet_t txPacket;
	uint32_t failCounter = 0;

	for(;;){
		if ( xQueueReceive(txBufferQueue, &txPacket, portMAX_DELAY) == pdTRUE ){
			while (CDC_Transmit_FS(txPacket.data,  txPacket.len) == USBD_BUSY){
				vTaskDelay(pdMS_TO_TICKS(2));
				if (failCounter++ > 10){
					break;
				}
			}
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------
void dc_usb_rx_task(void * pvParameters){
	dc_usb_packet_t rxPacket;
	for(;;){
		if ( xQueueReceive(rxBufferQueue, &rxPacket, portMAX_DELAY) == pdTRUE ){
			// Sharing the incoming packet with the modules
			dc_ble_usb_rx(&rxPacket);
			dc_can_controller(&rxPacket);
			dc_forwarder_usb_rx(&rxPacket);
			dc_infocollector_commander(&rxPacket);
			dc_transmitter_send_command(rxPacket.data, rxPacket.len);
			dc_main_commander(&rxPacket);
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------
void dc_usb_rx_handler(uint8_t * pData, uint32_t len){
	dc_usb_packet_t rxPacket;
	if (len > DC_USB_PACKET_SIZE){
		Error_Handler();
	}

	memcpy(rxPacket.data, pData, len);
	rxPacket.len = len;

	if ( dc_ble_dfu_is_active() ){
		dc_ble_usb_rx(&rxPacket);
		return; // BLE DFU is active
	}

	if (is_handler_mode()){
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		if ( xQueueSendFromISR( rxBufferQueue, &rxPacket, &xHigherPriorityTaskWoken) != pdTRUE ){
			// error
		}
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	} else {
		if ( xQueueSend(rxBufferQueue, &rxPacket, 50) != pdTRUE ){
			// error
		}
	}
}
//---------------------------------------------------------------------------------------------------------------------
