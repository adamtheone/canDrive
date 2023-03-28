#ifndef DC_MAIN_H_
#define DC_MAIN_H_
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "printf-stdarg.h"
#include <stdbool.h>
#include <string.h>
#include "dc_usb.h"
//---------------------------------------------------------------------------------------------------------------------
#define BOOTLOADER_JUMP_VALUE	0xBEEF
#define BOOTLOADER_CLEAR_VALUE	0x0000
//---------------------------------------------------------------------------------------------------------------------
#define IS_VALID_HEX_CHAR(x) ((x >= '0' && x <= '9') || (x >= 'A' && x <= 'F') || (x >= 'a' && x <= 'f'))
#define SEPARATOR_CHAR	(',')
#define TERMINATOR_CHAR	('\n')
#define SEPARATOR_NUM	(3)
#define MIN_LEN			(10) // 0A,01,00,\n
//--------------------------------------------------------------------------------------------------------------------
typedef enum{
	T_TARGET_MASK			= 0xF0,

	T_MAIN_CTRL_GROUP		= 0xA0,
	T_MAIN_CTRL_RESTART		= 0xA1,
	T_MAIN_CTRL_BOOTLOADER	= 0xA2,

	T_FORWARDER_GROUP		= 0xB0,
	T_FORWARDER_RX_SET 		= 0xB1,
	T_FORWARDER_TX_SET		= 0xB2,

	T_TRANSMITTER_CTRL		= 0xC0,

	T_INFOCOLLECTOR_CTRL 	= 0xD0,

	T_CAN_CTRL_GROUP		= 0xE0,
	T_CAN_CTRL_SET_TIMING	= 0xE1,
	T_CAN_CTRL_START		= 0xE2,
	T_CAN_CTRL_STOP			= 0xE3,

} dc_target_t;
//---------------------------------------------------------------------------------------------------------------------
#define IS_TARGET_IN_GROUP(target,group) ((target & T_TARGET_MASK) == group)
#define GET_TARGET(x) (x->data[0])
//---------------------------------------------------------------------------------------------------------------------
void dc_bootloader_check_and_jump(void);
void dc_bootloader_set_and_restart(void);

void dc_initTask( void *pvParameters );
void dc_main_commander (dc_usb_packet_t * packet);

void dc_main_led_on(void);
void dc_main_led_off(void);
//---------------------------------------------------------------------------------------------------------------------
#endif /* DC_MAIN_H_ */
