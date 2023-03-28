#ifndef DC_WIFI_H_
#define DC_WIFI_H_
//---------------------------------------------------------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
//---------------------------------------------------------------------------------------------------------------------
void dc_wifi_init(void);
void dc_wifi_triggerBootloader(void);
void dc_wifi_normalStart(void);
void dc_wifi_txTask(void * pvParameters);
void dc_wifi_rxTask(void * pvParameters);
void dc_wifi_directTx(uint8_t * pData, uint8_t len);
void dc_wifi_saveToTxBuffer(uint8_t * pData, uint8_t len);
bool dc_wifi_isBootloaderMode(void);
//---------------------------------------------------------------------------------------------------------------------
#endif /* DC_WIFI_H_ */
