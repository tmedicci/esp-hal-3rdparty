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

#if !NON_OS_BUILD
#ifndef __NuttX__
#include "freertos/FreeRTOS.h"
#endif
#endif
#include "spinlock.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * In theory, OS_SPINLOCK should only be defined in a multi-core environment, because critical-section-related
 * functions there take a lock as a parameter. In practice, since the Xtensa FreeRTOS port layer is the same
 * for the ESP32, S3, and S2, the latter also requires a parameter in its critical-section-related functions.
 */
#if (!CONFIG_FREERTOS_UNICORE || CONFIG_IDF_TARGET_ARCH_XTENSA) && !NON_OS_BUILD
/**
 * This macro also helps users switching between spinlock declarations/definitions for multi-/single core environments
 * if the macros below aren't sufficient.
 */
#define OS_SPINLOCK 1
#else
#define OS_SPINLOCK 0
#endif

#if OS_SPINLOCK == 1
typedef spinlock_t esp_os_spinlock_t;
#endif

#ifdef __NuttX__
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

#define ENTER_CRITICAL_SECTION(lock) do { \
        if (!g_lock_initialized) { \
            sq_init(&g_int_flags_free); \
            sq_init(&g_int_flags_used); \
            for (int i = 0; i < NR_IRQSTATE_FLAGS; i++) { \
                sq_addlast((sq_entry_t *)&g_int_flags[i], &g_int_flags_free); \
            } \
            g_lock_initialized = true; \
        } \
        struct irqstate_list_s *irqstate; \
        irqstate = (struct irqstate_list_s *)sq_remlast(&g_int_flags_free); \
        assert(irqstate != NULL); \
        irqstate->flags = spin_lock_irqsave(lock); \
        sq_addlast((sq_entry_t *)irqstate, &g_int_flags_used); \
    } while(0)

#define EXIT_CRITICAL_SECTION(lock) do { \
        struct irqstate_list_s *irqstate; \
        irqstate = (struct irqstate_list_s *)sq_remlast(&g_int_flags_used); \
        assert(irqstate != NULL); \
        spin_unlock_irqrestore((lock), irqstate->flags); \
        sq_addlast((sq_entry_t *)irqstate, &g_int_flags_free); \
    } while(0)
#else
#define ENTER_CRITICAL_SECTION(lock) do { \
    if (!g_lock_initialized) { \
        sq_init(&g_int_flags_free); \
        sq_init(&g_int_flags_used); \
        for (int i = 0; i < NR_IRQSTATE_FLAGS; i++) { \
            sq_addlast((sq_entry_t *)&g_int_flags[i], &g_int_flags_free); \
        } \
        g_lock_initialized = true; \
    } \
    struct irqstate_list_s *irqstate; \
    irqstate = (struct irqstate_list_s *)sq_remlast(&g_int_flags_free); \
    assert(irqstate != NULL); \
    irqstate->flags = enter_critical_section(); \
    sq_addlast((sq_entry_t *)irqstate, &g_int_flags_used); \
} while(0)

#define EXIT_CRITICAL_SECTION(lock) do { \
    struct irqstate_list_s *irqstate; \
    irqstate = (struct irqstate_list_s *)sq_remlast(&g_int_flags_used); \
    assert(irqstate != NULL); \
    leave_critical_section(irqstate->flags); \
    sq_addlast((sq_entry_t *)irqstate, &g_int_flags_free); \
} while(0)
#endif
#endif

/**
 * Define and initialize a static (internal linking) lock for entering critical sections.
 *
 * Use this when all the critical sections are local inside a file.
 * The lock will only be defined if built for a multi-core system, otherwise it is unnecessary.
 *
 * @note When using this macro, the critical section macros esp_os_enter_critical* and esp_os_exit_critical*
 * MUST be used, otherwise normal functions would be passed an undefined variable when build for single-core systems.
 *
 * @param lock_name Variable name of the lock. This will later be used to reference the declared lock.
 * @param optional_qualifiers Qualifiers such as DRAM_ATTR and other attributes. Can be omitted if no qualifiers are
 *        required.
 *
 * Example usage:
 * @code{c}
 * ...
 * #include "os/critical_section.h"
 * ...
 * DEFINE_CRIT_SECTION_LOCK_STATIC(my_lock); // will have internal linking (static)
 * ...
 * esp_os_enter_critical(&my_lock);
 * ...
 * esp_os_exit_critical(&my_lock);
 * @endcode
 */
#if OS_SPINLOCK == 1
#define DEFINE_CRIT_SECTION_LOCK_STATIC(lock_name, optional_qualifiers...) static optional_qualifiers esp_os_spinlock_t lock_name = SPINLOCK_INITIALIZER
#else
#define DEFINE_CRIT_SECTION_LOCK_STATIC(lock_name, optional_qualifiers...)
#endif

