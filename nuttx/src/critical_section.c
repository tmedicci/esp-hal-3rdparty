#include "esp_private/critical_section.h"
#include "platform/os.h"

#include <nuttx/mutex.h>
#include <nuttx/spinlock.h>
#include <nuttx/irq.h>
#include <nuttx/queue.h>

static uint8_t g_int_flags_count[OS_PORT_NUM_PROCESSORS];
static irqstate_t g_int_flags[OS_PORT_NUM_PROCESSORS];

nooptimiziation_function
#if OS_SPINLOCK == 1
void nuttx_enter_critical(rspinlock_t *lock)
#else
void nuttx_enter_critical(void)
#endif
{
  irqstate_t flags;

  int cpu = this_cpu();

#if OS_SPINLOCK == 1
  flags = rspin_lock_irqsave(lock);
#else
  flags = up_irq_save();
#endif

  if (g_int_flags_count[cpu] == 0)
    {
      /* First time acquiring this lock */

      g_int_flags[cpu] = flags;
    }

  g_int_flags_count[cpu]++;
}

nooptimiziation_function
#if OS_SPINLOCK == 1
void nuttx_exit_critical(rspinlock_t *lock)
#else
void nuttx_exit_critical(void)
#endif
{
  int cpu = this_cpu();

#if OS_SPINLOCK == 1
  rspin_unlock_irqrestore(lock, g_int_flags[cpu]);
#else
  up_irq_restore(g_int_flags[cpu]);
#endif

  g_int_flags_count[cpu]--;
}
