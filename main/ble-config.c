//
// Created by Kok on 3/10/26.
//

#include <string.h>

#include "ble_config.h"
#include "ble.h"
#include "log.h"
#include "app_state.h"

#define BLE_DEVICE_PASSWORD                                          123456

static const ble_uuid16_t automation_service_uuid = BLE_UUID16_INIT(0x1815);

static const ble_uuid16_t digital_chr_uuid = BLE_UUID16_INIT(0x2A56);

static const ble_uuid16_t description_dsc_uuid = BLE_UUID16_INIT(0x2901);
static const ble_uuid16_t presentation_dsc_uuid = BLE_UUID16_INIT(0x2904);
static const ble_uuid16_t num_digitals_dsc_uuid = BLE_UUID16_INIT(0x2909);

#define BLE_DSC_OBJ_DESCRIPTION(AccessCB, Description)             {\
                                                                        .uuid = &description_dsc_uuid.u,\
                                                                        .att_flags = BLE_ATT_F_READ | BLE_ATT_F_READ_ENC,\
                                                                        .access_cb = AccessCB,\
                                                                        .arg = Description\
                                                                    }\


BLE_BspChrsTypeDef gBleBspChrs;

int light_state_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);

int light_state_char_presentation_dsc_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);

int num_digitals_dsc_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);

int description_dsc_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                               struct ble_gatt_access_ctxt *ctxt, void *arg);



struct ble_gatt_svc_def gGattServices[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &automation_service_uuid.u,
        .characteristics = (struct ble_gatt_chr_def[]) {
                {
                    .uuid = &digital_chr_uuid.u,
                    .flags = BLE_GATT_CHR_F_READ  | BLE_GATT_CHR_F_READ_ENC | BLE_GATT_CHR_F_NOTIFY,
                    .val_handle = &gBleBspChrs.LightStateChrHandle,
                    .access_cb = light_state_access_cb,
                    .descriptors = (struct ble_gatt_dsc_def[]){
                        {
                            .uuid = &presentation_dsc_uuid.u,
                            .att_flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
                            .access_cb = light_state_char_presentation_dsc_access_cb
                        },
                        {
                            .uuid = &num_digitals_dsc_uuid.u,
                            .att_flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_READ_ENC,
                            .access_cb = num_digitals_dsc_access_cb
                        },
                            BLE_DSC_OBJ_DESCRIPTION(description_dsc_access_cb, "Light state"),
                        {0}
                    }
                },
            {0}
        },
    },
    {0}
};

void BLE_GapEventCB(BLE_GapEventTypeDef Event, struct ble_gap_event *GapEvent, void *Arg) {
    switch (Event) {
        case BLE_GAP_EVENT_CONN_SUCCESS:
            LOGGER_LogF(LOGGER_LEVEL_INFO, "BLE Device %d connected!", GapEvent->connect.conn_handle);
            break;
        case BLE_GAP_EVENT_CONN_FAILED:
            LOGGER_Log(LOGGER_LEVEL_INFO, "BLE Device connection failed!");
            break;
        case BLE_GAP_EVENT_CONN_STORE_FAILED:
            LOGGER_LogF(LOGGER_LEVEL_INFO, "BLE Device %d connection store failed!", GapEvent->connect.conn_handle);
            break;
        case BLE_GAP_EVENT_CONN_UPD:
            LOGGER_LogF(LOGGER_LEVEL_INFO, "BLE Device %d connection updated!", GapEvent->conn_update.conn_handle);
            break;
        case BLE_GAP_EVENT_CONN_DISCONNECT:
            LOGGER_LogF(LOGGER_LEVEL_INFO, "BLE Device %d disconnected!", GapEvent->disconnect.conn.conn_handle);
            break;
        case BLE_GAP_EVENT_SUB:
            LOGGER_LogF(LOGGER_LEVEL_INFO, "BLE Device %d subscribed!", GapEvent->subscribe.conn_handle);
            break;
        case BLE_GAP_EVENT_CONN_ENC:
            LOGGER_Log(LOGGER_LEVEL_INFO, "BLE Connection encrypted");
            break;
        case BLE_GAP_EVENT_CONN_ENC_FAILED:
            LOGGER_LogF(LOGGER_LEVEL_ERROR, "BLE Connection could not be encrypted! Status code: %d", GapEvent->enc_change.status);
            break;
        case BLE_GAP_EVENT_UNSUB:
            LOGGER_LogF(LOGGER_LEVEL_INFO, "BLE Device %d unsubscribed!", GapEvent->subscribe.conn_handle);
            break;
        case BLE_GAP_EVENT_PASSKEY:
            if (GapEvent->passkey.params.action == BLE_SM_IOACT_DISP) {
                struct ble_sm_io pkey= {0};
                pkey.action = GapEvent->passkey.params.action;
                pkey.passkey = BLE_DEVICE_PASSWORD;
                ble_sm_inject_io(GapEvent->passkey.conn_handle, &pkey);
            }
            break;
        default:
            LOGGER_LogF(LOGGER_LEVEL_WARNING, "Unhandled GAP event %d!", Event);
            break;
    }
}

