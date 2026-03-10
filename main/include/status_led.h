//
// Created by Kok on 3/10/26.
//

#ifndef ESP32S3_BLE_STATUS_LIGHT_H
#define ESP32S3_BLE_STATUS_LIGHT_H

#include "led_strip.h"
#include "soc/gpio_num.h"

#define STATUS_LED_GPIO                             (GPIO_NUM_48)

typedef enum {
    STATUSLED_STATE_CONFIGURING,
    STATUSLED_STATE_ERROR_HW,
    STATUSLED_STATE_ERROR_BT,
    STATUSLED_STATE_OPERATIONAL
} STATUSLED_StateTypeDef;

void STATUSLED_Init();
void STATUSLED_SetState(STATUSLED_StateTypeDef State);

#endif //ESP32S3_BLE_STATUS_LIGHT_H