#include "dc_infocollector_module.h"
#include "dc_usb.h"
#include "dc_main.h"
//--------------------------------------------------------------------------------------------------------------------
dc_info_struct_t dcInfoStruct;
volatile bool runCollector = false;
//--------------------------------------------------------------------------------------------------------------------
uint8_t queryIndex = 0;
uint8_t hsPidArray[] = {
		HS_PID_ENGINE_LOAD,
		HS_PID_ENGINE_COOLANT_TEMP,
		HS_PID_INTAKE_MANIFOLD_PRESSURE,
		HS_PID_ENGINE_RPM,
		HS_PID_VEHICLE_SPEED,
		HS_PID_INTAKE_AIR_TEMP,
		HS_PID_MAF_AIRFLOW_RATE,
		HS_PID_FUEL_RAIL_GAUGE_PRESSURE,
};
//---------------------------------------------------------------------------------------------------------------------
extern ADC_HandleTypeDef hadc1;
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc){
	uint32_t adc = 0;
	adc = ((float)HAL_ADC_GetValue(hadc) / 4095) * 3300 * 1.0254f / (1.54f / (1.54f + 6.04f));
	dcInfoStruct.battery_voltage = adc;
	/*
	char str[20];
	sprintf(str, "batt: %d\r\n", (int)adc);
	dc_usb_send_str(str);
	*/
}
//--------------------------------------------------------------------------------------------------------------------
void dc_infocollector_print(void){
	char str[200];
	sprintf(str, "bat %d\r\ncoolant %d\r\ndoors %d\r\n", dcInfoStruct.battery_voltage, dcInfoStruct.cooland_temp, dcInfoStruct.doors_state);
	sprintf(str + strlen(str), "engine %d\r\nrpm %d\r\npress %d\r\n", dcInfoStruct.engine_load, dcInfoStruct.engine_rpm, dcInfoStruct.fuel_rail_gauge_pressure);
	dc_usb_send_str(str);
	vTaskDelay(pdMS_TO_TICKS(2));

	sprintf(str, "air %d\r\nlights %d\r\nairflow %d\r\n", dcInfoStruct.intake_air_temp, dcInfoStruct.lights_state, dcInfoStruct.maf_airflow_rate);
	sprintf(str + strlen(str), "manif %d\r\npedals %d\r\nspeed %d\r\n", dcInfoStruct.manifold_pressure, dcInfoStruct.pedals_state, dcInfoStruct.speed);
	dc_usb_send_str(str);
	vTaskDelay(pdMS_TO_TICKS(2));

	sprintf(str, "------------------------------\r\n");
	dc_usb_send_str(str);

}
//--------------------------------------------------------------------------------------------------------------------
void dc_infocollector_commander(dc_usb_packet_t * usbPacket){
	if (usbPacket->data[0] == T_INFOCOLLECTOR_CTRL && usbPacket->len >= 2){
		runCollector = usbPacket->data[1] ? true : false;
	}
}

