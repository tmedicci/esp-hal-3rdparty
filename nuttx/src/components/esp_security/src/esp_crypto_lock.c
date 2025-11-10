/*
 * SPDX-FileCopyrightText: 2022-2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <nuttx/mutex.h>
 
#include "esp_crypto_lock.h"

#if defined(SOC_SHA_SUPPORTED) || defined(SOC_AES_SUPPORTED)
/* Single lock for SHA and AES, sharing a reserved GDMA channel */
static mutex_t s_crypto_sha_aes_lock;
#endif /* defined(SOC_SHA_SUPPORTED) || defined(SOC_AES_SUPPORTED) */

#if defined(SOC_SHA_CRYPTO_DMA) || defined(SOC_AES_CRYPTO_DMA)
void esp_crypto_dma_lock_acquire(void)
{
    nxmutex_lock(&s_crypto_sha_aes_lock);
}

void esp_crypto_dma_lock_release(void)
{
    nxmutex_unlock(&s_crypto_sha_aes_lock);
}
#endif /* defined(SOC_SHA_CRYPTO_DMA) || defined(SOC_AES_CRYPTO_DMA) */
