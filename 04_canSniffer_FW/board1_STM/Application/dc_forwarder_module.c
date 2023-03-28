#include "dc_forwarder_module.h"
#include "dc_main.h"
#include "dc_usb.h"
#include "dc_can_defines.h"
#include "dc_ble.h"
#include <stdlib.h>
//--------------------------------------------------------------------------------------------------------------------
static bool 		canRxForwardList[CAN_MAX_CH];
static dc_can_ch_t 	canTxForwardCh = CAN_H;
static bool			format_realdash = false;
//--------------------------------------------------------------------------------------------------------------------
void dc_forwarder_init(void){
	canRxForwardList[CAN_L] = false;
	canRxForwardList[CAN_M] = false;
	canRxForwardList[CAN_H] = false;
}
//--------------------------------------------------------------------------------------------------------------------
bool dc_forwarder_check_if_valid_usb_packet(dc_usb_packet_t * usbPacket){
	uint8_t separator_cnt = 0;

	if (usbPacket->len < 9) { return false; }

	for (uint8_t i = 0; i < usbPacket->len; i++){
		switch (usbPacket->data[i]){
			case SEPARATOR_CHAR:
				separator_cnt++;
				continue;
			case TERMINATOR_CHAR:
				if (i != usbPacket->len - 1){ return false; }
				break;
			case '0' ... '9':
			case 'A' ... 'F':
			case 'a' ... 'f':
				continue;
			default: return false;
		}
	}
	return separator_cnt == SEPARATOR_NUM && usbPacket->data[usbPacket->len - 1] == TERMINATOR_CHAR;
}
//--------------------------------------------------------------------------------------------------------------------
uint8_t dc_forwarder_parse_bytes(char ** startPtr, uint8_t *buff, uint8_t maxSize){
	uint8_t i = 0, tmp;
	char * ptr = *startPtr;
	memset(buff, 0, maxSize);
	while ( (ptr[i] != SEPARATOR_CHAR) && (ptr[i] != TERMINATOR_CHAR) && (i < 2 * maxSize) ){
		tmp = 0;
		switch (ptr[i]){
			case '0' ... '9': tmp = ptr[i] - '0'; break;
			case 'A' ... 'F': tmp = ptr[i] - 'A' + 10; break;
			case 'a' ... 'f': tmp = ptr[i] - 'a' + 10; break;
			default: break;
		}
		buff[(uint8_t)i/2] |= i % 2 ? tmp & 0x0F : tmp << 4;
		i++;
	}
	*startPtr += i; // 2nd char after last valid hex char
	return (uint8_t)i / 2;
}

//--------------------------------------------------------------------------------------------------------------------
void dc_forwarder_usb_rx(dc_usb_packet_t * usbPacket){
	if (usbPacket->data[0] == T_FORWARDER_RX_SET && usbPacket->len >= 2){
		canRxForwardList[CAN_L] = usbPacket->data[1] & 0x01 ? true : false;
		canRxForwardList[CAN_M] = usbPacket->data[1] & 0x02 ? true : false;
		canRxForwardList[CAN_H] = usbPacket->data[1] & 0x04 ? true : false;

		if (usbPacket->len >= 3){
			format_realdash = usbPacket->data[2] == 0x01;
		}
		char str[200];
		sprintf(str, "- forwarder rx L/M/H: %d %d %d\r\n", canRxForwardList[CAN_L], canRxForwardList[CAN_M], canRxForwardList[CAN_H]);
		dc_usb_send_str(str);
	}

	if (usbPacket->data[0] == T_FORWARDER_TX_SET && usbPacket->len >= 2){
		switch (usbPacket->data[1]){
			case 0: canTxForwardCh = CAN_L; break;
			case 1: canTxForwardCh = CAN_M; break;
			case 2: canTxForwardCh = CAN_H; break;
			default: break;
		}
		dc_usb_send_str("- forwarder tx setter\r\n");
	}

	if (dc_forwarder_check_if_valid_usb_packet(usbPacket)){
		dc_can_packet_t canPacket = {
				.dlc = 0,
		};

		char * ptr = NULL;
		uint8_t tmp;

		canPacket.id = (uint32_t)strtol((char*)usbPacket->data, &ptr, 16);
		if ( (ptr == NULL) || (*ptr != SEPARATOR_CHAR) ) { return; }
		ptr++;

		dc_forwarder_parse_bytes(&ptr, &tmp, 1);
		if (*ptr != SEPARATOR_CHAR) { return; } else { ptr++; }
		canPacket.rtr = tmp ? true : false;

		dc_forwarder_parse_bytes(&ptr, &tmp, 1);
		if (*ptr != SEPARATOR_CHAR) { return; } else { ptr++; }
		canPacket.ide = tmp ? true : false;

		canPacket.dlc = dc_forwarder_parse_bytes(&ptr, canPacket.data, 8);
		if (*ptr != TERMINATOR_CHAR) { return; }

		if (canPacket.dlc > 8) { return; }
		dc_can_send_packet(canTxForwardCh, canPacket.id, canPacket.rtr, canPacket.ide, canPacket.dlc, canPacket.data);
	}
}
//--------------------------------------------------------------------------------------------------------------------
void dc_forwarder_convert_can_to_dipcan(dc_can_packet_t * canPacket, dc_usb_packet_t * usbPacket){
	sprintf((char*)usbPacket->data, "%X,%02X,%02X,", (unsigned int)canPacket->id, canPacket->rtr, canPacket->ide);
	for (uint8_t i = 0; i < canPacket->dlc; i++){
		sprintf((char*)usbPacket->data + strlen((char*)usbPacket->data), "%02X", canPacket->data[i]);
	}
	sprintf((char*)usbPacket->data + strlen((char*)usbPacket->data), "\n");
	usbPacket->len = strlen((char*)usbPacket->data);
}
//--------------------------------------------------------------------------------------------------------------------
void dc_forwarder_convert_can_to_realdash(dc_can_packet_t * canPacket, dc_usb_packet_t * usbPacket){
	const uint8_t serialBlockTag[4] = { 0x44, 0x33, 0x22, 0x11 };
	memset(usbPacket->data, 0, sizeof(usbPacket->data));
	memcpy(usbPacket->data, serialBlockTag, sizeof(serialBlockTag));
	usbPacket->data[4] = (canPacket->id >> 0)  & 0xFF;
	usbPacket->data[5] = (canPacket->id >> 8)  & 0xFF;
	usbPacket->data[6] = (canPacket->id >> 16) & 0xFF;
	usbPacket->data[7] = (canPacket->id >> 24) & 0xFF;
	memcpy(&usbPacket->data[8], canPacket->data, canPacket->dlc);
	usbPacket->len = 16;
}
//--------------------------------------------------------------------------------------------------------------------
void dc_forwarder_can_rx(dc_can_packet_t * canPacket){
	if (canRxForwardList[canPacket->canCh]){
		dc_usb_packet_t usbPacket;

		if (format_realdash){
			switch (canPacket->id){
				case HS_ID_ECU_RESP:
					dc_forwarder_convert_can_to_realdash(canPacket, &usbPacket);
					dc_ble_send(usbPacket.data, usbPacket.len);
					break;
				default: break;
			}
		} else {
			dc_forwarder_convert_can_to_dipcan(canPacket, &usbPacket);
			dc_usb_send_data(&usbPacket);
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------

