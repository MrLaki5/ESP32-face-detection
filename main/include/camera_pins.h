#ifndef CAMERA_PINS_H_
#define CAMERA_PINS_H_

#include <esp_camera.h>

// Freenove ESP32-WROVER CAM Board PIN Map
#if BOARD_WROVER_KIT
#define CAM_PIN_PWDN -1
#define CAM_PIN_RESET -1
#define CAM_PIN_XCLK 21
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27
#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 19
#define CAM_PIN_D2 18
#define CAM_PIN_D1 5
#define CAM_PIN_D0 4
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22
#endif

// ESP-EYE PIN Map
#if BOARD_CAMERA_MODEL_ESP_EYE
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    4
#define SIOD_GPIO_NUM    18
#define SIOC_GPIO_NUM    23
#define Y9_GPIO_NUM      36
#define Y8_GPIO_NUM      37
#define Y7_GPIO_NUM      38
#define Y6_GPIO_NUM      39
#define Y5_GPIO_NUM      35
#define Y4_GPIO_NUM      14
#define Y3_GPIO_NUM      13
#define Y2_GPIO_NUM      34
#define VSYNC_GPIO_NUM   5
#define HREF_GPIO_NUM    27
#define PCLK_GPIO_NUM    25
#endif

// AiThinker ESP32Cam PIN Map
#if BOARD_ESP32CAM_AITHINKER
#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27
#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22
#endif

// TTGO T-Journal ESP32 Camera PIN Map
#if BOARD_CAMERA_MODEL_TTGO_T_JOURNAL
#define PWDN_GPIO_NUM      0
#define RESET_GPIO_NUM    15
#define XCLK_GPIO_NUM     27
#define SIOD_GPIO_NUM     25
#define SIOC_GPIO_NUM     23
#define Y9_GPIO_NUM       19
#define Y8_GPIO_NUM       36
#define Y7_GPIO_NUM       18
#define Y6_GPIO_NUM       39
#define Y5_GPIO_NUM        5
#define Y4_GPIO_NUM       34
#define Y3_GPIO_NUM       35
#define Y2_GPIO_NUM       17
#define VSYNC_GPIO_NUM    22
#define HREF_GPIO_NUM     26
#define PCLK_GPIO_NUM     21
#endif

void get_camera_current_resolution(framesize_t size, int* w, int* h) {
    if (!w || !h) {
        return; // Safety check for null pointers
    }

    switch(size) {
        case FRAMESIZE_96X96:    *w = 96; *h = 96; break;
        case FRAMESIZE_QQVGA:    *w = 160; *h = 120; break;
        case FRAMESIZE_QCIF:     *w = 176; *h = 144; break;
        case FRAMESIZE_HQVGA:    *w = 240; *h = 176; break;
        case FRAMESIZE_240X240:  *w = 240; *h = 240; break;
        case FRAMESIZE_QVGA:     *w = 320; *h = 240; break;
        case FRAMESIZE_CIF:      *w = 400; *h = 296; break;
        case FRAMESIZE_HVGA:     *w = 480; *h = 320; break;
        case FRAMESIZE_VGA:      *w = 640; *h = 480; break;
        case FRAMESIZE_SVGA:     *w = 800; *h = 600; break;
        case FRAMESIZE_XGA:      *w = 1024; *h = 768; break;
        case FRAMESIZE_HD:       *w = 1280; *h = 720; break;
        case FRAMESIZE_SXGA:     *w = 1280; *h = 1024; break;
        case FRAMESIZE_UXGA:     *w = 1600; *h = 1200; break;
        case FRAMESIZE_FHD:      *w = 1920; *h = 1080; break;
        case FRAMESIZE_P_HD:     *w = 720; *h = 1280; break;
        case FRAMESIZE_P_3MP:    *w = 864; *h = 1536; break;
        case FRAMESIZE_QXGA:     *w = 2048; *h = 1536; break;
        case FRAMESIZE_QHD:      *w = 2560; *h = 1440; break;
        case FRAMESIZE_WQXGA:    *w = 2560; *h = 1600; break;
        case FRAMESIZE_P_FHD:    *w = 1080; *h = 1920; break;
        case FRAMESIZE_QSXGA:    *w = 2560; *h = 1920; break;
        default:                 *w = 0; *h = 0; // Invalid Size
    }
}

#endif
