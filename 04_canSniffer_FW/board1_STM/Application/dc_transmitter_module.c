#include "dc_transmitter_module.h"
#include "dc_can_defines.h"
#include "dc_main.h"
#include "dc_can.h"
//--------------------------------------------------------------------------------------------------------------------
void dc_transmitter_send_command(uint8_t * pData, uint8_t len){
	if (pData[0] == T_TRANSMITTER_CTRL && len >= 2){
		switch(pData[1]){
			case CTRL_DASHBOARD_TEST:{
				} break;
			case CTRL_COOLANT_TEMP_GAUGE:{
				uint8_t data[] = { 0x07, 0xAE, 0x01, 0x08, 0x08, 0x00, 0xA7, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_INSTRUMENT_CLUSTER, false, false, 8, data);
				} break;
			case CTRL_RPM_GAUGE:{
				uint8_t data[] = { 0x07, 0xAE, 0x01, 0x02, 0x02, 0x00, 0x17, 0x70 };
				dc_can_send_packet(CAN_L, LS_ID_INSTRUMENT_CLUSTER, false, false, 8, data);
				} break;
			case CTRL_SPEED_GAUGE:{
				uint8_t data[] = { 0x07, 0xAE, 0x01, 0x01, 0x01, 0x00, 0x00, 0xF9 };
				dc_can_send_packet(CAN_L, LS_ID_INSTRUMENT_CLUSTER, false, false, 8, data);
				} break;
			case CTRL_FUEL_GAUGE:{
				uint8_t data[] = { 0x07, 0xAE, 0x01, 0x04, 0x04, 0x08, 0x28, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_INSTRUMENT_CLUSTER, false, false, 8, data);
				} break;
			case CTRL_GAUGES_DEFAULT:{
				uint8_t data[] = { 0x07, 0xAE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_INSTRUMENT_CLUSTER, false, false, 8, data);
				} break;

			case CTRL_TURNING_LIGHT_ON:{
				uint8_t data[] = { 0x06, 0xAE, 0x02, 0x03, 0x00, 0x03, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_UEC, false, false, 8, data);
				} break;
			case CTRL_LOWBEAM_LIGHT_ON:{
				uint8_t data[] = { 0x06, 0xAE, 0x02, 0xC0, 0x00, 0xC0, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_UEC, false, false, 8, data);
				} break;
			case CTRL_FOG_LIGHT_ON:{
				uint8_t data[] = { 0x06, 0xAE, 0x02, 0x00, 0x0C, 0x00, 0x0C, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_UEC, false, false, 8, data);
				} break;
			case CTRL_HIGHBEAM_LIGHT_ON:{
				uint8_t data[] = { 0x06, 0xAE, 0x02, 0x30, 0x00, 0x30, 0x00, 0x00};
				dc_can_send_packet(CAN_L, LS_ID_UEC, false, false, 8, data);
				} break;
			case CTRL_PARK_LIGHT_ON:{
				uint8_t data[] = { 0x06, 0xAE, 0x02, 0x0C, 0x00, 0x0C, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_UEC, false, false, 8, data);
				} break;


			case CTRL_VOLUME_DOWN:{
				uint8_t data[] = { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x02, 0x00, 0x1F };
				dc_can_send_packet(CAN_L, LS_ID_WHEEL, false, false, 8, data);
				} break;
			case CTRL_VOLUME_UP:{
				uint8_t data[] = { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x01, 0x00, 0x01 };
				dc_can_send_packet(CAN_L, LS_ID_WHEEL, false, false, 8, data);
				} break;
			case CTRL_NEXT_SONG:{
				uint8_t data[] = { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x04, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_WHEEL, false, false, 8, data);
				} break;
			case CTRL_PREV_SONG:{
				uint8_t data[] = { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x05, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_WHEEL, false, false, 8, data);
				} break;
			case CTRL_MENU_UP:{
				uint8_t data[] = { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x10, 0x1F, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_WHEEL, false, false, 8, data);
				} break;
			case CTRL_MENU_DOWN:{
				uint8_t data[] = { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x20, 0x01, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_WHEEL, false, false, 8, data);
				} break;
			case CTRL_MENU_PUSH:{
				uint8_t data[] = { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x30, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_WHEEL, false, false, 8, data);
				} break;
			case CTRL_MENU_BACK:{
				uint8_t data[] = { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x50, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_WHEEL, false, false, 8, data);
				} break;


			case CTRL_RL_WINDOW_UP:{
				uint8_t data[] = { 0x00, 0x00, 0x20, 0x1B, 0xF0, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_DRIVER_DOOR_MODULE, false, false, 8, data);
				} break;
			case CTRL_RR_WINDOW_UP:{
				uint8_t data[] = { 0x00, 0x00, 0x02, 0x1B, 0xF0, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_DRIVER_DOOR_MODULE, false, false, 8, data);
				} break;
			case CTRL_FL_WINDOW_UP:{
				uint8_t data[] = { 0x04, 0xAE, 0x02, 0x10, 0x00, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_FRONT_WINDOWS_DRIVER, false, false, 8, data);
				} break;
			case CTRL_FR_WINDOW_UP:{
				uint8_t data[] = { 0x04, 0xAE, 0x02, 0x10, 0x00, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_FRONT_WINDOW_PASSANGER, false, false, 8, data);
				} break;
			case CTRL_RL_WINDOW_DOWN:{
				uint8_t data[] = { 0x00, 0x00, 0x40, 0x1B, 0xF0, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_DRIVER_DOOR_MODULE, false, false, 8, data);
				} break;
			case CTRL_RR_WINDOW_DOWN:{
				uint8_t data[] = { 0x00, 0x00, 0x04, 0x1B, 0xF0, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_DRIVER_DOOR_MODULE, false, false, 8, data);
				} break;
			case CTRL_FL_WINDOW_DOWN:{
				uint8_t data[] = { 0x04, 0xAE, 0x02, 0x08, 0x00, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_FRONT_WINDOWS_DRIVER, false, false, 8, data);
				} break;
			case CTRL_FR_WINDOW_DOWN:{
				uint8_t data[] = { 0x04, 0xAE, 0x02, 0x08, 0x00, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_FRONT_WINDOW_PASSANGER, false, false, 8, data);
				} break;


			case CTRL_BCM_FAULT:{
				uint8_t data[] = { 0x03, 0xA9, 0x81, 0x12, 0x00, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_BCM, false, false, 8, data);
				} break;
			case CTRL_INSTRUMENT_CLUSTER_FAULT:{
				uint8_t data[] = { 0x03, 0xA9, 0x81, 0x12, 0x00, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_INSTRUMENT_CLUSTER, false, false, 8, data);
				} break;
			case CTRL_DRIVER_DOOR_FAULT:{
				uint8_t data[] = { 0x03, 0xA9, 0x81, 0x12, 0x00, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_FRONT_WINDOWS_DRIVER, false, false, 8, data);
				} break;
			case CTRL_PASSANGER_DOOR_FAULT:{
				uint8_t data[] = { 0x03, 0xA9, 0x81, 0x12, 0x00, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_FRONT_WINDOW_PASSANGER, false, false, 8, data);
				} break;
			case CTRL_REC_FAULT:{
				uint8_t data[] = { 0x03, 0xA9, 0x81, 0x12, 0x00, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_REC, false, false, 8, data);
				} break;
			case CTRL_SDM_FAULT:{
				uint8_t data[] = { 0x03, 0xA9, 0x81, 0x12, 0x00, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_SDM, false, false, 8, data);
				} break;
			case CTRL_UEC_FAULT:{
				uint8_t data[] = { 0x03, 0xA9, 0x81, 0x12, 0x00, 0x00, 0x00, 0x00 };
				dc_can_send_packet(CAN_L, LS_ID_UEC, false, false, 8, data);
				} break;
		}
	}
}
//--------------------------------------------------------------------------------------------------------------------
