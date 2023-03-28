#ifndef DC_BLE_H_
#define DC_BLE_H_
//---------------------------------------------------------------------------------------------------------------------
#include "dc_usb.h"
#include "main.h"
//---------------------------------------------------------------------------------------------------------------------
void dc_ble_task(void * pvParameters);
void dc_ble_usb_rx(dc_usb_packet_t *rxPacket);
bool dc_ble_dfu_is_active(void);
HAL_StatusTypeDef dc_ble_send(uint8_t * pData, uint8_t len);
//---------------------------------------------------------------------------------------------------------------------
#endif /* DC_BLE_H_ */
