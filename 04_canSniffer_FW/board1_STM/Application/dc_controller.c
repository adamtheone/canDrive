#include "dc_controller.h"
#include "dc_main.h"
#include "dc_wifi.h"
#include <string.h>
#include "dc_wifi_packet_descriptor.h"
#include "main.h"
#include "dc_usb.h"
#include "dc_can.h"
#include "dc_packet_processor.h"
//---------------------------------------------------------------------------------------------------------------------
extern QueueHandle_t 			dc_controller_faultQueue;
//---------------------------------------------------------------------------------------------------------------------
#define FAUL_CODE_READING_OFF 0xff
static uint8_t faultCodePtr = FAUL_CODE_READING_OFF;
dc_can_tx_packet_t fault_code_getter_array[] = {
		{
				.ID			= MID_COMFORTMODULE,
				.DLC		= 8,
				.data		= { 0x03, 0xA9, 0x81, 0x12, 0x00, 0x00, 0x00, 0x00},
		}, {
				.ID			= MID_DASHBOARD,
				.DLC		= 8,
				.data		= { 0x03, 0xA9, 0x81, 0x12, 0x00, 0x00, 0x00, 0x00},
		}, {
				.ID			= MID_DRIVER_SIDE_DOOR,
				.DLC		= 8,
				.data		= { 0x03, 0xA9, 0x81, 0x12, 0x00, 0x00, 0x00, 0x00},
		}, {

				.ID			= MID_PASSANGER_SIDE_DOOR,
				.DLC		= 8,
				.data		= { 0x03, 0xA9, 0x81, 0x12, 0x00, 0x00, 0x00, 0x00},
		}, {
				.ID			= MID_REC,
				.DLC		= 8,
				.data		= { 0x03, 0xA9, 0x81, 0x12, 0x00, 0x00, 0x00, 0x00},
		}, {
				.ID			= MID_SDM,
				.DLC		= 8,
				.data		= { 0x03, 0xA9, 0x81, 0x12, 0x00, 0x00, 0x00, 0x00},
		}, {
				.ID			= MID_UEC,
				.DLC		= 8,
				.data		= { 0x03, 0xA9, 0x81, 0x12, 0x00, 0x00, 0x00, 0x00},
		}
};

