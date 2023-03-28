#include "dc_main.h"
#include "main.h"
#include "dc_usb.h"
#include "dc_can.h"
#include "usb_device.h"
#include "dc_wifi.h"
#include "dc_controller.h"
#include "dc_packet_processor.h"
#include "dc_buzzer.h"
#include "dc_infocollector_module.h"
#include "dc_ble.h"
//---------------------------------------------------------------------------------------------------------------------
TaskHandle_t dc_wifi_txTaskHandle;
extern RTC_HandleTypeDef hrtc;
//---------------------------------------------------------------------------------------------------------------------
void dc_main_led_on(void){ HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); }
void dc_main_led_off(void){ HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); }
//---------------------------------------------------------------------------------------------------------------------
void JumpToBootloader(void) {
    void (*SysMemBootJump)(void);
    volatile uint32_t addr = 0x1FFF0000;
    HAL_RCC_DeInit();
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    //__disable_irq(); // <-- irq cannot be disable for USB bootloader
    __DSB(); //	ARM Cortex-M Programming Guide to Memory Barrier Instructions.
    __HAL_SYSCFG_REMAPMEMORY_SYSTEMFLASH();
    __DSB();
    __ISB();
    SysMemBootJump = (void (*)(void)) (*((uint32_t *)(addr + 4)));
    __set_MSP(*(uint32_t *)addr);
    SysMemBootJump();
}

void dc_bootloader_check_and_jump(void){
	bool jump_to_bl = false;

	HAL_PWR_EnableBkUpAccess();
	HAL_PWREx_EnableBkUpReg();

	if ( HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == BOOTLOADER_JUMP_VALUE){
		HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, BOOTLOADER_CLEAR_VALUE);
		jump_to_bl = true;
	}

	if (jump_to_bl) {
		JumpToBootloader();
	}
}

void dc_bootloader_set_and_restart(void){
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, BOOTLOADER_JUMP_VALUE);
	NVIC_SystemReset();
}
//---------------------------------------------------------------------------------------------------------------------
void dc_main_commander (dc_usb_packet_t * packet){
	if ( !IS_TARGET_IN_GROUP(packet->data[0], T_MAIN_CTRL_GROUP)) { return;	}

	switch (packet->data[0]){
		case T_MAIN_CTRL_RESTART:
			NVIC_SystemReset();
			break;
		case T_MAIN_CTRL_BOOTLOADER:
			dc_bootloader_set_and_restart();
			break;
	}
}
//---------------------------------------------------------------------------------------------------------------------
void dc_initTask( void *pvParameters ){
	// peripheral init functions
	dc_usb_init();
	//dc_wifi_init();
	//dc_controller_init();
	//dc_packet_processor_init();
	dc_buzzer_init();


	vTaskDelay(pdMS_TO_TICKS(500));

	// creating tasks
	xTaskCreate( dc_usb_tx_task,		"usbTxTask", 		configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate( dc_usb_rx_task,		"usbRxTask", 		256, NULL, 3, NULL );

	xTaskCreate( dc_can_rx_task,		"canRxTask", 		configMINIMAL_STACK_SIZE, NULL, 1, NULL );
	xTaskCreate( dc_can_tx_task, 		"canTxTask", 		configMINIMAL_STACK_SIZE, NULL, 1, NULL );

	xTaskCreate( dc_infocollector_task, "collectorTask", 	512, NULL, 1, NULL );

	xTaskCreate( dc_ble_task,			"bleTask", 			512, NULL, 1, NULL );

	//xTaskCreate( dc_controller_task, "control", 256, NULL, 3, NULL);

	dc_can_init();

	// hello world
	dc_usb_send_str("I am alive!\r\n");

	//dc_initial_dashboard_test();

	vTaskDelete(NULL);
}

