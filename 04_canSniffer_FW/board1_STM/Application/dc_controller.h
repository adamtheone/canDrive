#ifndef DC_CONTROLLER_H_
#define DC_CONTROLLER_H_
//---------------------------------------------------------------------------------------------------------------------
#include "stdint.h"
#include <stdbool.h>
#include "stm32f4xx.h"
//---------------------------------------------------------------------------------------------------------------------
typedef enum {
	CTRL_SRC_USB,
	CTRL_SRC_WIFI,
} dc_controller_source_t;
//---------------------------------------------------------------------------------------------------------------------
bool dc_controller_processControlPacket(uint8_t *pData, uint8_t len, dc_controller_source_t source);
bool dc_controller_fault_code_reader_callback(CAN_HandleTypeDef * canHandle, CAN_RxHeaderTypeDef *rxHeader, uint8_t *pData);
void dc_controller_task(void * pvParameters);
void dc_controller_init(void);
//---------------------------------------------------------------------------------------------------------------------
#endif /* DC_CONTROLLER_H_ */
