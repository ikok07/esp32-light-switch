#include "app_state.h"
#include "power.h"
#include "log.h"
#include "ble.h"
#include "led_strip.h"

#include "bt.h"
#include "log-config.h"
#include "light_control.h"
#include "status_led.h"

void app_main(void) {
    esp_err_t esp_err;

    // Initialize app state
    APP_Init();

    // Configure logger
    LOG_Configure();

    // Configure power
    if ((esp_err = POWER_Config())!= ESP_OK) {
        LOGGER_LogF(LOGGER_LEVEL_FATAL, "Failed to configure board power! Error code: %d", esp_err);
        return;
    }
    POWER_FreqControl(pdTRUE);

    // Disable MCU sleep during configuration
    POWER_LightSleepControl(pdFALSE);

    // Configure Status LED
    STATUSLED_Init();

    // Set configuring LED state
    STATUSLED_SetState(STATUSLED_STATE_CONFIGURING);

    // Configure light control
    LCTRL_Init();

    // Configure and start BLE task
    BT_Init();

    // Wait for all tasks to start
    while (
        !gAppState.Tasks->StatusLedTask.Active ||
        !gAppState.Tasks->LightCtrlTask.Active ||
        !gAppState.Tasks->BleTask.Active
    ) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    // Allow MCU to sleep
    POWER_LightSleepControl(pdTRUE);
}