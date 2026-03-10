//
// Created by Kok on 2/12/26.
//

#ifndef ESP32S3_APP_STATE_H
#define ESP32S3_APP_STATE_H

#include "timer.h"
#include "task_scheduler.h"
#include "ble.h"
#include "shared_values.h"
#include "led_strip.h"

typedef struct {
    SCHEDULER_TaskTypeDef BleTask;
    SCHEDULER_TaskTypeDef LightCtrlTask;
    SCHEDULER_TaskTypeDef StatusLedTask;
} APP_TasksTypeDef;

typedef struct {
    SHVAL_HandleTypeDef LightState;
} APP_SharedValuesTypeDef;

typedef struct {
    led_strip_handle_t *hstatusled;
    BLE_HandleTypeDef *hble;
    APP_TasksTypeDef *Tasks;
    APP_SharedValuesTypeDef *SharedValues;
} APP_StateTypeDef;

extern APP_StateTypeDef gAppState;

void APP_Init();

#endif //ESP32S3_APP_STATE_H