/**
 * Define and initialize a non-static (external linking) lock for entering critical sections.
 *
 * Locks defined by this macro can be linked among object files but this rather exceptional.
 * Prefer the static lock definition whenever possible.
 * The lock will only be defined if built for a multi-core system, otherwise it is unnecessary.
 *
 * @note When using this macro, the critical section macros esp_os_enter_critical* and esp_os_exit_critical*
 * MUST be used, otherwise normal functions would be passed an undefined variable when build for single-core systems.
 *
 * @param lock_name Variable name of the lock. This will later be used to reference the declared lock.
 * @param optional_qualifiers Qualifiers such as DRAM_ATTR and other attributes. Can be omitted if no qualifiers are
 *        required.
 *
 * Example usage:
 * @code{c}
 * ...
 * #include "os/critical_section.h"
 * ...
 * DEFINE_CRIT_SECTION_LOCK(my_lock); // will have external linking (non-static)
 * ...
 * esp_os_enter_critical(&my_lock);
 * ...
 * esp_os_exit_critical(&my_lock);
 * @endcode
 */
#if OS_SPINLOCK == 1
#define DEFINE_CRIT_SECTION_LOCK(lock_name, optional_qualifiers...) optional_qualifiers esp_os_spinlock_t lock_name = SPINLOCK_INITIALIZER
#else
#define DEFINE_CRIT_SECTION_LOCK(lock_name, optional_qualifiers...)
#endif

/**
 * @brief This macro initializes a critical section lock at runtime.
 *
 * This macro basically creates a member of the initialization list, including the trailing comma.
 * If the lock is unnecessary because the architecture is single-core, this macro will not do anything.
 * This is incompatible with a lock created by DEFINE_CRIT_SECTION_LOCK_STATIC from above.
 *
 * @param lock_name Pointer to the lock.
 *
 * @note When using this macro, the critical section macros esp_os_enter_critical* and esp_os_exit_critical*
 *       MUST be used, otherwise normal functions would be passed an undefined variable when build for single-core
 *       systems.
 *
 * Example usage:
 * @code{c}
 * ...
 * #include "os/critical_section.h"
 * ...
 * typedef struct protected_struct_t {
 *     int member1;
 *     DECLARE_CRIT_SECTION_LOCK_IN_STRUCT(my_lock)
 *     int another_member;
 * };
 * ...
 * protected_struct_t my_protected;
 * INIT_CRIT_SECTION_LOCK_IN_STRUCT(&(my_protected.my_lock));
 * };
 * @endcode
 */
#if OS_SPINLOCK == 1
#define INIT_CRIT_SECTION_LOCK_RUNTIME(lock_name) spinlock_initialize(lock_name)
#else
#define INIT_CRIT_SECTION_LOCK_RUNTIME(lock_name)
#endif

/**
 * @brief This macro declares a critical section lock as a member of a struct.
 *
 * The critical section lock member is only declared if built for multi-core systems, otherwise it is omitted.
 *
 * @note When using this macro, the critical section macros esp_os_enter_critical* and esp_os_exit_critical*
 *       MUST be used, otherwise normal functions would be passed an undefined variable when build for single-core
 *       systems.
 * @note Do NOT add any semicolon after declaring the member with this macro.
 *       The trailing semicolon is included in the macro, otherwise -Wpedantic would complain about
 *       superfluous ";" if OS_SPINLOCK == 0.
 *
 * Example usage:
 * @code{c}
 * ...
 * #include "os/critical_section.h"
 * ...
 * typedef struct protected_struct_t {
 *     int member1;
 *     DECLARE_CRIT_SECTION_LOCK_IN_STRUCT(my_lock) // no semicolon!
 *     int another_member;
 * };
 * @endcode
 */
#if OS_SPINLOCK == 1
#define DECLARE_CRIT_SECTION_LOCK_IN_STRUCT(lock_name) esp_os_spinlock_t lock_name;
#else
#define DECLARE_CRIT_SECTION_LOCK_IN_STRUCT(lock_name)
#endif

