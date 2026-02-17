//
// Created by Kok on 2/12/26.
//

#ifndef ESP32S3_LED_H
#define ESP32S3_LED_H

#define LED_PIN                 (GPIO_NUM_48)

typedef enum {
    LED_ACTIVE_RED,
    LED_ACTIVE_GREEN,
    LED_ACTIVE_BLUE,
} LED_ActiveLightTypeDef;

void LED_Init();
char *LED_ActiveLightLabel(LED_ActiveLightTypeDef ActiveLight);

#endif //ESP32S3_LED_H