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

/**
 * @brief This macro declares a queue handle as a member of a struct.
 *
 * @note When using this macro, the critical section macros esp_os_enter_critical* and esp_os_exit_critical*
 *       MUST be used, otherwise normal functions would be passed an undefined variable when build for single-core
 *       systems.
 * @note Do NOT add any semicolon after declaring the member with this macro.
 *       The trailing semicolon is included in the macro, otherwise -Wpedantic would complain about
 *       superfluous ";".
 *
 * Example usage:
 * @code{c}
 * ...
 * #include "os/queue.h"
 * ...
 * typedef struct protected_struct_t {
 *     int member1;
 *     DECLARE_QUEUE_IN_STRUCT(my_queue) // no semicolon!
 *     int another_member;
 * };
 * @endcode
 */
#define DECLARE_QUEUE_IN_STRUCT(queue_name) esp_os_queue_handle_t queue_name;

esp_os_queue_handle_t esp_os_queue_create_with_caps(size_t max_items, size_t item_size, uint32_t caps);

esp_err_t esp_os_queue_send(esp_os_queue_handle_t queue, void *item, uint32_t ticks);

esp_err_t esp_os_queue_send_from_isr(esp_os_queue_handle_t queue, void *item, void *hptw);

esp_err_t esp_os_queue_receive(esp_os_queue_handle_t queue, void *item, uint32_t ticks);

esp_err_t esp_os_queue_receive_from_isr(esp_os_queue_handle_t queue, void *item, void *hptw);

void esp_os_queue_delete_with_caps(esp_os_queue_handle_t queue);

void esp_os_queue_delete(esp_os_queue_handle_t queue);

#ifdef __cplusplus
}
#endif
