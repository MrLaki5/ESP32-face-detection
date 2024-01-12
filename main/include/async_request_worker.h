#ifndef ASYNC_REQUEST_WORKER_
#define ASYNC_REQUEST_WORKER_

#include <esp_http_server.h>

typedef esp_err_t (*httpd_req_handler_t)(httpd_req_t* req);

bool is_on_async_worker_thread(void);
esp_err_t submit_async_req(httpd_req_t* req, httpd_req_handler_t handler);
void start_async_req_workers(void);

#endif
