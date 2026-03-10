//
// Created by Kok on 2/11/26.
//

#ifndef ESP32S3_POWER_H
#define ESP32S3_POWER_H

#include "esp_pm.h"

#define CPU_MAX_MHZ             80
#define CPU_MIN_MHZ             10

esp_err_t POWER_Config();

esp_err_t POWER_RunMax();
esp_err_t POWER_DisableLightSleep();
esp_err_t POWER_RunNormal();

void POWER_WaitAndRestart(uint32_t DelayMs);

#endif //ESP32S3_POWER_H