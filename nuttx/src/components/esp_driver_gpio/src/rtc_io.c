/*
 * SPDX-FileCopyrightText: 2019-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_private/periph_ctrl.h"
#include "esp_private/io_mux.h"
#include "hal/rtc_io_hal.h"
#include "soc/rtc_io_periph.h"
#include "soc/soc_caps.h"
#include "driver/gpio.h"

bool rtc_gpio_is_valid_gpio(gpio_num_t gpio_num)
{
#if SOC_RTCIO_PIN_COUNT > 0
    return (gpio_num < GPIO_PIN_COUNT && rtc_io_num_map[gpio_num] >= 0);
#else
    return false;
#endif
}

#if SOC_RTCIO_PIN_COUNT > 0
/*---------------------------------------------------------------
                        RTC IO
---------------------------------------------------------------*/
int rtc_io_number_get(gpio_num_t gpio_num)
{
    return rtc_io_num_map[gpio_num];
}

#endif // SOC_RTCIO_PIN_COUNT > 0
