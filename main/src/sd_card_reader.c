#include "sd_card_reader.h"

#include <driver/sdmmc_host.h>
#include <driver/sdmmc_defs.h>
#include <sdmmc_cmd.h>
#include <esp_vfs_fat.h>
#include <esp_log.h>

static const char *TAG = "sd_card_reader";

esp_err_t init_sd_card(void) {
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 3,
    };

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    
    sdmmc_card_t *card;
    esp_err_t err = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (err != ESP_OK) {
        return err;
    }

    return ESP_OK;
}

void load_wifi_credentials(char* ssid, char* password) {
    FILE* f = fopen("/sdcard/wifi.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open configuration file, please insert SD card with wifi.txt file");
        return;
    }

    // Read SSID and password from file
    fgets(ssid, 32, f);
    fgets(password, 64, f);
    fclose(f);

    // Remove trailing newline characters
    for (int i = 0; i < 32; i++) {
        if (ssid[i] == '\n') {
            ssid[i] = '\0';
            break;
        }
    }
    for (int i = 0; i < 64; i++) {
        if (password[i] == '\n') {
            password[i] = '\0';
            break;
        }
    }
}
