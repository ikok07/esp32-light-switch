//
// Created by Kok on 2/14/26.
//

#include "bt.h"

#include <ble_config.h>
#include <sys/types.h>

#include "app_state.h"
#include "tasks_common.h"
#include "log.h"
#include "power.h"
#include "include/status_led.h"

void bt_config_task(void *arg);
void light_state_notify_task(void *arg);

static uint8_t gManufacturerData[4] = {
    0x62, 0x48,                                  // Manufacturer ID (Defined in Home Assistant integration)
    BT_DEVICE_TYPE,                              // Device type - generic switch (Defined in Home Assistant integration),
    0x00                                         // Device does not require pairing
};

static SCHEDULER_TaskTypeDef gConfigTask = {
    .Active = 0,
    .CoreID = BT_CFG_TASK_CORE_ID,
    .Name = "BT Config Task",
    .Priority = BT_CFG_TASK_PRIORITY,
    .StackDepth = BT_CFG_TASK_STACK_DEPTH,
    .Args = NULL,
    .Function = bt_config_task
};

void BT_Init() {
    SCHEDULER_Create(&gConfigTask);
}

void bt_config_task(void *arg) {
    // Wait for light control to initialize
    while (!gAppState.Tasks->LightCtrlTask.Active) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    BLE_ErrorTypeDef ble_err = BLE_ERROR_OK;
    gAppState.Tasks->BleTask = (SCHEDULER_TaskTypeDef){
        .Active = 0,
        .CoreID = BLE_TASK_CORE_ID,
        .Name = "NimBLE Task",
        .Priority = BLE_TASK_PRIORITY,
        .StackDepth = BLE_TASK_STACK_DEPTH,
        .Args = NULL,
    };

    gAppState.Tasks->BleLightStateNotificationsTask = (SCHEDULER_TaskTypeDef){
        .Active = 0,
        .CoreID = BLE_LIGHT_STATE_N_TASK_CORE_ID,
        .Name = "BLE Light state notification",
        .Priority = BLE_LIGHT_STATE_N_TASK_PRIORITY,
        .StackDepth = BLE_LIGHT_STATE_N_TASK_STACK_DEPTH,
        .Args = NULL,
        .Function = light_state_notify_task
    };

    *gAppState.hble = (BLE_HandleTypeDef){
        .BLE_Task = &gAppState.Tasks->BleTask,
        .Config = {
            .DeviceName = "LSwitch",
            .GapAppearance = 0x04C1,            // Switch appearance
            .AdvertisingIntervalMS = 50,
            .GapRole = BLE_GAP_ROLE_PERIPHERAL,
            .PrivateAddressEnabled = 0,
            .NonResolvablePrivateAddress = 0,
            .MaxConnections = 1,
            .DiscoverabilityMode = BLE_DISC_MODE_ALLOW_ALL,
            .ConnectionMode = BLE_CONN_MODE_ALLOW_ALL,
            .Security = {
                .EncryptedConnection = 0,
                .IOCapability = BLE_IOCAP_NO_INP_OUT,
                .ProtectionType = BLE_PROTECTION_JUST_WORKS
            },
            .ManufacturerData = {
                .ManufacturerName = BT_MANUFACTURER_NAME,
                .SerialNumber = BT_SERIAL
            },
            .AdvMfgData = gManufacturerData,
            .AdvMfgDataLen = sizeof(gManufacturerData) / sizeof(gManufacturerData[0]),
        }
    };

    // Configure platform-specific options
    BT_Configure(gAppState.hble);

    if ((ble_err = BLE_Init(gAppState.hble)) != BLE_ERROR_OK) {
        LOGGER_LogF(LOGGER_LEVEL_FATAL, "Failed to initialize BLE! Error code: %d", ble_err);
        STATUSLED_SetState(STATUSLED_STATE_ERROR_BT_CFG);
        POWER_WaitAndRestart(3000);
    } else {
        LOGGER_Log(LOGGER_LEVEL_INFO, "BLE initialized!");
        STATUSLED_SetState(STATUSLED_STATE_READY_TO_CONNECT);
    };

    // Start light state notifications task
    SCHEDULER_Create(&gAppState.Tasks->BleLightStateNotificationsTask);

    // Remove the config task
    SCHEDULER_Remove(&gConfigTask);
}

void light_state_notify_task(void *arg) {
    while (1) {
        if (xTaskNotifyWait(0, 0xFF, NULL, portMAX_DELAY)) {

            // Clear pending notifications
            xTaskNotifyStateClear(NULL);

            uint32_t light_state;
            SHVAL_ErrorTypeDef shval_err;
            if ((shval_err = SHVAL_GetValue(&gAppState.SharedValues->LightState, &light_state, 1000)) != SHVAL_ERROR_OK) {
                LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to get shared light state value! Error code: %d", shval_err);
                continue;
            }

            BLE_ErrorTypeDef ble_err = BLE_ERROR_OK;
            uint8_t conn_count = sizeof(gAppState.hble->Connections) / sizeof(gAppState.hble->Connections[0]);
            if ((ble_err = BLE_SendNotification(gAppState.hble->Connections, conn_count, gBleAttributes.LightStateChrHandle, &light_state, 1, gAppState.hble->Config.Security.EncryptedConnection)) != BLE_ERROR_OK) {
                LOGGER_LogF(LOGGER_LEVEL_INFO, "Failed to send BLE notification! Error code: %d", ble_err);
                continue;
            }
        }
    }
}