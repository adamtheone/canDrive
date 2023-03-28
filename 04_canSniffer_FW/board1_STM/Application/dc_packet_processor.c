#include "dc_packet_processor.h"
#include "dc_wifi_packet_descriptor.h"
#include "dc_usb.h"
#include "dc_can.h"
#include "dc_main.h"
#include "main.h"
#include <string.h>
#include <stdbool.h>
//--------------------------------------------------------------------------------------------------------------------
extern QueueHandle_t 	dc_wifi_txQueue;
extern ADC_HandleTypeDef hadc1;
//--------------------------------------------------------------------------------------------------------------------
#define	FAULTQUEUE_SIZE	50
QueueHandle_t 			dc_controller_faultQueue;
//--------------------------------------------------------------------------------------------------------------------
uint8_t incomingInfoArray[INFO_PACKET_LEN] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//--------------------------------------------------------------------------------------------------------------------
uint8_t pid_query_array[][2] = {
		{PID_ENGINE_LOAD,				1},
		{PID_ENGINE_COOLANT_TEMP,		1},
		{PID_INTAKE_MANIFOLD_PRESSURE, 	1},
		{PID_ENGINE_RPM, 				2},
		{PID_VEHICLE_SPEED, 			1},
		{PID_INTAKE_AIR_TEMP, 			1},
		{PID_MAF_AIRFLOW_RATE,			2},
		{PID_FUEL_RAIL_GAUGE_PRESSURE,	2},
};
//--------------------------------------------------------------------------------------------------------------------
void dc_packet_processor_init(void){
	dc_controller_faultQueue = xQueueCreate( FAULTQUEUE_SIZE, sizeof(uint8_t) * FAULT_CODE_LEN);
}

