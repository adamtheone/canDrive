#ifndef DC_PACKET_PROCESSOR_H_
#define DC_PACKET_PROCESSOR_H_
//---------------------------------------------------------------------------------------------------------------------
#include "stm32f4xx.h"
//---------------------------------------------------------------------------------------------------------------------
#define UNKNOWN_QUERY		0xFF
#define SHOW_CURRENT_DATA	0x01
#define CURRENT_DATA_RESP	0x41
#define SHOW_DTC_CODES		0x03
#define DTC_CODES_RESP		0x43
#define ECU_ADDR			0x7DF
#define ECU_RESP_ADDR		0x7E8
//---------------------------------------------------------------------------------------------------------------------
typedef enum {
	PID_ENGINE_LOAD 				= 0x04,// A/2.55 [%]
	PID_ENGINE_COOLANT_TEMP 		= 0x05,// A-40 [C]
	PID_INTAKE_MANIFOLD_PRESSURE	= 0x0b,// A [kPa]
	PID_ENGINE_RPM					= 0x0c,// (256A+B)/4 [rpm]
	PID_VEHICLE_SPEED				= 0x0d,// A [kmph]
	PID_INTAKE_AIR_TEMP				= 0x0f,// A-40 [C]
	PID_MAF_AIRFLOW_RATE			= 0x10,// (256A+B)/100 [g/sec]
	PID_FUEL_RAIL_GAUGE_PRESSURE	= 0x23,// 10(256A + B) [kPa]
	//---------------------------------------------------------------
	PID_LS_FRONT_LEFT_DOOR_STATUS	= 0x405,
	PID_LS_FRONT_RIGHT_DOOR_STATUS	= 0x420,
	PID_LS_REAR_DOORS_STATUS		= 0x230,
	PID_LS_LIGHT_SWITCH				= 0x23A,
	PID_LS_TURNING_LIGHTS			= 0x260,
	PID_LS_PEDALS					= 0x115,
	PID_LS_HANDBRAKE				= 0x220,
	PID_LS_IMMO						= 0x621,
	PID_LS_REMOTE					= 0x160,
	//---------------------------------------------------------------
	PID_LS_INTERIOR_LIGHT			= 0x252,
	PID_LS_INSTRUMENT_CLUSTER		= 0x255,
	PID_LS_BRIGHTNESS				= 0x235,
} PID_t;

typedef enum{
	MID_SDM							= 0x257,
	MIS_DIS							= 0x246,
	MID_REC							= 0x254,
	MID_DASHBOARD					= 0x255,
	MID_COMFORTMODULE				= 0x252,
	MID_PASSANGER_SIDE_DOOR			= 0x258,
	MID_DRIVER_SIDE_DOOR			= 0x259,
	MID_UEC							= 0x253,
} modul_id_t;


enum{
	PID_NAME,
	PID_LEN,
};
//---------------------------------------------------------------------------------------------------------------------
void dc_packet_processor_print(CAN_RxHeaderTypeDef *rxHeader, uint8_t *pData);
void dc_packet_processor_ls(CAN_RxHeaderTypeDef *rxHeader, uint8_t *pData);
void dc_packet_processor_ms(CAN_RxHeaderTypeDef *rxHeader, uint8_t *pData);
void dc_packet_processor_hs(CAN_RxHeaderTypeDef *rxHeader, uint8_t *pData);
void dc_packet_processor_query_loop(void);
void dc_packet_processor_init(void);
//---------------------------------------------------------------------------------------------------------------------
#endif /* DC_PACKET_PROCESSOR_H_ */
