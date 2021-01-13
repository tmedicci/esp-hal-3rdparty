// Copyright 2020 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <esp_bit_defs.h>
#include "esp_efuse.h"
#include "esp_efuse_table.h"

int esp_efuse_rtc_calib_get_ver(void)
{
    uint32_t result = 0;
    esp_efuse_read_field_blob(ESP_EFUSE_BLOCK2_VERSION, &result, 3);
    return result;
}

uint16_t esp_efuse_rtc_calib_get_init_code(int version, int atten)
{
    assert(version == 1);
    const esp_efuse_desc_t** init_code_efuse;
    assert(atten < 4);
    if (atten == 0) {
        init_code_efuse = ESP_EFUSE_ADC1_INIT_CODE_ATTEN0;
    } else if (atten == 1) {
        init_code_efuse = ESP_EFUSE_ADC1_INIT_CODE_ATTEN1;
    } else if (atten == 2) {
        init_code_efuse = ESP_EFUSE_ADC1_INIT_CODE_ATTEN2;
    } else {
        init_code_efuse = ESP_EFUSE_ADC1_INIT_CODE_ATTEN3;
    }

    int init_code_size = esp_efuse_get_field_size(init_code_efuse);
    assert(init_code_size == 10);

    uint32_t init_code = 0;
    esp_err_t err = esp_efuse_read_field_blob(init_code_efuse, &init_code, init_code_size);
    assert(err == ESP_OK);
    return init_code + 1000;    // version 1 logic
}

esp_err_t esp_efuse_rtc_calib_get_cal_voltage(int version, int atten, uint32_t* out_digi, uint32_t* out_vol_mv)
{
    const esp_efuse_desc_t** cal_vol_efuse;
    uint32_t calib_vol_expected_mv;
    if (version != 1) {
        return ESP_ERR_INVALID_ARG;
    }
    if (atten >= 4) {
        return ESP_ERR_INVALID_ARG;
    }
    if (atten == 0) {
        cal_vol_efuse = ESP_EFUSE_ADC1_CAL_VOL_ATTEN0;
        calib_vol_expected_mv = 400;
    } else if (atten == 1) {
        cal_vol_efuse = ESP_EFUSE_ADC1_CAL_VOL_ATTEN1;
        calib_vol_expected_mv = 550;
    } else if (atten == 2) {
        cal_vol_efuse = ESP_EFUSE_ADC1_CAL_VOL_ATTEN2;
        calib_vol_expected_mv = 750;
    } else {
        cal_vol_efuse = ESP_EFUSE_ADC1_CAL_VOL_ATTEN3;
        calib_vol_expected_mv = 1370;
    }

    int cal_vol_size = esp_efuse_get_field_size(cal_vol_efuse);
    assert(cal_vol_size == 10);

    uint32_t cal_vol = 0;
    esp_err_t err = esp_efuse_read_field_blob(cal_vol_efuse, &cal_vol, cal_vol_size) & 0x3FF;
    assert(err == ESP_OK);

    *out_digi = 2000 + ((cal_vol & BIT(9))? -(cal_vol & ~BIT9): cal_vol);
    *out_vol_mv = calib_vol_expected_mv;
    return ESP_OK;
}

float esp_efuse_rtc_calib_get_cal_temp(int version)
{
    assert(version == 1);
    const esp_efuse_desc_t** cal_temp_efuse;
    cal_temp_efuse = ESP_EFUSE_TEMP_CALIB;
    int cal_temp_size = esp_efuse_get_field_size(cal_temp_efuse);
    assert(cal_temp_size == 9);

    uint32_t cal_temp = 0;
    esp_err_t err = esp_efuse_read_field_blob(cal_temp_efuse, &cal_temp, cal_temp_size);
    assert(err == ESP_OK);
    // BIT(8) stands for sign: 1: negtive, 0: positive
    return ((cal_temp & BIT(8)) != 0)? -(uint8_t)cal_temp: (uint8_t)cal_temp;
}
