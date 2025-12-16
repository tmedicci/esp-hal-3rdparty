#pragma once

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

struct mq_adpt
{
  struct file mq;           /* Message queue handle */
  uint32_t    msgsize;      /* Message size */
  char        name[16];     /* Message queue name */
};

struct irq_adpt
{
  void (*func)(void *arg);  /* Interrupt callback function */
  void *arg;                /* Interrupt private data */
};

typedef struct mq_adpt *esp_os_queue_handle_t;
typedef uint32_t esp_os_tick_type_t;
typedef intr_handle_t esp_os_intr_handle_t;
typedef intr_handler_t esp_os_intr_handler_t;
