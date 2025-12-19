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

#include "platform/os.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocate an aligned chunk of memory which has the given capabilities. The initialized value in the memory is set to zero.
 *
 * @param alignment  How the pointer received needs to be aligned
 *                   must be a power of two
 * @param n    Number of continuing chunks of memory to allocate
 * @param size Size, in bytes, of a chunk of memory to allocate
 * @param caps        Bitwise OR of MALLOC_CAP_* flags indicating the type
 *                    of memory to be returned
 *
 * @return A pointer to the memory allocated on success, NULL on failure
 *
 */
void *esp_os_aligned_calloc_with_caps(size_t alignment, size_t n, size_t size, uint32_t caps);

/**
 * @brief Allocate a chunk of memory which has the given capabilities. The initialized value in the memory is set to zero.
 *
 * Equivalent semantics to libc calloc(), for capability-aware memory.
 *
 * In IDF, ``calloc(p)`` is equivalent to ``heap_caps_calloc(p, MALLOC_CAP_8BIT)``.
 *
 * @param n    Number of continuing chunks of memory to allocate
 * @param size Size, in bytes, of a chunk of memory to allocate
 * @param caps        Bitwise OR of MALLOC_CAP_* flags indicating the type
 *                    of memory to be returned
 *
 * @return A pointer to the memory allocated on success, NULL on failure
 */
void *esp_os_calloc_with_caps(size_t n, size_t size, uint32_t caps);

/**
 * @brief Allocate a chunk of memory which has the given capabilities
 *
 * Equivalent semantics to libc malloc(), for capability-aware memory.
 *
 * @param size Size, in bytes, of the amount of memory to allocate
 * @param caps        Bitwise OR of MALLOC_CAP_* flags indicating the type
 *                    of memory to be returned
 *
 * @return A pointer to the memory allocated on success, NULL on failure
 */
void *esp_os_malloc_with_caps(size_t size, uint32_t caps);

#ifdef __cplusplus
}
#endif
