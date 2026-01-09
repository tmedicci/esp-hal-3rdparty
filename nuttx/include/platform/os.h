#pragma once

#include "sdkconfig.h"
#include <nuttx/clock.h>

#include "spinlock.h"
#include "esp_intr_alloc.h"

#define OS_PORT_YIELD_FROM_ISR()
#define OS_PORT_MAX_DELAY 0xfffffffful
#define OS_PORT_TICKS_TO_MS(ticks) MSEC2TICK(ticks)
#define OS_BASE_TYPE int
#define OS_FALSE FALSE
#define OS_TRUE TRUE
#define OS_PORT_YIELD_FROM_ISR()
#define traceISR_ENTER(param1)
#define traceISR_EXIT(param1)
#define os_task_switch_is_pended(_cpu_) (false)
#define OS_PORT_NUM_PROCESSORS CONFIG_FREERTOS_NUMBER_OF_CORES

struct mq_adpt
{
  struct file mq;           /* Message queue handle */
  uint32_t msgsize;      /* Message size */
  char     name[16];     /* Message queue name */
};

struct irq_adpt
{
  void (*func)(void *arg);  /* Interrupt callback function */
  void *arg;                /* Interrupt private data */
};

typedef struct esp_os_intr_handle_t
{
  intr_handle_t *intr_handle;
  int irq;
} esp_os_intr_handle_t;

typedef struct mq_adpt *esp_os_queue_handle_t;
typedef uint32_t esp_os_tick_type_t;
typedef intr_handler_t esp_os_intr_handler_t;
