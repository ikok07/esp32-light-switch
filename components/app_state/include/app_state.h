//
// Created by Kok on 2/12/26.
//

#ifndef ESP32S3_APP_STATE_H
#define ESP32S3_APP_STATE_H

#include <stdint.h>

#include "timer.h"
#include "task_scheduler.h"
#include "ble.h"

typedef struct {
    SCHEDULER_TaskTypeDef LedTask;
    SCHEDULER_TaskTypeDef LedNotifyTask;
    SCHEDULER_TaskTypeDef BleTask;
} APP_TasksTypeDef;

typedef struct {
    TIMER_HandleTypeDef *htim;
    BLE_HandleTypeDef *hble;
    APP_TasksTypeDef *Tasks;
} APP_StateTypeDef;

extern APP_StateTypeDef gAppState;

void APP_Init();

#endif //ESP32S3_APP_STATE_H