#ifndef DC_CAN_DEFINES_H_
#define DC_CAN_DEFINES_H_
//--------------------------------------------------------------------------------------------------------------------
typedef enum {
	HS_PID_ENGINE_LOAD 				= 0x04,
	HS_PID_ENGINE_COOLANT_TEMP 		= 0x05,
	HS_PID_INTAKE_MANIFOLD_PRESSURE	= 0x0b,
	HS_PID_ENGINE_RPM				= 0x0c,
	HS_PID_VEHICLE_SPEED			= 0x0d,
	HS_PID_INTAKE_AIR_TEMP			= 0x0f,
	HS_PID_MAF_AIRFLOW_RATE			= 0x10,
	HS_PID_FUEL_RAIL_GAUGE_PRESSURE	= 0x23,
} hs_pid_t;
//--------------------------------------------------------------------------------------------------------------------
typedef enum{
	HS_ECU_SHOW_CURRENT_DATA		= 0x01,
	HS_ECU_RESP_CURRENT_DATA		= 0x41,
	HS_ECU_SHOW_DTC_DATA			= 0x03,
	HS_ECU_RESP_DTC_DATA			= 0x43,
} hs_pid_response_t;
//--------------------------------------------------------------------------------------------------------------------
typedef enum{
	HS_ID_ECU						= 0x7DF,
	HS_ID_ECU_RESP					= 0x7E8,
} hs_id_t;
//--------------------------------------------------------------------------------------------------------------------
/*typedef enum{

} ms_id_t;*/
//--------------------------------------------------------------------------------------------------------------------
typedef enum{
	LS_ID_FRONT_LEFT_DOOR_STATUS	= 0x405,
	LS_ID_FRONT_RIGHT_DOOR_STATUS	= 0x420,
	LS_ID_REAR_DOORS_STATUS			= 0x230,
	LS_ID_LIGHT_SWITCH				= 0x23A,
	LS_ID_TURNING_LIGHTS			= 0x260,
	LS_ID_PEDALS					= 0x115,
	LS_ID_HANDBRAKE					= 0x220,
	LS_ID_IMMO						= 0x621,
	LS_ID_REMOTE					= 0x160,
	LS_ID_BCM						= 0x252,
	LS_ID_INSTRUMENT_CLUSTER		= 0x255,
	LS_ID_BRIGHTNESS				= 0x235,
	LS_ID_REC						= 0x254,
	LS_ID_SDM						= 0x257,

	LS_ID_UEC						= 0x253,
	LS_ID_WHEEL						= 0x175,
	LS_ID_DRIVER_DOOR_MODULE		= 0x415,
	LS_ID_FRONT_WINDOW_PASSANGER	= 0x258,
	LS_ID_FRONT_WINDOWS_DRIVER		= 0x259,
} ls_id_t;
//--------------------------------------------------------------------------------------------------------------------
typedef enum{
	FRONT_LEFT_DOOR_BITMAP			= 0x01,// bit 1.
	FRONT_RIGHT_DOOR_BITMAP			= 0x02,// bit 2.
	REAR_LEFT_DOOR_BITMAP			= 0x04,// bit 3.
	REAR_RIGHT_DOOR_BITMAP			= 0x08,// bit 4.
	DOORS_LOCKED_BITMAP				= 0x10,// bit 5.

	LIGHT_SWITCH_AUTO_BITMAP		= 0x01,// bit 1.
	LIGHT_SWITCH_PARK_BITMAP		= 0x02,// bit 2.
	LIGHT_SWITCH_LOWBEAM_BITMAP		= 0x04,// bit 3.
	TURNING_LIGHT_LEFT_BITMAP		= 0x08,// bit 4.
	TURNING_LIGHT_RIGHT_BITMAP		= 0x10,// bit 5.
	INTERIOR_LED_BRIGHTNESS_OFFSET	= 8,// bit 8-15.

	PEDALS_CLUTCH_BITMAP			= 0x01,// bit 0.
	PEDALS_BREAK_BITMAP				= 0x02,// bit 1.
	PEDALS_HANDBREAK_BITMAP			= 0x04,// bit 2.
} info_offset_t;
//--------------------------------------------------------------------------------------------------------------------
typedef enum{
	CTRL_DASHBOARD_TEST,		//0
	CTRL_COOLANT_TEMP_GAUGE,	//1
	CTRL_RPM_GAUGE,				//2
	CTRL_SPEED_GAUGE,			//3
	CTRL_FUEL_GAUGE,			//4
	CTRL_GAUGES_DEFAULT,		//5
	CTRL_TURNING_LIGHT_ON,		//6
	CTRL_LOWBEAM_LIGHT_ON,		//7
	CTRL_FOG_LIGHT_ON,			//8
	CTRL_HIGHBEAM_LIGHT_ON,		//9
	CTRL_PARK_LIGHT_ON,			//10
	CTRL_VOLUME_DOWN,			//11
	CTRL_VOLUME_UP,				//12
	CTRL_NEXT_SONG,				//13
	CTRL_PREV_SONG,				//14
	CTRL_MENU_UP,				//15
	CTRL_MENU_DOWN,				//16
	CTRL_MENU_PUSH,				//17
	CTRL_MENU_BACK,				//18
	CTRL_RL_WINDOW_UP,			//19
	CTRL_RR_WINDOW_UP,			//20
	CTRL_FL_WINDOW_UP,			//21
	CTRL_FR_WINDOW_UP,			//22
	CTRL_RL_WINDOW_DOWN,		//23
	CTRL_RR_WINDOW_DOWN,		//24
	CTRL_FL_WINDOW_DOWN,		//25
	CTRL_FR_WINDOW_DOWN,		//26

	CTRL_BCM_FAULT,				//27
	CTRL_INSTRUMENT_CLUSTER_FAULT,//28
	CTRL_DRIVER_DOOR_FAULT,		//29
	CTRL_PASSANGER_DOOR_FAULT,	//30
	CTRL_REC_FAULT,				//31
	CTRL_SDM_FAULT,				//32
	CTRL_UEC_FAULT				//33

} dc_control_t;
//--------------------------------------------------------------------------------------------------------------------
typedef struct {
	uint16_t engine_load;
	uint16_t cooland_temp;
	uint16_t manifold_pressure;
	uint16_t engine_rpm;
	uint16_t speed;
	uint16_t intake_air_temp;
	uint16_t maf_airflow_rate;
	uint16_t fuel_rail_gauge_pressure;

	uint16_t doors_state;
	uint16_t lights_state;
	uint16_t pedals_state;

	uint16_t battery_voltage;
} dc_info_struct_t;
//--------------------------------------------------------------------------------------------------------------------
typedef struct{
	char dtc[10];
} dc_dtc_struct_t;
//---------------------------------------------------------------------------------------------------------------------
#endif /* DC_CAN_DEFINES_H_ */

