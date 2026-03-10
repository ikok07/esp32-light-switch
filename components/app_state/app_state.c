//
// Created by Kok on 2/12/26.
//

#include "app_state.h"

led_strip_handle_t hstatusled;
BLE_HandleTypeDef hble;
APP_TasksTypeDef tasks;
APP_SharedValuesTypeDef shared_values;

APP_StateTypeDef gAppState;

void APP_Init() {
    gAppState = (APP_StateTypeDef){
        .hstatusled = &hstatusled,
        .hble = &hble,
        .Tasks = &tasks,
        .SharedValues = &shared_values
    };
}