void dc_infocollector_task(void * pvParameters){
	TickType_t xLastWakeTime = xTaskGetTickCount();

	//HAL_ADC_Start_DMA(&hadc1,  (uint32_t*)adcVal,  1);

	for(;;){
		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(100) );

		if (!runCollector) {continue;}

		//HS PID QUERY
		uint8_t buff[8] = { 2, HS_ECU_SHOW_CURRENT_DATA, hsPidArray[queryIndex] };
		dc_can_send_packet(CAN_H, HS_ID_ECU, false, false, 3, buff);
		queryIndex = queryIndex == sizeof(hsPidArray) / sizeof(hsPidArray[0]) - 1 ? 0 : queryIndex + 1;
		if (queryIndex == 0){
			dc_infocollector_print();
		}

		//ADC MEASUREMENT
		HAL_ADC_Start_IT(&hadc1);
	}
}
//--------------------------------------------------------------------------------------------------------------------
void dc_infocollector_can_rx(dc_can_packet_t * canPacket){
	switch (canPacket->canCh){
		//------------------------------------------------------------------------------------------------------------
		//------------------------------------------------------------------------------------------------------------
		case CAN_H:{
			switch (canPacket->id){
				case HS_ID_ECU_RESP:{
					uint8_t Service = canPacket->data[1],
							PID 	= canPacket->data[2],
							A 		= canPacket->data[3],
							B 		= canPacket->data[4];
					switch (Service){
						case HS_ECU_RESP_CURRENT_DATA:
							switch (PID){
								case HS_PID_ENGINE_LOAD: dcInfoStruct.engine_load = (float)A / 2.55f; break;
								case HS_PID_ENGINE_COOLANT_TEMP: dcInfoStruct.cooland_temp = A - 40; break;
								case HS_PID_INTAKE_MANIFOLD_PRESSURE: dcInfoStruct.manifold_pressure = A; break;
								case HS_PID_ENGINE_RPM: dcInfoStruct.engine_rpm = (float)(256 * A + B) / 4.0f; break;
								case HS_PID_VEHICLE_SPEED: dcInfoStruct.speed = A; break;
								case HS_PID_INTAKE_AIR_TEMP: dcInfoStruct.intake_air_temp = A - 40; break;
								case HS_PID_MAF_AIRFLOW_RATE: dcInfoStruct.maf_airflow_rate = (float)(256 * A + B) / 100.0f; break;
								case HS_PID_FUEL_RAIL_GAUGE_PRESSURE: dcInfoStruct.fuel_rail_gauge_pressure = 0.1f * (float)(256 * A + B); break;
								default: break;
							}
						break;

						case HS_ECU_RESP_DTC_DATA:{
							dc_dtc_struct_t dtc_code;
							uint8_t A6_7 = A >> 6;
							char DTCGroup;
							switch (A6_7){
								case 0: DTCGroup = 'P'; break;
								case 1: DTCGroup = 'C'; break;
								case 2: DTCGroup = 'B'; break;
								case 3: DTCGroup = 'U'; break;
								default: DTCGroup = 'X'; break;
							}
							sprintf((char*)dtc_code.dtc, "%c%d%d%d%d", DTCGroup,(A >> 4) & 0x03, A&0x0f, B >> 4, B & 0x0f);
						}break;
					}
				}break;
			}
		} break;
		//------------------------------------------------------------------------------------------------------------
		//------------------------------------------------------------------------------------------------------------
		case CAN_M:{
		} break;
		//------------------------------------------------------------------------------------------------------------
		//------------------------------------------------------------------------------------------------------------
		case CAN_L:{
			switch (canPacket->id){
				case LS_ID_FRONT_LEFT_DOOR_STATUS:
					dcInfoStruct.doors_state &= ~FRONT_LEFT_DOOR_BITMAP;
					dcInfoStruct.doors_state |= canPacket->data[2] & 0x20 ? FRONT_LEFT_DOOR_BITMAP : 0x00;
					break;

				case LS_ID_FRONT_RIGHT_DOOR_STATUS:
					dcInfoStruct.doors_state &= ~FRONT_RIGHT_DOOR_BITMAP;
					dcInfoStruct.doors_state |= canPacket->data[2] & 0x40 ? FRONT_RIGHT_DOOR_BITMAP : 0x00;
					break;

				case LS_ID_REAR_DOORS_STATUS:
					dcInfoStruct.doors_state &= ~(REAR_LEFT_DOOR_BITMAP | REAR_RIGHT_DOOR_BITMAP | DOORS_LOCKED_BITMAP);
					dcInfoStruct.doors_state |= canPacket->data[1] & 0x40 ? REAR_LEFT_DOOR_BITMAP : 0x00;
					dcInfoStruct.doors_state |= canPacket->data[1] & 0x10 ? REAR_RIGHT_DOOR_BITMAP : 0x00;
					dcInfoStruct.doors_state |= canPacket->data[0] & 0x05 ? DOORS_LOCKED_BITMAP : 0x00;
					break;

				case LS_ID_LIGHT_SWITCH:
					dcInfoStruct.lights_state &= ~(LIGHT_SWITCH_AUTO_BITMAP | LIGHT_SWITCH_PARK_BITMAP | LIGHT_SWITCH_LOWBEAM_BITMAP);
					dcInfoStruct.lights_state |= canPacket->data[0] == 0x04 ? LIGHT_SWITCH_AUTO_BITMAP : 0x00;
					dcInfoStruct.lights_state |= canPacket->data[1] == 0x20 ? LIGHT_SWITCH_PARK_BITMAP : 0x00;
					dcInfoStruct.lights_state |= canPacket->data[1] == 0xA0 ? LIGHT_SWITCH_LOWBEAM_BITMAP : 0x00;
					break;

				case LS_ID_TURNING_LIGHTS:
					dcInfoStruct.lights_state &= ~(TURNING_LIGHT_LEFT_BITMAP | TURNING_LIGHT_RIGHT_BITMAP);
					dcInfoStruct.lights_state |= canPacket->data[0] == 0x25 ? TURNING_LIGHT_LEFT_BITMAP : 0x00;
					dcInfoStruct.lights_state |= canPacket->data[0] == 0x3A ? TURNING_LIGHT_RIGHT_BITMAP : 0x00;
					break;

				case LS_ID_BRIGHTNESS:
					dcInfoStruct.lights_state =
							(dcInfoStruct.lights_state & 0xFF) | ((uint8_t)((float)canPacket->data[1] / 2.55f) << INTERIOR_LED_BRIGHTNESS_OFFSET);
					break;

				case LS_ID_PEDALS:
					dcInfoStruct.pedals_state &= ~(PEDALS_CLUTCH_BITMAP | PEDALS_BREAK_BITMAP);
					dcInfoStruct.pedals_state |= canPacket->data[0] & 0x04 ? PEDALS_CLUTCH_BITMAP : 0x00;
					dcInfoStruct.pedals_state |= canPacket->data[0] & 0x08 ? PEDALS_BREAK_BITMAP : 0x00;
					break;

				case LS_ID_HANDBRAKE:
					dcInfoStruct.pedals_state &= ~PEDALS_HANDBREAK_BITMAP;
					dcInfoStruct.pedals_state |= canPacket->data[0] == 0x18 ? PEDALS_HANDBREAK_BITMAP : 0x00;
					break;

				case LS_ID_REMOTE:
					break;

				case LS_ID_IMMO:
					break;
				}
		} break;
		//------------------------------------------------------------------------------------------------------------
		//------------------------------------------------------------------------------------------------------------
		default: break;
	}
}
//--------------------------------------------------------------------------------------------------------------------


