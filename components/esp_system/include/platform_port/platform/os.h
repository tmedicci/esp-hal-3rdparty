#pragma once

#define OS_PORT_MAX_DELAY portMAX_DELAY
#define OS_PORT_TICKS_TO_MS pdMS_TO_TICKS
#define OS_BASE_TYPE BaseType_t
#define OS_FALSE pdFALSE
#define OS_TRUE pdTRUE
#define OS_PORT_NUM_PROCESSORS portNUM_PROCESSORS

typedef QueueHandle_t esp_os_queue_handle_t;
typedef TickType_t esp_os_tick_type_t;
typedef intr_handle_t esp_os_intr_handle_t;
typedef intr_handler_t esp_os_intr_handler_t;
