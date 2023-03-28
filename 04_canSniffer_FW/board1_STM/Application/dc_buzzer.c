#include "dc_buzzer.h"
#include "dc_main.h"
#include "main.h"
//---------------------------------------------------------------------------------------------------------------------
extern TIM_HandleTypeDef htim12;
//---------------------------------------------------------------------------------------------------------------------
void dc_buzzer_init(void){
	for (uint8_t i = 0; i < 2; i++){
		HAL_TIM_PWM_Start(&htim12, TIM_CHANNEL_2);
	  	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
	  	vTaskDelay(pdMS_TO_TICKS(50));
	  	HAL_TIM_PWM_Stop(&htim12, TIM_CHANNEL_2);
	  	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	  	vTaskDelay(pdMS_TO_TICKS(50));
	}
}

//---------------------------------------------------------------------------------------------------------------------
