/*
 * SPDX-FileCopyrightText: 2022-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * This file provides an abstract OS API for entering and exiting critical sections.
 * It furthermore provides macros to define and initialize an optional spinlock
 * if the used chip is a multi-core chip. If a single-core chip is used, just disabling interrupts
 * is sufficient to guarantee consecutive, non-interrupted execution of a critical section.
 * Hence, the spinlock is unnecessary and will be automatically omitted by the macros.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "esp_err.h"
#include "platform/os.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t esp_os_intr_free(intr_handle_t handle);

esp_err_t esp_os_intr_alloc(int source, int flags, esp_os_intr_handler_t handler, void *arg, intr_handle_t *ret_handle);

esp_err_t esp_os_intr_alloc_intrstatus(int source, int flags, uint32_t intrstatusreg, uint32_t intrstatusmask, esp_os_intr_handler_t handler,
  void *arg, intr_handle_t *ret_handle);

#ifdef __cplusplus
}
#endif