/**
 * @brief This macro initializes a critical section lock as a member of a struct when using an list initialization.
 *        It has to be used together with \c DECLARE_CRIT_SECTION_LOCK_IN_STRUCT() to work.
 *
 * This macro basically creates a member of the initialization list, including the trailing comma.
 * If the lock is unnecessary because the architecture is single-core, this macro will not do anything.
 * This means that if \c lock_name is still a member of the struct, \c lock_name will be uninitialized.
 * Hence, this macro has to be used together with \c DECLARE_CRIT_SECTION_LOCK_IN_STRUCT() to correctly to declare
 * or omit the struct member \c lock_name.
 *
 * @param lock_name The field name of the lock inside the struct.
 *
 * @note When using this macro, the critical section macros esp_os_enter_critical* and esp_os_exit_critical*
 *       MUST be used, otherwise normal functions would be passed an undefined variable when build for single-core
 *       systems.
 * @note Do NOT add any comma in the initializer list after using this macro.
 *
 * Example usage:
 * @code{c}
 * ...
 * #include "os/critical_section.h"
 * ...
 * typedef struct protected_struct_t {
 *     int member1;
 *     DECLARE_CRIT_SECTION_LOCK_IN_STRUCT(my_lock)
 *     int another_member;
 * };
 * ...
 * protected_struct_t my_protected = {
 *     .member1 = 0,
 *     INIT_CRIT_SECTION_LOCK_IN_STRUCT(my_lock) // no comma!
 *     another_member = 47,
 * };
 * @endcode
 */
#if OS_SPINLOCK == 1
#ifdef __NuttX__
#define INIT_CRIT_SECTION_LOCK_IN_STRUCT(lock_name) .lock_name = SP_UNLOCKED,
#else
#define INIT_CRIT_SECTION_LOCK_IN_STRUCT(lock_name) .lock_name = portMUX_INITIALIZER_UNLOCKED,
#endif
#else
#define INIT_CRIT_SECTION_LOCK_IN_STRUCT(lock_name)
#endif

/**
 * @brief Enter a critical section, i.e., a section that will not be interrupted by any other task or interrupt.
 *
 * On multi-core systems, this will disable interrupts and take the spinlock \c lock. On single core systems, a
 * spinlock is unnecessary, hence \c lock is ignored and interrupts are disabled only.
 *
 * @note This macro MUST be used together with any of the initialization macros, e.g.
 *       DEFINE_CRIT_SECTION_LOCK_STATIC. If not, there may be unused variables.
 *
 * @param lock Pointer to the critical section lock. Ignored if build for single core system.
 *
 * Example usage with static locks:
 * @code{c}
 * ...
 * #include "os/critical_section.h"
 * ...
 * DEFINE_CRIT_SECTION_LOCK_STATIC(my_lock); // will have internal linking (static)
 * ...
 * esp_os_enter_critical(&my_lock);
 * // code inside critical section
 * esp_os_exit_critical(&my_lock);
 * @endcode
 */
#ifdef __NuttX__
#define esp_os_enter_critical(lock)         ENTER_CRITICAL_SECTION(lock)
#else
#if OS_SPINLOCK == 1
#define esp_os_enter_critical(lock)         portENTER_CRITICAL(lock)
#else
#define esp_os_enter_critical(lock)         vPortEnterCritical()
#endif
#endif

/**
 * @brief Exit a critical section.
 *
 * On multi-core systems, this will enable interrupts and release the spinlock \c lock. On single core systems, a
 * spinlock is unnecessary, hence \c lock is ignored and interrupts are enabled only.
 *
 * @note This macro MUST be used together with any of the initialization macros, e.g.
 *       DEFINE_CRIT_SECTION_LOCK_STATIC. If not, there may be unused variables.
 *
 * @param lock Pointer to the critical section lock. Ignored if build for single core system.
 *
 * Example usage with static locks:
 * @code{c}
 * ...
 * #include "os/critical_section.h"
 * ...
 * DEFINE_CRIT_SECTION_LOCK_STATIC(my_lock); // will have internal linking (static)
 * ...
 * esp_os_enter_critical(&my_lock);
 * // code inside critical section
 * esp_os_exit_critical(&my_lock);
 * @endcode
 */
#ifdef __NuttX__
#define esp_os_exit_critical(lock)          EXIT_CRITICAL_SECTION(lock)
#else
#if OS_SPINLOCK == 1
#define esp_os_exit_critical(lock)          portEXIT_CRITICAL(lock)
#else
#define esp_os_exit_critical(lock)          vPortExitCritical()
#endif
#endif

/**
 * @brief Enter a critical section while from ISR.
 *
 * On multi-core systems, this will disable interrupts and take the spinlock \c lock. On single core systems, a
 * spinlock is unnecessary, hence \c lock is ignored and interrupts are disabled only.
 *
 * @note This macro MUST be used together with any of the initialization macros, e.g.
 *       DEFINE_CRIT_SECTION_LOCK_STATIC. If not, there may be unused variables.
 *
 * @param lock Pointer to the critical section lock. Ignored if build for single core system.
 *
 * Example usage with static locks:
 * @code{c}
 * ...
 * #include "os/critical_section.h"
 * ...
 * DEFINE_CRIT_SECTION_LOCK_STATIC(my_lock); // will have internal linking (static)
 * ...
 * esp_os_enter_critical(&my_lock);
 * // code inside critical section
 * esp_os_exit_critical(&my_lock);
 * @endcode
 */