void BLE_GattRegEventCB(BLE_GattRegisterEventTypeDef Event, struct ble_gatt_register_ctxt *EventCtxt, void *Arg) {
    switch (Event) {
        case BLE_GATT_REG_EVENT_REG_SVC:
            LOGGER_LogF(LOGGER_LEVEL_INFO, "New service registered! Handle: 0x%04X", EventCtxt->svc.handle);
            break;
        case BLE_GATT_REG_EVENT_REG_CHR:
            LOGGER_LogF(LOGGER_LEVEL_INFO, "New service characteristic registered! Handle: 0x%04X", EventCtxt->svc.handle);
            break;
        case BLE_GATT_REG_EVENT_REG_DSC:
            LOGGER_LogF(LOGGER_LEVEL_INFO, "New characteristic descriptor registered! Handle: 0x%04X", EventCtxt->svc.handle);
            break;
        default:
            break;
    }
}

uint8_t BLE_GattSubscribeCB(struct ble_gap_event *event) {
    if (event->subscribe.attr_handle == gBleBspChrs.LightStateChrHandle) {
        uint8_t is_encrypted;
        if (BLE_CheckConnEncrypted(event->subscribe.conn_handle, &is_encrypted) != BLE_ERROR_OK || !is_encrypted) {
            return BLE_ATT_ERR_INSUFFICIENT_AUTHEN;
        }
    }
    return 0;
}

void BLE_ErrorCB(BLE_ErrorTypeDef Error) {
    LOGGER_LogF(LOGGER_LEVEL_ERROR, "An error occurred in BLE driver! Error code: %d", Error);
}

void BLE_AdvertiseSvcsCB(struct ble_hs_adv_fields *Fields) {
    Fields->uuids16 = (ble_uuid16_t[]){automation_service_uuid};
    Fields->num_uuids16 = 1;
    Fields->uuids16_is_complete = 1;
}

int light_state_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                        struct ble_gatt_access_ctxt *ctxt, void *arg) {
    uint8_t err = 0;
    SHVAL_ErrorTypeDef shval_err;

    switch (ctxt->op) {
        case BLE_GATT_ACCESS_OP_READ_CHR:
            uint32_t light_state;
            if ((shval_err = SHVAL_GetValue(&gAppState.SharedValues->LightState, &light_state, 1000)) != SHVAL_ERROR_OK) {
                LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to get shared LED light state! Error code: %d", shval_err);
                return BLE_ATT_ERR_UNLIKELY;
            }

            err = os_mbuf_append(ctxt->om, &light_state, 1);

            return err == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
            break;
        case BLE_GATT_ACCESS_OP_WRITE_CHR:
            if (ctxt->om->om_len != 1) return BLE_ATT_ERR_VALUE_NOT_ALLOWED;
            uint8_t write_val = *ctxt->om->om_data;
            if (write_val > 1) return BLE_ATT_ERR_VALUE_NOT_ALLOWED;

            if ((shval_err = SHVAL_SetValue(&gAppState.SharedValues->LightState, write_val, 1000)) != SHVAL_ERROR_OK) {
                LOGGER_LogF(LOGGER_LEVEL_ERROR, "Failed to set shared light state variable! Error code: %d", shval_err);
                return BLE_ATT_ERR_UNLIKELY;
            }
            return 0;
            break;
        default:
            break;
    }

    return BLE_ATT_ERR_UNLIKELY;
}

int light_state_char_presentation_dsc_access_cb(uint16_t conn_handle, uint16_t attr_handle,
    struct ble_gatt_access_ctxt *ctxt, void *arg) {
    uint8_t format = 0x01;                  // Boolean
    return os_mbuf_append(ctxt->om, &format, 1);
}

int num_digitals_dsc_access_cb(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt,
    void *arg) {
    uint8_t count = 1;
    return os_mbuf_append(ctxt->om, &count, 1);
}

int description_dsc_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg) {
    if (ctxt->op != BLE_GATT_ACCESS_OP_READ_DSC) return BLE_ATT_ERR_UNLIKELY;
    const char *name = (const char*)arg;
    return os_mbuf_append(ctxt->om, name, strlen(name));
}
