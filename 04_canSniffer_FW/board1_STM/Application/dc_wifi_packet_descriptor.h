#ifndef DC_WIFI_PACKET_DESCRIPTOR_H_
#define DC_WIFI_PACKET_DESCRIPTOR_H_
//---------------------------------------------------------------------------------------------------------------------

typedef enum{
	CTRL_PACKET_START 		= 0,
	//--------------------
	CTRL_VOLUME_DOWN		= 1,
	CTRL_VOLUME_UP			= 2,
	CTRL_NEXT_SONG			= 3,
	CTRL_PREV_SONG			= 4,
	CTRL_MENU_UP			= 5,
	CTRL_MENU_DOWN			= 6,
	CTRL_MENU_PUSH			= 7,
	CTRL_MENU_BACK			= 8,
	CTRL_REAR_WINDOWS		= 9,
	CTRL_FRONT_WINDOWS		= 10,
	CTRL_LEFT_MIRROR		= 11,
	CTRL_RIGHT_MIRROR		= 12,
	CTRL_METERS_TEST		= 13,
	CTRL_LIGHTS				= 14,
	CTRL_GETFAULTCODES		= 15,
	//--------------------
	CTRL_PACKET_END,
	CTRL_PACKET_LEN
} dc_wifi_control_packet_t;

#define CTRL_PACKET_START_CHAR	'['
#define CTRL_PACKET_END_CHAR	']'
#define CTRL_PACKET_DATA_LEN	(CTRL_PACKET_LEN - 2)

//---------------------------------------------------------------------------------------------------------------------

typedef enum{
	INFO_PACKET_START 		= 0,
	//--------------------
	INFO_ENGINE_LOAD		= 1,
	INFO_COOLANT_TEMP		= 2,
	INFO_INMANIF_PRESS		= 3,
	INFO_ENGINE_RPM			= 4,//5
	INFO_SPEED				= 6,
	INFO_INAIR_TEMP		 	= 7,
	INFO_MAF_RATE			= 8,//9
	INFO_FUELGAUGE_PRESS	= 10,//11
	INFO_BRIGHTNESS			= 12,
	INFO_INCREMENT			= 13,
	INFO_DOORS_STATE		= 14,// 000[L] [RR][RL][FR][FL]
	INFO_LIGHTS				= 15,// 00[RI][LI] [R][LB][P][A]
	INFO_PEDALS				= 16,// 0000 0[HB][B][C]
	INFO_FAULT1				= 17,
	INFO_FAULT2				= 18,
	INFO_FAULT3				= 19,
	INFO_VOLTAGE			= 20,
	//--------------------
	INFO_PACKET_END,
	INFO_PACKET_LEN
} dc_wifi_info_packet_t;

#define INFO_PACKET_START_CHAR	'{'
#define INFO_PACKET_END_CHAR	'}'
#define INFO_PACKET_DATA_LEN	(INFO_PACKET_LEN - 2)

#define FAULT_CODE_LEN		3
//---------------------------------------------------------------------------------------------------------------------
#endif /* DC_WIFI_PACKET_DESCRIPTOR_H_ */
