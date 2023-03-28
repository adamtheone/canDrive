#ifndef DC_FORWARDER_MODULE_H_
#define DC_FORWARDER_MODULE_H_
//--------------------------------------------------------------------------------------------------------------------
#include "dc_can.h"
#include "dc_usb.h"
//--------------------------------------------------------------------------------------------------------------------
void dc_forwarder_init(void);
void dc_forwarder_usb_rx(dc_usb_packet_t * usbPacket);
void dc_forwarder_can_rx(dc_can_packet_t * canPacket);
//--------------------------------------------------------------------------------------------------------------------
#endif /* DC_FORWARDER_MODULE_H_ */
