#include "async_request_worker.h"

#include <esp_log.h>

#define ASYNC_WORKER_TASK_STACK_SIZE 4096
#define ASYNC_WORKER_TASK_PRIORITY configMAX_PRIORITIES - 1
#define MAX_ASYNC_REQUESTS 1

static const char *TAG = "async_request_worker";

static QueueHandle_t async_req_queue;
static SemaphoreHandle_t worker_ready_count;
static TaskHandle_t worker_handles[MAX_ASYNC_REQUESTS];

typedef struct {
    httpd_req_t* req;
    httpd_req_handler_t handler;
} httpd_async_req_t;

bool is_on_async_worker_thread(void) {
    TaskHandle_t handle = xTaskGetCurrentTaskHandle();
    for (int i = 0; i < MAX_ASYNC_REQUESTS; i++) {
        if (worker_handles[i] == handle) {
            return true;
        }
    }
    return false;
}

// Submit an HTTP req to the async worker queue
esp_err_t submit_async_req(httpd_req_t* req, httpd_req_handler_t handler) {
    // Must create a copy of the request that async worker owns
    httpd_req_t* copy = NULL;
    esp_err_t err = httpd_req_async_handler_begin(req, &copy);
    if (err != ESP_OK) {
        return err;
    }

    httpd_async_req_t async_req = {
        .req = copy,
        .handler = handler,
    };

    // Counting semaphore: if success, we know 1 or
    // more asyncReqTaskWorkers are available.
    int wait_ticks = 0;
    if (xSemaphoreTake(worker_ready_count, wait_ticks) == false) {
        ESP_LOGE(TAG, "No workers are available");
        httpd_req_async_handler_complete(copy); // Cleanup
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Putting request into worker");

    // Since worker_ready_count > 0 the queue should already have space.
    // But lets wait up to 100ms just to be safe.
    if (xQueueSend(async_req_queue, &async_req, pdMS_TO_TICKS(100)) == false) {
        ESP_LOGE(TAG, "Worker queue is full");
        httpd_req_async_handler_complete(copy); // Cleanup
        return ESP_FAIL;
    }

    return ESP_OK;
}

void async_req_worker_task(void* p) {
    ESP_LOGI(TAG, "Starting async request task worker");

    while (true) {
        // Counting semaphore - this signals that a worker
        // is ready to accept work
        xSemaphoreGive(worker_ready_count);

        // Wait for a request
        httpd_async_req_t async_req;
        if (xQueueReceive(async_req_queue, &async_req, portMAX_DELAY)) {
            ESP_LOGI(TAG, "Invoking %s", async_req.req->uri);

            // Call the handler
            async_req.handler(async_req.req);

            // Inform the server that it can purge the socket used for
            // this request, if needed.
            if (httpd_req_async_handler_complete(async_req.req) != ESP_OK) {
                ESP_LOGE(TAG, "Failed to complete async req");
            }
        }
    }

    ESP_LOGW(TAG, "Async worker stopped");
    vTaskDelete(NULL);
}

void start_async_req_workers(void) {
    // Counting semaphore keeps track of available workers
    worker_ready_count = xSemaphoreCreateCounting(
        MAX_ASYNC_REQUESTS,  // Max Count
        0); // Initial Count
    if (worker_ready_count == NULL) {
        ESP_LOGE(TAG, "Failed to create workers counting Semaphore");
        return;
    }

    // Create queue
    async_req_queue = xQueueCreate(1, sizeof(httpd_async_req_t));
    if (async_req_queue == NULL){
        ESP_LOGE(TAG, "Failed to create async_req_queue");
        vSemaphoreDelete(worker_ready_count);
        return;
    }

    // Start worker tasks
    for (int i = 0; i < MAX_ASYNC_REQUESTS; i++) {
        bool success = xTaskCreate(async_req_worker_task, "async_req_worker",
                                    ASYNC_WORKER_TASK_STACK_SIZE, // Stack size
                                    (void *)0, // Argument
                                    ASYNC_WORKER_TASK_PRIORITY, // Priority
                                    &worker_handles[i]);

        if (!success) {
            ESP_LOGE(TAG, "Failed to start async_req_worker");
            continue;
        }
    }
}
