
#include <debug.h>
#include <errno.h>
#include <clock/clock.h>

#include <nuttx/mutex.h>
#include <nuttx/spinlock.h>
#include <nuttx/irq.h>
#include <nuttx/queue.h>
#include <nuttx/mqueue.h>
#include <nuttx/kmalloc.h>

#include "esp_irq.h"

#include "esp_private/critical_section.h"

#include "platform/os.h"

#define OS_PORT_MAX_DELAY      0xfffffffful

struct intr_handle_data_t
{
  int irq;
};

static esp_err_t esp_os_queue_send_generic(esp_os_queue_handle_t queue,
                                           void *item,
                                           uint32_t ticks,
                                           int prio);

static esp_err_t esp_os_queue_send_generic(esp_os_queue_handle_t queue,
                                           void *item,
                                           uint32_t ticks,
                                           int prio)
{
  int ret;
  struct timespec timeout;
  struct mq_adpt *mq_adpt = (struct mq_adpt *)queue;

  if (ticks == OS_PORT_MAX_DELAY || ticks == 0)
    {
      ret = file_mq_send(&mq_adpt->mq, (const char *)item,
                         mq_adpt->msgsize, prio);
      if (ret < 0)
        {
          _err("Failed to send message to mqueue error=%d\n",
               ret);
        }
    }
  else
    {
      ret = clock_gettime(CLOCK_REALTIME, &timeout);
      if (ret < 0)
        {
          _err("Failed to get time\n");
          return false;
        }

      if (ticks)
        {
          struct timespec ts;

          clock_ticks2time(&ts, ticks);
          clock_timespec_add(&timeout, &ts, &timeout);
        }

      ret = file_mq_timedsend(&mq_adpt->mq, (const char *)item,
                              mq_adpt->msgsize, prio, &timeout);
      if (ret < 0)
        {
          _err("Failed to timedsend message to mqueue error=%d\n",
               ret);
        }
    }

  return ret == 0 ? ESP_OK : ESP_FAIL;
}

static esp_err_t esp_os_queue_receive_generic(esp_os_queue_handle_t queue,
                                              void *item,
                                              uint32_t ticks)
{
  ssize_t ret;
  struct timespec timeout;
  unsigned int prio;
  struct mq_adpt *mq_adpt = (struct mq_adpt *)queue;

  if (ticks == OS_PORT_MAX_DELAY)
    {
      ret = file_mq_receive(&mq_adpt->mq, (char *)item,
                            mq_adpt->msgsize, &prio);
      if (ret < 0)
        {
          _err("Failed to receive from mqueue error=%d\n", ret);
        }
    }
  else
    {
      ret = clock_gettime(CLOCK_REALTIME, &timeout);
      if (ret < 0)
        {
          _err("Failed to get time\n");
          return false;
        }

      if (ticks)
        {
          struct timespec ts;

          clock_ticks2time(&ts, ticks);
          clock_timespec_add(&timeout, &ts, &timeout);
        }

      ret = file_mq_timedreceive(&mq_adpt->mq, (char *)item,
                                 mq_adpt->msgsize, &prio, &timeout);
      if (ret < 0)
        {
          _err("Failed to timedreceive from mqueue error=%d\n",
               ret);
        }
    }

  return ret == 0 ? ESP_OK : ESP_FAIL;
}

static int esp_os_int_adpt_cb(int irq, void *context, void *arg)
{
  struct irq_adpt *adapter = (struct irq_adpt *)arg;

  adapter->func(adapter->arg);

  return 0;
}

IRAM_ATTR void *heap_caps_calloc(size_t n, size_t size, uint32_t caps)
{
  return kmm_calloc(n, size);
}

esp_err_t esp_os_intr_free(esp_os_intr_handle_t handle)
{
  int irq = (int)handle;
  int cpuint = esp_get_cpuint(irq);

  ASSERT(cpuint != IRQ_UNMAPPED);

  up_disable_irq(irq);
  esp_teardown_irq(ESP_IRQ2SOURCE(irq), cpuint);

  return ESP_OK;
}

