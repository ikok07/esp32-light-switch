//
// Created by Kok on 2/13/26.
//

#include "ble.h"
#include "gap.h"
#include "gatt.h"

#include "nvs_flash.h"
#include "task_scheduler.h"

#include "host/ble_hs.h"

#include "nimble/nimble_port.h"
#include "nimble/ble.h"

/* ------ Global variables ------ */
BLE_HandleTypeDef *gHble = NULL;

/* ------ Callbacks ------ */

static void on_stack_sync_cb(void);

/* ------ Tasks ------ */

static void ble_task(void *arg);

/* ------ Private methods ------ */

static void format_addr(char *AddrStr, uint8_t Len, uint8_t Address[]);

/**
 * @brief Initializes the BLE driver by enabling NVS, GAP and GATT
 * @param hble BLE Handle
 */
BLE_ErrorTypeDef BLE_Init(BLE_HandleTypeDef *hble) {
    gHble = hble;

    BLE_ErrorTypeDef ble_err = BLE_ERROR_OK;
    esp_err_t err = ESP_OK;

    uint8_t nvs_ready = 0;
    while (!nvs_ready) {
        // Initialize the flash memory
        if ((err = nvs_flash_init()) != ESP_OK) {
            if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
                nvs_flash_erase();
            }
            return BLE_ERROR_NVS;
        }
        nvs_ready = 1;
    }

    // Initialize the NimBLE stack
    if ((err = nimble_port_init()) != ESP_OK) {
        return BLE_ERROR_INIT;
    }

    ble_hs_cfg.reset_cb = BLE_StackResetCB;
    ble_hs_cfg.sync_cb = on_stack_sync_cb;
    ble_hs_cfg.gatts_register_cb = on_gatt_event;

    // Initialize GAP
    if ((ble_err = gap_init(gHble)) != BLE_ERROR_OK) return ble_err;

    // Initialize GATT
    if ((ble_err = gatt_init()) != BLE_ERROR_OK) return ble_err;

    // Create BLE task
    hble->BLE_Task->Function = ble_task;
    SCHEDULER_Create(hble->BLE_Task);

    return BLE_ERROR_OK;
}

/**
 * @brief This callback will be executed whenever the BLE stack gets reset by an error
 */
__weak void BLE_StackResetCB(int Reason) {}

/**
 * @brief This callback will be executed when a GAP event occurs
 */
__weak void BLE_GapEventCB(BLE_GapEventTypeDef Event, struct ble_gap_event *GapEvent, void *Arg) {};

/**
 * @brief This callback will be executed when an error occurs while the driver is running
 * @param Error BLE Error
 */
__weak void BLE_ErrorCB(BLE_ErrorTypeDef Error) {}

/**
 * @brief This callback will be executed when configuring GAP advertisement.
 *        It is used to set the required service UUIDs in the advertised fields.
 * @param Fields Advertisement fields
 */
__weak void BLE_AdvertiseSvcsCB(struct ble_hs_adv_fields *Fields) {}

void on_stack_sync_cb(void) {
    BLE_ErrorTypeDef err;
    if ((err = gap_start_adv(gHble)) != BLE_ERROR_OK) {
        BLE_ErrorCB(err);
    }
}

void ble_task(void *arg) {
    nimble_port_run();
    vTaskDelete(NULL);
}
