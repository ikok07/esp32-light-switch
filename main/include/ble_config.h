//
// Created by Kok on 3/10/26.
//

#ifndef ESP32S3_BLE_BLE_CONFIG_H
#define ESP32S3_BLE_BLE_CONFIG_H

#include "ble.h"

typedef struct {
    uint16_t LightStateChrHandle;
} BLE_BspChrsTypeDef;

extern BLE_BspChrsTypeDef gBleBspChrs;

void BT_Configure(BLE_HandleTypeDef *hble);

#endif //ESP32S3_BLE_BLE_CONFIG_H