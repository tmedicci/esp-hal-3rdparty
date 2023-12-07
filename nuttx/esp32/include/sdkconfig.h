/*
 * Copyright (c) 2023 Espressif Systems (Shanghai) Co., Ltd.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <nuttx/config.h>

#define CONFIG_ESP_REV_MIN_FULL 0
#define CONFIG_ESP_REV_MAX_FULL 399
#define CONFIG_IDF_TARGET_ARCH_XTENSA 1
#define CONFIG_IDF_TARGET_ESP32 1
#define CONFIG_IDF_FIRMWARE_CHIP_ID 0x0000
#define CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ
#define CONFIG_RTC_CLK_SRC_INT_RC 1
#define CONFIG_RTC_CLK_CAL_CYCLES 1024
#define CONFIG_XTAL_FREQ 40
#define CONFIG_MMU_PAGE_SIZE 0x10000
#define CONFIG_LOG_TIMESTAMP_SOURCE_RTOS 1
#define CONFIG_LOG_DEFAULT_LEVEL 3
#define CONFIG_LOG_MAXIMUM_LEVEL 3
#define CONFIG_ESPTOOLPY_FLASHMODE_DIO 1

#if (defined(CONFIG_UART0_SERIAL_CONSOLE) && defined(CONFIG_ESP32_UART0))
#define CONFIG_ESP_CONSOLE_UART_NUM 0
#elif defined(CONFIG_UART1_SERIAL_CONSOLE) && defined(CONFIG_ESP32_UART1)
#define CONFIG_ESP_CONSOLE_UART_NUM 1
#endif