esp_err_t esp_os_intr_alloc_intrstatus(int source, int flags, uint32_t intrstatusreg, uint32_t intrstatusmask, esp_os_intr_handler_t handler,
  void *arg, esp_os_intr_handle_t *ret_handle)
{
  int ret;
  esp_os_intr_handle_t intr_handle;
  struct irq_adpt *adapter;
  int irq = ESP_SOURCE2IRQ(source);
  int level = esp_intr_flags_to_level(flags);
  int type = flags & ESP_INTR_FLAG_EDGE ? ESP_IRQ_TRIGGER_EDGE : ESP_IRQ_TRIGGER_LEVEL;
  int cpuint = esp_setup_irq(source, level, type);

  adapter = kmm_malloc(sizeof(struct irq_adpt));
  if (!adapter)
    {
      _err("Failed to alloc memory\n");
      return ESP_ERR_NO_MEM;
    }

  adapter->func = handler;
  adapter->arg = arg;

  ret = irq_attach(irq, esp_os_int_adpt_cb, adapter);

  if (ret != OK)
    {
      return ESP_ERR_INVALID_ARG;
    }

  intr_handle = kmm_malloc(sizeof(esp_os_intr_handle_t));
  if (!intr_handle)
    {
      _err("Failed to alloc memory\n");
      return ESP_ERR_NO_MEM;
    }

  intr_handle->irq = irq;

  *ret_handle = intr_handle;

  return ESP_OK;
}

esp_os_queue_handle_t esp_os_queue_create_with_caps(size_t max_items,
                                                    size_t item_size,
                                                    uint32_t caps)
{
  struct mq_attr attr;
  struct mq_adpt *mq_adpt;
  int ret;

  mq_adpt = kmm_malloc(sizeof(struct mq_adpt));
  if (!mq_adpt)
    {
      wlerr("Failed to kmm_malloc\n");
      return NULL;
    }

  snprintf(mq_adpt->name, sizeof(mq_adpt->name),
           "/tmp/%p", mq_adpt);

  attr.mq_maxmsg  = max_items;
  attr.mq_msgsize = item_size;
  attr.mq_curmsgs = 0;
  attr.mq_flags   = 0;

  ret = file_mq_open(&mq_adpt->mq, mq_adpt->name,
    O_RDWR | O_CREAT, 0644, &attr);
  if (ret < 0)
    {
      wlerr("Failed to create mqueue\n");
      kmm_free(mq_adpt);
      return NULL;
    }

  mq_adpt->msgsize = item_size;

  return (esp_os_queue_handle_t)mq_adpt;
}

esp_err_t esp_os_queue_send(esp_os_queue_handle_t queue, void *item, uint32_t ticks)
{
  return esp_os_queue_send_generic(queue, item, ticks, 0);
}

esp_err_t esp_os_queue_send_from_isr(esp_os_queue_handle_t queue, void *item, void *hptw)
{
  *(int *)hptw = 0;

  return esp_os_queue_send_generic(queue, item, 0, 0);
}

esp_err_t esp_os_queue_receive(esp_os_queue_handle_t queue, void *item, uint32_t ticks)
{
  return esp_os_queue_receive_generic(queue, item, ticks);
}

esp_err_t esp_os_queue_receive_from_isr(esp_os_queue_handle_t queue, void *item, void *hptw)
{
  *(int *)hptw = 0;

  return esp_os_queue_receive_generic(queue, item, 0);
}

void esp_os_queue_delete_with_caps(esp_os_queue_handle_t queue)
{
  struct mq_adpt *mq_adpt = (struct mq_adpt *)queue;

  ASSERT(file_mq_close(&mq_adpt->mq) == OK);
  ASSERT(file_mq_unlink(mq_adpt->name) == OK);
  kmm_free(mq_adpt);
}

void esp_os_queue_delete(esp_os_queue_handle_t queue)
{
  esp_os_queue_delete_with_caps(queue);
}