#ifdef __NuttX__
#define esp_os_enter_critical_isr(lock)     ENTER_CRITICAL_SECTION(lock)
#else
#if OS_SPINLOCK == 1
#define esp_os_enter_critical_isr(lock)     portENTER_CRITICAL_ISR(lock)
#else
#define esp_os_enter_critical_isr(lock)     vPortEnterCritical()
#endif
#endif
/**
 * @brief Exit a critical section after entering from ISR.
 *
 * On multi-core systems, this will enable interrupts and release the spinlock \c lock. On single core systems, a
 * spinlock is unnecessary, hence \c lock is ignored and interrupts are enabled only.
 *
 * @note This macro MUST be used together with any of the initialization macros, e.g.
 *       DEFINE_CRIT_SECTION_LOCK_STATIC. If not, there may be unused variables.
 *
 * @param lock Pointer to the critical section lock. Ignored if build for single core system.
 *
 * Example usage with static locks:
 * @code{c}
 * ...
 * #include "os/critical_section.h"
 * ...
 * DEFINE_CRIT_SECTION_LOCK_STATIC(my_lock); // will have internal linking (static)
 * ...
 * esp_os_enter_critical(&my_lock);
 * // code inside critical section
 * esp_os_exit_critical(&my_lock);
 * @endcode
 */
#ifdef __NuttX__
#define esp_os_exit_critical_isr(lock)      EXIT_CRITICAL_SECTION(lock)
#else
#if OS_SPINLOCK == 1
#define esp_os_exit_critical_isr(lock)      portEXIT_CRITICAL_ISR(lock)
#else
#define esp_os_exit_critical_isr(lock)      vPortExitCritical()
#endif
#endif

/**
 * @brief Enter a critical section from normal task or ISR. This macro will check if the current CPU is processing
 *        an ISR or not and enter the critical section accordingly.
 *
 * On multi-core systems, this will disable interrupts and take the spinlock \c lock. On single core systems, a
 * spinlock is unnecessary, hence \c lock is ignored and interrupts are disabled only.
 *
 * @note This macro MUST be used together with any of the initialization macros, e.g.
 *       DEFINE_CRIT_SECTION_LOCK_STATIC. If not, there may be unused variables.
 *
 * @param lock Pointer to the critical section lock. Ignored if build for single core system.
 *
 * Example usage with static locks:
 * @code{c}
 * ...
 * #include "os/critical_section.h"
 * ...
 * DEFINE_CRIT_SECTION_LOCK_STATIC(my_lock); // will have internal linking (static)
 * ...
 * esp_os_enter_critical(&my_lock);
 * // code inside critical section
 * esp_os_exit_critical(&my_lock);
 * @endcode
 */
#ifdef __NuttX__
#define esp_os_enter_critical_safe(lock)    ENTER_CRITICAL_SECTION(lock)
#else
#if OS_SPINLOCK == 1
#define esp_os_enter_critical_safe(lock)    portENTER_CRITICAL_SAFE(lock)
#else
#define esp_os_enter_critical_safe(lock)    vPortEnterCritical()
#endif
#endif

/**
 * @brief Exit a critical section after entering via esp_os_enter_critical_safe.
 *
 * On multi-core systems, this will enable interrupts and release the spinlock \c lock. On single core systems, a
 * spinlock is unnecessary, hence \c lock is ignored and interrupts are enabled only.
 *
 * @note This macro MUST be used together with any of the initialization macros, e.g.
 *       DEFINE_CRIT_SECTION_LOCK_STATIC. If not, there may be unused variables.
 *
 * @param lock Pointer to the critical section lock. Ignored if build for single core system.
 *
 * Example usage with static locks:
 * @code{c}
 * ...
 * #include "os/critical_section.h"
 * ...
 * DEFINE_CRIT_SECTION_LOCK_STATIC(my_lock); // will have internal linking (static)
 * ...
 * esp_os_enter_critical(&my_lock);
 * // code inside critical section
 * esp_os_exit_critical(&my_lock);
 * @endcode
 */
#ifdef __NuttX__
#define esp_os_exit_critical_safe(lock)    EXIT_CRITICAL_SECTION(lock)
#else
#if OS_SPINLOCK == 1
#define esp_os_exit_critical_safe(lock)     portEXIT_CRITICAL_SAFE(lock)
#else
#define esp_os_exit_critical_safe(lock)     vPortExitCritical()
#endif
#endif

#ifdef __cplusplus
}
#endif
