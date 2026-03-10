//
// Created by Kok on 3/9/26.
//

#include "include/light_control.h"

#include "app_state.h"
#include "task_scheduler.h"
#include "tasks_common.h"
#include "driver/gpio.h"
#include "log.h"
#include "power.h"
#include "status_led.h"

#include "soc/gpio_num.h"


static void light_ctrl_task(void *arg);
static void IRAM_ATTR light_toggle_isr(void *arg);

void LCTRL_Init() {
    esp_err_t esp_err;
    gpio_config_t io_cfg = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1 << LIGHT_CTRL_GPIO),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    if ((esp_err = gpio_config(&io_cfg)) != ESP_OK) {
        LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to initialize light control GPIO! Error code: %d", esp_err);
        STATUSLED_SetState(STATUSLED_STATE_ERROR_HW);
        POWER_WaitAndRestart(3000);
    } else {
        LOGGER_Log(LOGGER_LEVEL_INFO, "Light control GPIO initialized successfully!");
    };

    io_cfg.mode = GPIO_MODE_INPUT;
    io_cfg.pin_bit_mask = (1 << LIGHT_TOGGLE_GPIO);
    io_cfg.intr_type = GPIO_INTR_POSEDGE;
    io_cfg.pull_down_en = GPIO_PULLDOWN_ENABLE;
    if ((esp_err = gpio_config(&io_cfg)) != ESP_OK) {
        LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to initialize light toggle GPIO! Error code: %d", esp_err);
        STATUSLED_SetState(STATUSLED_STATE_ERROR_HW);
        POWER_WaitAndRestart(3000);
    } else {
        LOGGER_Log(LOGGER_LEVEL_INFO, "Light toggle GPIO initialized successfully!");
    };

    if ((esp_err = gpio_install_isr_service(0)) != ESP_OK) {
        LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to install GPIO ISR service! Error code: %d", esp_err);
        STATUSLED_SetState(STATUSLED_STATE_ERROR_HW);
        POWER_WaitAndRestart(3000);
    };

    if ((esp_err = gpio_isr_handler_add(LIGHT_TOGGLE_GPIO, light_toggle_isr, NULL)) != ESP_OK) {
        LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to add light toggle ISR handler! Error code: %d", esp_err);
        STATUSLED_SetState(STATUSLED_STATE_ERROR_HW);
        POWER_WaitAndRestart(3000);
    };

    gAppState.Tasks->LightCtrlTask = (SCHEDULER_TaskTypeDef){
        .Active = 0,
        .CoreID = LIGHT_CONTROL_TASK_CORE_ID,
        .Name = "Light control task",
        .Priority = LIGHT_CONTROL_TASK_PRIORITY,
        .StackDepth = LIGHT_CONTROL_TASK_STACK_DEPTH,
        .Function = light_ctrl_task
    };

    SHVAL_ConfigTypeDef shval_config = {
        .InitialValue = 0,
        .SubscribersQueueSize = 1
    };
    gAppState.SharedValues->LightState = SHVAL_Init(&shval_config);

    SCHEDULER_Create(&gAppState.Tasks->LightCtrlTask);
}

void light_ctrl_task(void *arg) {
    while (1) {
        uint32_t enable;
        if (xTaskNotifyWait(0, 0xFF, &enable, portMAX_DELAY)) {
            uint8_t level;
            // Enable light
            if (enable == 1) level = 1;
            // Toggle light
            else if (enable == 2) level = !gpio_get_level(LIGHT_CTRL_GPIO);
            // Disable light
            else level = 0;

            gpio_set_level(LIGHT_CTRL_GPIO, level);

            if (SHVAL_SetValue(&gAppState.SharedValues->LightState, level, 1000) != SHVAL_ERROR_OK) {
                LOGGER_Log(LOGGER_LEVEL_ERROR, "Failed to set shared light state variable!");
            }

            LOGGER_Log(LOGGER_LEVEL_INFO, level == 1 ? "Light enabled" : "Light disabled");
        }
    }
}

void light_toggle_isr(void *arg) {
    BaseType_t higher_priority_woken = pdFALSE;
    xTaskNotifyFromISR(gAppState.Tasks->LightCtrlTask.OsTask, 2, eSetValueWithOverwrite, &higher_priority_woken);
    portYIELD_FROM_ISR(higher_priority_woken);
}