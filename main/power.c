//
// Created by Kok on 2/11/26.
//

#include "power.h"

#include "esp_system.h"
#include "freertos/FreeRTOS.h"

static esp_pm_lock_handle_t gMaxFreqLock;
static esp_pm_lock_handle_t gNoSleeplock;

/**
 * @brief Configures the MCU's power management
 */
esp_err_t POWER_Config() {
    esp_pm_config_t POWER_Config = {
        .max_freq_mhz = CPU_MAX_MHZ,
        .min_freq_mhz = CPU_MIN_MHZ,
        .light_sleep_enable = pdTRUE                // Tickless Idle must be enabled in menuconfig
    };
    return esp_pm_configure(&POWER_Config);
}

/**
 * @brief Enables or disables max frequency usage
 */
esp_err_t POWER_FreqControl(uint8_t MaxFreqEnabled) {
    esp_err_t err;
    if (MaxFreqEnabled) {
        if ((err = esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "max-power", &gMaxFreqLock)) != ESP_OK) {
            return err;
        }
        return esp_pm_lock_acquire(gMaxFreqLock);
    }

    if (gMaxFreqLock != NULL) {
        return esp_pm_lock_release(gMaxFreqLock);
    }

    return ESP_OK;
}

/**
 * @brief Controls wether the MCU can enter light sleep
 */
esp_err_t POWER_LightSleepControl(uint8_t Enabled) {
    esp_err_t err;
    if (Enabled) {
        if ((err = esp_pm_lock_create(ESP_PM_NO_LIGHT_SLEEP, 0, "no-sleep", &gNoSleeplock)) != ESP_OK) {
            return err;
        }
        return esp_pm_lock_acquire(gNoSleeplock);
    }

    if (gNoSleeplock != NULL) {
        return esp_pm_lock_release(gNoSleeplock);
    }

    return ESP_OK;
}

/**
 * @brief Waits some time and restarts the MCU
 */
void POWER_WaitAndRestart(uint32_t DelayMs) {
    vTaskDelay(pdMS_TO_TICKS(DelayMs));
    esp_restart();
}