static uint8_t dashBoardTestPtr = 0xff;
dc_can_tx_packet_t dashboard_test_array[] = {
		{
			// COOLANT TEMP
				.ID			= MID_DASHBOARD,
				.DLC		= 8,
				.data		= { 0x07, 0xAE, 0x01, 0x08, 0x08, 0x00, 0xA7, 0x00 },
		}, {
			// RPM
				.ID			= MID_DASHBOARD,
				.DLC		= 8,
				.data		= { 0x07, 0xAE, 0x01, 0x02, 0x02, 0x00, 0x17, 0x70 },
		}, {
			// SPEED
				.ID			= MID_DASHBOARD,
				.DLC		= 8,
				.data		= { 0x07, 0xAE, 0x01, 0x01, 0x01, 0x00, 0x00, 0xF9 },
		}, {
			// FUEL
				.ID			= MID_DASHBOARD,
				.DLC		= 8,
				.data		= { 0x07, 0xAE, 0x01, 0x04, 0x04, 0x08, 0x28, 0x00 },
		}, {
			// OFF
				.ID			= MID_DASHBOARD,
				.DLC		= 8,
				.data		= { 0x07, 0xAE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
		},
};

static uint8_t lightsTestPtr = 0xff;
dc_can_tx_packet_t lights_test_array[] = {
		{
			// TURNING
				.ID			= MID_UEC,
				.DLC		= 8,
				.data		= { 0x06, 0xAE, 0x02, 0x03, 0x00, 0x03, 0x00, 0x00 },
		}, {
			// LOWBEAM
				.ID			= MID_UEC,
				.DLC		= 8,
				.data		= { 0x06, 0xAE, 0x02, 0xC0, 0x00, 0xC0, 0x00, 0x00 },
		}, {
			// FOG
				.ID			= MID_UEC,
				.DLC		= 8,
				.data		= { 0x06, 0xAE, 0x02, 0x00, 0x0C, 0x00, 0x0C, 0x00 },
		}, {
			// HIGHBEAM
				.ID			= MID_UEC,
				.DLC		= 8,
				.data		= { 0x06, 0xAE, 0x02, 0x30, 0x00, 0x30, 0x00, 0x00 },
		}, {
			// PARK
				.ID			= MID_UEC,
				.DLC		= 8,
				.data		= { 0x06, 0xAE, 0x02, 0x0C, 0x00, 0x0C, 0x00, 0x00 },
		} , /*{
			// OFF
				.ID			= MID_UEC,
				.DLC		= 8,
				.data		= { 0x06, 0xAE, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00 },
		}*/
};

dc_can_tx_packet_t control_array[] = {
		{
			// DUMMY!
				.ID			= 0x000,
				.DLC		= 1,
				.data		= {0x00},
		}, {
			// VOLUME DOWN
				.ID 		= 0x175,
				.DLC 		= 8,
				.data 		= { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x02, 0x00, 0x1F},
		}, {
			// VOLUME UP
				.ID 		= 0x175,
				.DLC 		= 8,
				.data 		= { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x01, 0x00, 0x01},
		}, {
			// NEXT SONG/STATION
				.ID 		= 0x175,
				.DLC 		= 8,
				.data 		= { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x04, 0x00, 0x00},
		}, {
			// PREVIOUS SONG/STATION
				.ID 		= 0x175,
				.DLC 		= 8,
				.data 		= { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x05, 0x00, 0x00},
		}, {
			// MENU UP
				.ID 		= 0x175,
				.DLC 		= 8,
				.data 		= { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x10, 0x1F, 0x00},
		}, {
			// MENU DOWN
				.ID 		= 0x175,
				.DLC 		= 8,
				.data 		= { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x20, 0x01, 0x00},
		}, {
			// MENU PUSH
				.ID 		= 0x175,
				.DLC 		= 8,
				.data 		= { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x30, 0x00, 0x00},
		}, {
			// MENU BACK
				.ID			= 0x175,
				.DLC		= 8,
				.data		= { 0x00, 0x00, 0x00, 0x00, 0xCC, 0x50, 0x00, 0x00},
		}, {
			// REAR WINDOWS
				.ID 		= 0x415,
				.DLC 		= 5,
				.data 		= { 0x00, 0x00, 0x02, 0x1B, 0xF0, 0x00, 0x00, 0x00},//3.byte: 02,04 - 20,40
		}, {
			// FRONT WINDOWS
				.ID 		= 0x259,//0x415,
				.DLC 		= 8,//5,
				.data 		= { 0x04, 0xAE, 0x02, 0x08, 0x00, 0x00, 0x00, 0x00},//{ 0x00, 0x02, 0x00, 0x1B, 0xF0, 0x00, 0x00, 0x00},//2.byte: 02,04 - 20,40
		}, {
			// LEFT MIRROR
				.ID 		= 0x405,
				.DLC 		= 6,
				.data 		= { 0x00, 0x02, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00},//2. byte: 01,02,04,08
		}, {
			// RIGHT MIRROR
				.ID 		= 0x405,
				.DLC 		= 6,
				.data 		= { 0x00, 0x02, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00},//2. byte: 01,02,04,08
		}, {
			// METERS_TEST
				.ID 		= PID_LS_INSTRUMENT_CLUSTER,
				.DLC 		= 8,
				.data 		= { 0x07, 0xAE, 0x01, 0x02, 0x02, 0x00, 0x17, 0x70},
		}, {
			// LIGHTS
				.ID			= PID_LS_INTERIOR_LIGHT,
				.DLC		= 8,
				.data		= { 0x07, 0xAE, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00},
		}
};
//--------------------------------------------------------------------------------------------------------------------
extern QueueHandle_t dc_wifi_txQueue;
extern ADC_HandleTypeDef hadc1;
//---------------------------------------------------------------------------------------------------------------------
void dc_controller_init(void){
	for (uint8_t i = 0; i < sizeof(fault_code_getter_array) / sizeof(fault_code_getter_array[0]); i++){
		fault_code_getter_array[i].canHandle = canHandles[CAN_L];
	}

	if (HAL_ADC_Start_IT(&hadc1) != HAL_OK ){
		Error_Handler();
	}
}
//---------------------------------------------------------------------------------------------------------------------
bool dc_controller_processControlPacket(uint8_t *pData, uint8_t len, dc_controller_source_t source){
	switch (source){
		case CTRL_SRC_USB:
			if (len == 5 && !memcmp((char*)pData, "#BLON", len)){
				dc_wifi_triggerBootloader();
				//dc_usb_sendStr("...ok");
				return true;
			}
			if (len == 6 && !memcmp((char*)pData, "#BLOFF", len)){
				NVIC_SystemReset();
				dc_wifi_normalStart();
				//dc_usb_sendStr("...ok");
				return true;
			}

			if (len == 2 && pData[0] == 'C'){
				dc_can_sendPacket(	canHandles[CAN_L],
									control_array[pData[1]-'0'].ID,
									control_array[pData[1]-'0'].DLC,
									control_array[pData[1]-'0'].data );
			}

			if (len == 1 && pData[0] == '#'){
				faultCodePtr = 0;
				dc_can_sendPacket(	fault_code_getter_array[faultCodePtr].canHandle,
									fault_code_getter_array[faultCodePtr].ID,
									fault_code_getter_array[faultCodePtr].DLC,
									fault_code_getter_array[faultCodePtr].data );
			}
			break;

		case CTRL_SRC_WIFI:
			for (uint8_t cmd = CTRL_PACKET_START + 1; cmd < CTRL_PACKET_END; cmd++){
				if (pData[cmd]){
					switch (cmd){
						case CTRL_REAR_WINDOWS:	control_array[cmd].data[2] = pData[cmd]; break;
						case CTRL_FRONT_WINDOWS:
								control_array[cmd].ID 		= pData[cmd] & 0x01 ? MID_PASSANGER_SIDE_DOOR : MID_DRIVER_SIDE_DOOR;
								control_array[cmd].data[4]	= pData[cmd] & 0xFE;
								break;
						case CTRL_LEFT_MIRROR: control_array[cmd].data[1] = pData[cmd]; break;
						case CTRL_RIGHT_MIRROR: control_array[cmd].data[1] = pData[cmd]; break;

						case CTRL_METERS_TEST:
							dashBoardTestPtr = 0;
							continue;
							break;

						case CTRL_LIGHTS:
							lightsTestPtr = 0;
							break;

						case CTRL_GETFAULTCODES:
							faultCodePtr = 0;
							dc_can_sendPacket(	fault_code_getter_array[faultCodePtr].canHandle,
												fault_code_getter_array[faultCodePtr].ID,
												fault_code_getter_array[faultCodePtr].DLC,
												fault_code_getter_array[faultCodePtr].data );
							continue;
							break;
						default: break;

					}
					if (cmd < sizeof(control_array) / sizeof(control_array[0])){
						dc_usb_packet_t packet;
						sprintf((char*)packet.data, "cmd:%02X, id:%02X\r\n", (unsigned int)cmd, (unsigned int)control_array[cmd].ID);
						packet.len = strlen((char*)packet.data);
						dc_usb_send_data(&packet);
						dc_can_sendPacket(	canHandles[CAN_L],
											control_array[cmd].ID,
											control_array[cmd].DLC,
											control_array[cmd].data );
					}
				}
			}
			break;
	}
	return false;
}
//---------------------------------------------------------------------------------------------------------------------
bool dc_controller_fault_code_reader_callback(CAN_HandleTypeDef * canHandle, CAN_RxHeaderTypeDef *rxHeader, uint8_t *pData){
	if ( 	(faultCodePtr == FAUL_CODE_READING_OFF) ||
			(faultCodePtr >= sizeof(fault_code_getter_array) / sizeof(fault_code_getter_array[0])) ||
			(canHandle != fault_code_getter_array[faultCodePtr].canHandle) ||
			(rxHeader->StdId != fault_code_getter_array[faultCodePtr].ID + 0x300)	)
	{
		return false;
	}

	if ( pData[0] == 0x81 && pData[1] == 0x00 && pData[2] == 0x00 && pData[3] == 0x00){
		if (faultCodePtr < (sizeof(fault_code_getter_array) / sizeof(fault_code_getter_array[0])) - 1){
			faultCodePtr++;
			dc_can_sendPacket(	fault_code_getter_array[faultCodePtr].canHandle,
								fault_code_getter_array[faultCodePtr].ID,
								fault_code_getter_array[faultCodePtr].DLC,
								fault_code_getter_array[faultCodePtr].data );
			return true;
		} else {
			faultCodePtr = FAUL_CODE_READING_OFF;
		}
	}

	if ( pData[0] == 0x81){
		xQueueSend(dc_controller_faultQueue, &pData[1], 0);

		return true;
	}

	return false;
}

//---------------------------------------------------------------------------------------------------------------------
void dc_initial_dashboard_test(void){
	uint8_t on[] 	= { 0x07, 0xAE, 0x01, 0x03, 0x03, 0x00, 0x17, 0x00 };
	uint8_t off[] 	= { 0x07, 0xAE, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00 };

	dc_can_sendPacket(	canHandles[CAN_L], MID_DASHBOARD, 8, on );
	vTaskDelay(pdMS_TO_TICKS(800));
	dc_can_sendPacket(	canHandles[CAN_L], MID_DASHBOARD, 8, off );
}



static uint8_t testPrescaler = 0;
void dc_controller_task(void * pvParameters){
	//-uint8_t increment = 0;
	//vTaskDelay(pdMS_TO_TICKS(2000));
	for(;;){
		TickType_t xLastWakeTime = xTaskGetTickCount();
		//HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
		dc_packet_processor_query_loop();

		// DASHBOARD TEST
		if (testPrescaler > 35 && dashBoardTestPtr < sizeof(dashboard_test_array) / sizeof(dashboard_test_array[0])){
			dc_can_sendPacket(	canHandles[CAN_L],
								dashboard_test_array[dashBoardTestPtr].ID,
								dashboard_test_array[dashBoardTestPtr].DLC,
								dashboard_test_array[dashBoardTestPtr].data );
			dashBoardTestPtr++;
			testPrescaler = 0;
		}

		// LIGHTS TEST
		if (testPrescaler > 15 && lightsTestPtr < sizeof(lights_test_array) / sizeof(lights_test_array[0])){
			dc_can_sendPacket(	canHandles[CAN_L],
								lights_test_array[lightsTestPtr].ID,
								lights_test_array[lightsTestPtr].DLC,
								lights_test_array[lightsTestPtr].data );
			lightsTestPtr++;
			testPrescaler = 0;
		}

		// USB test
//		dc_usb_packet_t usbPacket;
//		sprintf((char*)usbPacket.data, "PWR: %d\r\n", (int)adcVal);
//		usbPacket.len = strlen((char*)usbPacket.data);
//		dc_usb_sendData(&usbPacket);


		// WIFI test
//		incomingInfoArray[INFO_PACKET_START] = INFO_PACKET_START_CHAR;
//		incomingInfoArray[INFO_PACKET_END] = INFO_PACKET_END_CHAR;
//		incomingInfoArray[INFO_INCREMENT] = increment++;
//
//		xQueueSend(dc_wifi_txQueue, incomingInfoArray, 50);


		// CAN test
		//uint8_t pData[] = {increment & 0xff, increment >> 8, 0, 1, 2, 3, 4, 5};
		//dc_can_sendPacket(canHandles[CAN_H], 0x123, 8, pData);
		vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(20) );
		testPrescaler++;
	}
}
//---------------------------------------------------------------------------------------------------------------------
