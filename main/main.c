#include "app_state.h"
#include "ble.h"
#include "bt.h"
#include "led_strip.h"

#include "power.h"
#include "led.h"
#include "log.h"

// TODO: Different tasks should subscribe to led color change event
// TODO: Handle GAP unsubscribe event
// TODO: Implement BLE bonding

void app_main(void) {
    // Initialize app state
    APP_Init();

    // Configure logger
    LOGGER_Init();
    LOGGER_Enable();
    LOGGER_SetLevel(LOGGER_LEVEL_DEBUG);

    // Configure power
    if (POWER_Config() != ESP_OK) {
        LOGGER_Log(LOGGER_LEVEL_FATAL, "Failed to configure board power!");
        return;
    }

    // Configure and start LED task
    LED_Init();

    // Configure and start BLE task
    BT_Init();
}