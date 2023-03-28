#ifndef DC_USB_H_
#define DC_USB_H_
//---------------------------------------------------------------------------------------------------------------------
#include <stdint.h>
//---------------------------------------------------------------------------------------------------------------------
#define DC_USB_RX_QUEUE_SIZE 	10
#define DC_USB_TX_QUEUE_SIZE 	20
#define DC_USB_PACKET_SIZE 		200
//---------------------------------------------------------------------------------------------------------------------
typedef struct{
	uint8_t data[DC_USB_PACKET_SIZE];
	uint8_t len;
} dc_usb_packet_t;
//---------------------------------------------------------------------------------------------------------------------
void dc_usb_init(void);
void dc_usb_send_str(char * cPtr);
void dc_usb_send_data(const dc_usb_packet_t * txPacket);
void dc_usb_rx_handler(uint8_t * pData, uint32_t len);
void dc_usb_tx_task(void * pvParameters);
void dc_usb_rx_task(void * pvParameters);
//---------------------------------------------------------------------------------------------------------------------
#endif /* DC_USB_H_ */