void dc_packet_processor_query_loop(void){
	static uint8_t adcPrescaler = 0;
	static uint8_t increment = 0;
	static uint8_t queryIndex = 0;

	if (queryIndex == 0){

		if (pdPASS != xQueueReceive(dc_controller_faultQueue, &incomingInfoArray[INFO_FAULT1], 0)){
			incomingInfoArray[INFO_FAULT1] = 0xff;
			incomingInfoArray[INFO_FAULT2] = 0xff;
			incomingInfoArray[INFO_FAULT3] = 0xff;
		}

		incomingInfoArray[INFO_PACKET_START] = INFO_PACKET_START_CHAR;
		incomingInfoArray[INFO_PACKET_END] = INFO_PACKET_END_CHAR;
		incomingInfoArray[INFO_INCREMENT] = increment++;

/* FOR TESTING
		incomingInfoArray[INFO_ENGINE_LOAD] = increment;
		incomingInfoArray[INFO_COOLANT_TEMP] = increment;
		incomingInfoArray[INFO_INMANIF_PRESS] = increment;
		incomingInfoArray[INFO_ENGINE_RPM] = (10*increment) & 0xff;
		incomingInfoArray[INFO_ENGINE_RPM+1] = (10*increment) >> 8;
		incomingInfoArray[INFO_SPEED] = increment;
		incomingInfoArray[INFO_INAIR_TEMP] = increment;
		incomingInfoArray[INFO_MAF_RATE] = (10*increment) & 0xff;
		incomingInfoArray[INFO_MAF_RATE+1] = (10*increment) >>8;
		incomingInfoArray[INFO_FUELGAUGE_PRESS] = (10*increment) & 0xff;
		incomingInfoArray[INFO_FUELGAUGE_PRESS+1] = (10*increment) >> 8;
		incomingInfoArray[INFO_BRIGHTNESS] = increment;
		incomingInfoArray[INFO_INCREMENT] = increment;

		incomingInfoArray[INFO_DOORS_STATE] = increment;
		incomingInfoArray[INFO_LIGHTS] = increment;
		incomingInfoArray[INFO_PEDALS] = increment;
 */

		xQueueSend(dc_wifi_txQueue, incomingInfoArray, 10);
	}

	uint8_t buff[8] = { 2, SHOW_CURRENT_DATA, pid_query_array[queryIndex][PID_NAME] };
	dc_can_sendPacket(canHandles[CAN_H], ECU_ADDR, 3, buff);
	queryIndex = queryIndex == sizeof(pid_query_array) / sizeof(pid_query_array[0]) - 1 ? 0 : queryIndex + 1;

	if (adcPrescaler++ > 50){
		HAL_ADC_Start_IT(&hadc1);
		adcPrescaler = 0;
	}
}
//--------------------------------------------------------------------------------------------------------------------
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	volatile uint32_t adc = HAL_ADC_GetValue(&hadc1);
	float voltage = 3.17f * ((float)HAL_ADC_GetValue(&hadc1) / 63.0f) * 5.89f;
	dc_usb_packet_t usbPacket;
	sprintf((char*)usbPacket.data, "PWR: %lu\r\n", adc);
	usbPacket.len = strlen((char*)usbPacket.data);
	//dc_usb_sendData(&usbPacket);

	incomingInfoArray[INFO_VOLTAGE] = (uint8_t)voltage * 10;
}
//--------------------------------------------------------------------------------------------------------------------
void dc_packet_processor_print(CAN_RxHeaderTypeDef *rxHeader, uint8_t *pData){
	//dc_usb_sendStr("incoming packet...");
	dc_usb_packet_t txPacket;

	sprintf((char*)txPacket.data, "%X,%X,%X,%X,", 	(unsigned int)rxHeader->StdId,
													(unsigned int)rxHeader->RTR,
													(unsigned int)rxHeader->IDE,
													(unsigned int)rxHeader->DLC);
	for (uint8_t i = 0; i < rxHeader->DLC; i++){
		sprintf((char*)txPacket.data + strlen((char*)txPacket.data), "%02X ", pData[i]);
	}
	sprintf((char*)txPacket.data + strlen((char*)txPacket.data), "\r\n");
	txPacket.len = strlen((char*)txPacket.data);
	dc_usb_send_data(&txPacket);
}
//---------------------------------------------------------------------------------------------------------------------
void dc_packet_processor_hs(CAN_RxHeaderTypeDef *rxHeader, uint8_t *pData){
	dc_usb_packet_t txPacket;
	if (rxHeader->StdId == ECU_RESP_ADDR){
		uint8_t Service = pData[1],	PID = pData[2], A = pData[3], B = pData[4];
		switch (Service) {

			case CURRENT_DATA_RESP:{
				bool printPacket = false;
				switch (PID){
					case PID_ENGINE_LOAD:
						incomingInfoArray[INFO_ENGINE_LOAD] = (uint16_t)(A / 2.55f);
						if (printPacket) { sprintf((char*)txPacket.data, "LOAD=%dp\r\n", incomingInfoArray[INFO_ENGINE_LOAD] ); }
						break;

					case PID_ENGINE_COOLANT_TEMP:
						incomingInfoArray[INFO_COOLANT_TEMP] = A-40;
						if (printPacket) { sprintf((char*)txPacket.data, "COOLANT=%dC\r\n", incomingInfoArray[INFO_COOLANT_TEMP] ); }
						break;

					case PID_INTAKE_MANIFOLD_PRESSURE:
						incomingInfoArray[INFO_INMANIF_PRESS] = A;
						if (printPacket) { sprintf((char*)txPacket.data, "INTAKEmanifold=%dkPa\r\n", incomingInfoArray[INFO_INMANIF_PRESS] ); }
						break;

					case PID_ENGINE_RPM:
						incomingInfoArray[INFO_ENGINE_RPM] 		= ((256 * A + B) / 4) & 0xff;
						incomingInfoArray[INFO_ENGINE_RPM+1] 	= ((256 * A + B) / 4) >> 8;
						if (printPacket) { sprintf((char*)txPacket.data, "RPM=%d\r\n", (256 * A + B) / 4 ); }
						break;

					case PID_VEHICLE_SPEED:
						incomingInfoArray[INFO_SPEED] = A;
						if (printPacket) { sprintf((char*)txPacket.data, "SPEED=%dkmph\r\n", incomingInfoArray[INFO_SPEED] ); }
						break;

					case PID_INTAKE_AIR_TEMP:
						incomingInfoArray[INFO_INAIR_TEMP] = A-40;
						if (printPacket) { sprintf((char*)txPacket.data, "INair=%dC\r\n", incomingInfoArray[INFO_INAIR_TEMP] ); }
						break;

					case PID_MAF_AIRFLOW_RATE:
						incomingInfoArray[INFO_MAF_RATE] = ((uint16_t)((256*A+B)/100.0f)) & 0x0f;
						incomingInfoArray[INFO_MAF_RATE+1] = ((uint16_t)((256*A+B)/100.0f)) >> 8;
						if (printPacket) { sprintf((char*)txPacket.data, "MAFairflowRate=%dg/s\r\n", (uint16_t)((256*A+B)/100.0f) ); }
						break;

					case PID_FUEL_RAIL_GAUGE_PRESSURE:
						incomingInfoArray[INFO_FUELGAUGE_PRESS] = ((uint16_t)(0.1f*(256*A + B))) & 0x0f;
						incomingInfoArray[INFO_FUELGAUGE_PRESS+1] = ((uint16_t)(0.1f*(256*A + B))) >> 8;
						if (printPacket) { sprintf((char*)txPacket.data, "FuelRailGaugePressure=%dbar\r\n", (uint16_t)(0.1f*(256*A + B)) ); }
						break;


					default: printPacket = false; break;
				}
				if (printPacket){
					txPacket.len = strlen((char*)txPacket.data);
					dc_usb_send_data(&txPacket);
				}
			}break;

			case DTC_CODES_RESP:{
				uint8_t A6_7 = A >> 6;
				char DTCGroup = 'X';
				switch (A6_7){
					case 0: DTCGroup = 'P'; break;
					case 1: DTCGroup = 'C'; break;
					case 2: DTCGroup = 'B'; break;
					case 3: DTCGroup = 'U'; break;
					default: break;
				}
				sprintf((char*)txPacket.data, "DTC=%c%d%d%d%d \r\n",	DTCGroup,
																		((A >> 4) & 0x03),
																		(A & 0x0f),
																		(B >> 4),
																		(B & 0x0f));
				txPacket.len = strlen((char*)txPacket.data);
				dc_usb_send_data(&txPacket);
			}break;

			default: break;
		}
	} else {
		dc_packet_processor_print(rxHeader, pData);
	}

	// TODO
}
//---------------------------------------------------------------------------------------------------------------------
void dc_packet_processor_ms(CAN_RxHeaderTypeDef *rxHeader, uint8_t *pData){
	/*
	static uint8_t rpm = 128;
	if (rxHeader->StdId == ECU_ADDR){
		uint8_t buff[8] = { 3, CURRENT_DATA_RESP, pData[2], 12, rpm++ };
		dc_can_sendPacket(canHandles[CAN_L], ECU_RESP_ADDR, 5, buff);
		dc_can_sendPacket(canHandles[CAN_M], ECU_RESP_ADDR, 5, buff);
	}*/

	//dc_packet_processor_print(rxHeader, pData);
	// TODO
}
//---------------------------------------------------------------------------------------------------------------------
void dc_initial_dashboard_test(void);
void dc_packet_processor_ls(CAN_RxHeaderTypeDef *rxHeader, uint8_t *pData){
	//dc_packet_processor_print(rxHeader, pData);
	bool printPacket = false;
	dc_usb_packet_t txPacket;
	switch (rxHeader->StdId){
		case PID_LS_FRONT_LEFT_DOOR_STATUS:
			incomingInfoArray[INFO_DOORS_STATE] &= 0xFE; // clearing bit0
			incomingInfoArray[INFO_DOORS_STATE] |= pData[2] & 0x20 ? 0x01 : 0x00;
			if (printPacket) { sprintf((char*)txPacket.data, "FrontLeftDoor=%02X\r\n", incomingInfoArray[INFO_DOORS_STATE] & 0x01 ); }
			break;

		case PID_LS_FRONT_RIGHT_DOOR_STATUS:
			incomingInfoArray[INFO_DOORS_STATE] &= 0xFD; // clearing bit1
			incomingInfoArray[INFO_DOORS_STATE] |= pData[2] & 0x40 ? 0x02 : 0x00;
			if (printPacket) { sprintf((char*)txPacket.data, "FrontRighttDoor=%02X\r\n", incomingInfoArray[INFO_DOORS_STATE] & 0x02 ); }
			break;

		case PID_LS_REAR_DOORS_STATUS:
			incomingInfoArray[INFO_DOORS_STATE] &= 0xE3; // clearing bit2 + bit3 + bit4
			incomingInfoArray[INFO_DOORS_STATE] |= pData[1] & 0x40 ? 0x04 : 0x00; // left
			incomingInfoArray[INFO_DOORS_STATE] |= pData[1] & 0x10 ? 0x08 : 0x00; // right
			incomingInfoArray[INFO_DOORS_STATE] |= pData[0] & 0x05 ? 0x10 : 0x00; // locked
			if (printPacket) { sprintf((char*)txPacket.data, "RLDoor=%02X RRDoor=%02X Locked=%02X\r\n",
					incomingInfoArray[INFO_DOORS_STATE] & 0x04,
					incomingInfoArray[INFO_DOORS_STATE] & 0x08,
					incomingInfoArray[INFO_DOORS_STATE] & 0x10); }
			break;

		case PID_LS_LIGHT_SWITCH:
			incomingInfoArray[INFO_LIGHTS] &= 0xF0;
			if (pData[0] == 0x04){ incomingInfoArray[INFO_LIGHTS] |= 0x01; } // auto
			if (pData[1] == 0x20){ incomingInfoArray[INFO_LIGHTS] |= 0x02; } // park
			if (pData[1] == 0xA0){ incomingInfoArray[INFO_LIGHTS] |= 0x04; } // lowbeam
			if (printPacket) { sprintf((char*)txPacket.data, "Lights=%02X\r\n", incomingInfoArray[INFO_LIGHTS] & 0x0F ); }
			break;

		case PID_LS_TURNING_LIGHTS:
			incomingInfoArray[INFO_LIGHTS] &= 0x0F;
			incomingInfoArray[INFO_LIGHTS] |= pData[0] == 0x25 ? 0x10 : 0x00;
			incomingInfoArray[INFO_LIGHTS] |= pData[0] == 0x3A ? 0x20 : 0x00;
			if (printPacket) { sprintf((char*)txPacket.data, "TurningLights=%02X\r\n", incomingInfoArray[INFO_LIGHTS] & 0xF0 ); }
			break;

		case PID_LS_PEDALS:
			incomingInfoArray[INFO_PEDALS] &= 0xFC; // clearing bit0 + bit1
			incomingInfoArray[INFO_PEDALS] |= pData[0] & 0x04 ? 0x01 : 0x00;
			incomingInfoArray[INFO_PEDALS] |= pData[0] & 0x08 ? 0x02 : 0x00;
			if (printPacket) { sprintf((char*)txPacket.data, "Pedals=%02X\r\n", incomingInfoArray[INFO_PEDALS] & 0x03 ); }
			break;

		case PID_LS_HANDBRAKE:
			incomingInfoArray[INFO_PEDALS] &= 0xFB; // clearing bit2
			incomingInfoArray[INFO_PEDALS] |= pData[0] == 0x18 ? 0x04 : 0x00;
			if (printPacket) { sprintf((char*)txPacket.data, "HandBrake=%02X\r\n", incomingInfoArray[INFO_PEDALS] & 0x04 ); }
			break;

		case PID_LS_BRIGHTNESS:
			incomingInfoArray[INFO_BRIGHTNESS] = pData[1];
			if (printPacket) { sprintf((char*)txPacket.data, "Brightness=%02X\r\n", incomingInfoArray[INFO_BRIGHTNESS]); }
			break;

		case PID_LS_IMMO:{
			static bool ignPrep = false;
			static uint8_t cntr = 0;
			if (pData[1] == 0x48 && pData[2] == 0x50 && ignPrep == true){
				if (cntr > 5){
					ignPrep = false;
					dc_initial_dashboard_test();
					dc_usb_send_str("dash\r\n");
				} else {
					cntr++;
				}

			}
			if (pData[1] == 0x20 && ignPrep == false){
				ignPrep = true;
				cntr = 0;
			}

			}break;

		case PID_LS_REMOTE:{
			if (pData[0]==0x01 && pData[1]==0x20 && pData[2]==0x7D && pData[3]==0x28){

			}
		}break;

		default: break;

	}
	if (printPacket){
		txPacket.len = strlen((char*)txPacket.data);
		dc_usb_send_data(&txPacket);
	}
}
