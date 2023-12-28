#ifndef SD_CARD_READER_H_
#define SD_CARD_READER_H_

#include <esp_check.h>

esp_err_t init_sd_card(void);
void load_wifi_credentials(char* ssid, char* password);

#endif
