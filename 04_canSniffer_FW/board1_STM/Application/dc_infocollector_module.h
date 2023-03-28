#ifndef DC_INFOCOLLECTOR_MODULE_H_
#define DC_INFOCOLLECTOR_MODULE_H_
//--------------------------------------------------------------------------------------------------------------------
#include "dc_can.h"
#include "dc_can_defines.h"
#include "dc_usb.h"
//--------------------------------------------------------------------------------------------------------------------
void dc_infocollector_can_rx(dc_can_packet_t * canPacket);
void dc_infocollector_commander(dc_usb_packet_t * usbPacket);
void dc_infocollector_task(void * pvParameters);
//--------------------------------------------------------------------------------------------------------------------
#endif /* DC_INFOCOLLECTOR_MODULE_H_ */
