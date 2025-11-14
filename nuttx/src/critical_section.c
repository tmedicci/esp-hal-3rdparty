#include "esp_private/critical_section.h"

#include <nuttx/mutex.h>
#include <nuttx/spinlock.h>
#include <nuttx/irq.h>
#include <nuttx/queue.h>

#define NR_IRQSTATE_FLAGS   3

struct irqstate_list_s
{
  struct irqstate_list_s *flink;
  irqstate_t flags;
};

static bool g_lock_initialized = false;
static sq_queue_t g_int_flags_free;
static sq_queue_t g_int_flags_used;
static struct irqstate_list_s g_int_flags[NR_IRQSTATE_FLAGS];

#if OS_SPINLOCK == 1
void nuttx_enter_critical(spinlock_t *lock)
#else
void nuttx_enter_critical(void)
#endif
{
  if (!g_lock_initialized)
    {
      sq_init(&g_int_flags_free);
      sq_init(&g_int_flags_used);

      for (int i = 0; i < NR_IRQSTATE_FLAGS; i++)
      {
        sq_addlast((sq_entry_t *)&g_int_flags[i], &g_int_flags_free);
      }
      g_lock_initialized = true;
    }

  struct irqstate_list_s *irqstate;
  irqstate = (struct irqstate_list_s *)sq_remlast(&g_int_flags_free);
  assert(irqstate != NULL);
#if OS_SPINLOCK == 1
  irqstate->flags = spin_lock_irqsave(lock);
#else
  irqstate->flags = enter_critical_section();
#endif
  sq_addlast((sq_entry_t *)irqstate, &g_int_flags_used);
}

#if OS_SPINLOCK == 1
void nuttx_exit_critical(spinlock_t *lock)
#else
void nuttx_exit_critical(void)
#endif
{
  struct irqstate_list_s *irqstate;
  irqstate = (struct irqstate_list_s *)sq_remlast(&g_int_flags_used);
#if OS_SPINLOCK == 1
  assert(irqstate != NULL);
  spin_unlock_irqrestore(lock, irqstate->flags);
#else
  assert(irqstate != NULL);
  leave_critical_section(irqstate->flags);
#endif
  sq_addlast((sq_entry_t *)irqstate, &g_int_flags_free);
}
