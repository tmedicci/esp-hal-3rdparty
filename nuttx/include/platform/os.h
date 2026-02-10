#pragma once

#include "sdkconfig.h"
#include <nuttx/clock.h>
#include <nuttx/sched.h>
#include <sched/sched.h>

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
#define OS_PORT_SUSPEND_SCHEDULER() sched_lock();

/* FreeRTOS type compatibility for NuttX */
typedef uint32_t UBaseType_t;
typedef int32_t BaseType_t;

/* FreeRTOS compatibility macros for NuttX */
#define xPortGetCoreID()              this_cpu()
#define portNUM_PROCESSORS            CONFIG_FREERTOS_NUMBER_OF_CORES
#define tskNO_AFFINITY                ((1 << CONFIG_FREERTOS_NUMBER_OF_CORES) - 1)

/**
 * Get the core ID of a task (FreeRTOS compatibility).
 * In NuttX, we return tskNO_AFFINITY if the task can run on multiple cores,
 * or the specific CPU ID if pinned to a single core.
 */
static inline UBaseType_t xTaskGetCoreID(void *task)
{
#ifdef CONFIG_SMP
  FAR struct tcb_s *tcb = task ? (FAR struct tcb_s *)task : this_task();
  cpu_set_t affinity = tcb->affinity;
  int count = 0;
  int cpu_id = 0;

  for (int i = 0; i < CONFIG_FREERTOS_NUMBER_OF_CORES; i++)
    {
      if (CPU_ISSET(i, &affinity))
        {
          count++;
          cpu_id = i;
        }
    }

  /* If pinned to a single core, return that core ID */
  if (count == 1)
    {
      return cpu_id;
    }

  /* Otherwise, return tskNO_AFFINITY (task can run on any core) */
  return tskNO_AFFINITY;
#else
  /* Non-SMP builds: always return core 0 (the only core) */
  return 0;
#endif
}

static inline void __attribute__((always_inline)) esp_os_spinlock_initialize(rspinlock_t *lock)
{
  assert(lock);

  rspin_lock_init(lock);
}

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
