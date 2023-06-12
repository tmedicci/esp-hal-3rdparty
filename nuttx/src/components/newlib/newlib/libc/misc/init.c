/*
 * SPDX-FileCopyrightText: 2024-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stddef.h>

extern void (*__init_priority_array_start []) (void) __attribute__((weak));
extern void (*__init_priority_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));

/* Iterate over all the init routines.  */
void
__libc_init_array (void)
{
  size_t count;
  size_t i;

#if __riscv
  count = __init_priority_array_end - __init_priority_array_start;
  for (i = 0; i < count; i++)
    __init_priority_array_start[i] ();
#endif

  count = __init_array_end - __init_array_start;
  for (i = 0; i < count; i++)
    __init_array_start[i] ();
}
