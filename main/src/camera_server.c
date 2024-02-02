// Select desired board from the list below
// #define BOARD_WROVER_KIT 1
// #define BOARD_CAMERA_MODEL_ESP_EYE 1
#define BOARD_ESP32CAM_AITHINKER 1
// #define BOARD_CAMERA_MODEL_TTGO_T_JOURNAL 1

#include <nvs_flash.h>
#include <esp_http_server.h>
#include <esp_timer.h>
#include <driver/gpio.h>

#include "web_pages.h"
#include "camera_pins.h"
#include "wifi_connection.h"
#include "sd_card_reader.h"
#include "async_request_worker.h"
#include <esp_log.h>
#include <stddef.h>
#include "face_detector.h"

#define LED_GPIO_PIN 4 // GPIO 4 for the onboard LED
static int led_status = 0;

static const char *TAG = "camera_server";
static int64_t frame_time_ms = 0;
static int camera_w = 0;
static int camera_h = 0;

// Web stream metadata
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n"; 

static esp_err_t init_camera(void) {
    camera_config_t camera_config = {
        .pin_pwdn  = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        .xclk_freq_hz = 10000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_RGB565,
        .frame_size = FRAMESIZE_QVGA,

        .jpeg_quality = 12,
        .fb_count = 2,
        .fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY
    };
    
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        return err;
    }

    // Get current camera resolution
    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
        ESP_LOGE(TAG, "Camera sensor not found");
        return ESP_FAIL;
    }
    get_camera_current_resolution(s->status.framesize, &camera_w, &camera_h);
    ESP_LOGI(TAG, "Current resolution: %dx%d", camera_w, camera_h);

    return ESP_OK;
}

// Initialize GPIO pins
esp_err_t init_pins(void) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    io_conf.pin_bit_mask = 1ULL << LED_GPIO_PIN;
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 0;
    return gpio_config(&io_conf);
}


// Flip on/off LED
void flip_led(void) {
    led_status = !led_status;
    gpio_set_level(LED_GPIO_PIN, led_status);
}

// API handler
esp_err_t get_index_handler(httpd_req_t* req) {
    /* Send a simple response */
    httpd_resp_send(req, index_page, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// Send data to websocket
void ws_send_data(httpd_req_t* req, char* msg, int len) {
    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.len = len;
    ws_pkt.payload = (uint8_t*)msg;
    esp_err_t ret = httpd_ws_send_frame(req, &ws_pkt);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
    }
}

// Camera streamer handler
esp_err_t jpg_stream_httpd_handler(httpd_req_t *req)
{
    // This handler is first invoked on the httpd thread.
    // In order to free the httpd thread to handle other requests,
    // we must resubmit our request to be handled on an async worker thread.
    if (is_on_async_worker_thread() == false) {

        // submit
        if (submit_async_req(req, jpg_stream_httpd_handler) == ESP_OK) {
            return ESP_OK;
        } else {
            httpd_resp_set_status(req, "503 Busy");
            httpd_resp_sendstr(req, "<div>Camera busy, workers not available.</div>");
            return ESP_OK;
        }
    }

    // Start camera streaming
    camera_fb_t* fb = NULL;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len;
    uint8_t* _jpg_buf;
    char* part_buf[64];
    static int64_t last_frame = 0;
    if(!last_frame) {
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }

    while(true){
        // Capture camera frame
        fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
            break;
        }

        // Run inference
        inference_face_detection((uint16_t*)fb->buf, (int)fb->width, (int)fb->height, 3);

        // Convert to jpeg if needed
        if(fb->format != PIXFORMAT_JPEG) {
            bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
            if(!jpeg_converted) {
                ESP_LOGE(TAG, "JPEG compression failed");
                esp_camera_fb_return(fb);
                res = ESP_FAIL;
            }
        } 
        else {
            _jpg_buf_len = fb->len;
            _jpg_buf = fb->buf;
        }

        // Stream captured data
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 64, _STREAM_PART, _jpg_buf_len);

            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }

        // Free captured data
        if(fb->format != PIXFORMAT_JPEG){
            free(_jpg_buf);
        }
        esp_camera_fb_return(fb);
        if(res != ESP_OK) {
            break;
        }

        // Calculate frame rate
        int64_t fr_end = esp_timer_get_time();
        frame_time_ms = (fr_end - last_frame) / 1000;
        last_frame = fr_end;
    }

    ESP_LOGI(TAG, "Camera streaming stopped");
    last_frame = 0;
    frame_time_ms = 0;
    return res;
}

esp_err_t handle_ws_req(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    ESP_LOGI(TAG, "Received data from websocket");

    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    }
    ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);
    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
        strcmp((char*)ws_pkt.payload,"flip_flash") == 0) {
        flip_led();
    }

    // Send framerate to websocket
    char msg[64];
    sprintf(msg, "{\"ms_time\": %lld, \"led\": %d, \"resolution\": \"%dx%d\"}", 
            frame_time_ms, led_status, camera_w, camera_h);
    ws_send_data(req, msg, strlen(msg));
    free(buf);
    return ret;
}

// Setup HTTP server
httpd_uri_t uri_get = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = jpg_stream_httpd_handler,
    .user_ctx = NULL
};

// Websocket handler
httpd_uri_t ws_uri = {
    .uri = "/ws",
    .method = HTTP_GET,
    .handler = handle_ws_req,
    .user_ctx = NULL,
    .is_websocket = true
};

// Setup HTTP server
httpd_uri_t uri_index_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_index_handler,
    .user_ctx = NULL
};

httpd_handle_t setup_server(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t stream_httpd = NULL;

    // Start the httpd server and register handlers
    if (httpd_start(&stream_httpd , &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &uri_get);
        httpd_register_uri_handler(stream_httpd, &uri_index_get);
        httpd_register_uri_handler(stream_httpd, &ws_uri);
    }

    return stream_httpd;
}

void app_main() {
    esp_err_t err;

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    // Initialize SD card
    err = init_sd_card();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "SD card init error: %s", esp_err_to_name(err));
        return;
    }

    // Load wifi credentials from SD card
    char ssid[32];
    char password[64];
    load_wifi_credentials(ssid, password);

    // Connect to wifi
    connect_wifi(ssid, password);
    if (wifi_connect_status) {
        // Initialize camera
        err = init_camera();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to init camera: %s", esp_err_to_name(err));
            return;
        }

        // Initialize pins
        err = init_pins();
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Failed to init pins: %s", esp_err_to_name(err));
            return;
        }
        // Set LED pin to low, disabled
        gpio_set_level(LED_GPIO_PIN, led_status);

        // Start web server
        start_async_req_workers();
        setup_server();
        ESP_LOGI(TAG, "ESP32 Web Camera Streaming Server is up and running");
    }
    else {
        ESP_LOGI(TAG, "Failed to connected to Wifi, check your network credentials");
    }
}
