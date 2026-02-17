//
// Created by Kok on 1/29/26.
//

#ifndef LOG_H
#define LOG_H

#include <stdint.h>

#define __weak __attribute__((weak))

#define LOGGER_MSG_MAX_LEN          255

typedef enum {
    LOGGER_LEVEL_DEBUG,
    LOGGER_LEVEL_INFO,
    LOGGER_LEVEL_WARNING,
    LOGGER_LEVEL_ERROR,
    LOGGER_LEVEL_FATAL,
} LOGGER_LevelTypeDef;

typedef enum {
    LOGGER_ERROR_OK,
    LOGGER_ERROR_IMPLEMENTATION,    // An error occurred in the system-specific implementation
    LOGGER_ERROR_UNINITIALIZED,
    LOGGER_ERROR_MESSAGE_LEN,        // The passed message is longer than the allowed maximum (LOGGER_MSG_MAX_LEN)
    LOGGER_ERROR_DISABLED
} LOGGER_ErrorTypeDef;

typedef struct {
    uint8_t Initialized;            // Used to track if the API has been initialized
    uint8_t Enabled;
    uint8_t FatalOccurred;
    LOGGER_LevelTypeDef ActiveLevel;
} LOGGER_TypeDef;

typedef struct {
    LOGGER_LevelTypeDef Level;
    char *msg;
} LOGGER_EventTypeDef;

/* ------ Main methods ------ */

void LOGGER_InitBasic();
void LOGGER_LogBasic();

LOGGER_ErrorTypeDef LOGGER_Init();
LOGGER_ErrorTypeDef LOGGER_DeInit();
LOGGER_ErrorTypeDef LOGGER_ReInit();
LOGGER_ErrorTypeDef LOGGER_Log(LOGGER_LevelTypeDef Level, char *Msg);
LOGGER_ErrorTypeDef LOGGER_LogF(LOGGER_LevelTypeDef Level, char *Fmt, ...);

/* ------ Controls ------ */

LOGGER_ErrorTypeDef LOGGER_Enable();
LOGGER_ErrorTypeDef LOGGER_Disable();
LOGGER_ErrorTypeDef LOGGER_SetLevel(LOGGER_LevelTypeDef Level);

/* ------ Callbacks ------ */

void LOGGER_InitBasicCB();
void LOGGER_LogBasicCB();

uint8_t LOGGER_InitCB();
uint8_t LOGGER_DeInitCB();
uint8_t LOGGER_LogCB(LOGGER_EventTypeDef *Event);
uint8_t LOGGER_FatalCB(LOGGER_EventTypeDef *Event);
uint8_t LOGGER_FormatCB(LOGGER_EventTypeDef *Event, char *Buffer, uint16_t Len);

/* ------ Utils ------ */

char *LOGGER_GetLevelLabel(LOGGER_LevelTypeDef Level);

#endif //LOG_H
