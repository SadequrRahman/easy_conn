/*
	File Name: BleManager.h
	Author: Sadequr Rahman
	Date: 03/13/2020
	Description:
*/



#ifndef _BLEMANAGER_H_
#define _BLEMANAGER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "uList.h"
#include "sdkconfig.h"


typedef struct {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
}esp_gatt_profile_t;


typedef struct {
	uint16_t gatts_if;
	uint16_t app_id;
	uint16_t conn_id;
	uint16_t service_handle;
	esp_gatt_srvc_id_t service_id;
	uint16_t num_handles;
}gatts_service_t;

typedef struct {
	uint32_t service_pos;
	esp_bt_uuid_t char_uuid;
	esp_gatt_perm_t char_perm;
	esp_gatt_char_prop_t char_property;
	esp_attr_value_t *char_val;
	esp_attr_control_t *char_control;
	uint16_t char_handle;
	char char_nvs[16];
}gatts_char_t;

typedef struct {
	uint32_t char_pos;
	esp_bt_uuid_t descr_uuid;
	esp_gatt_perm_t descr_perm;
	esp_attr_value_t *descr_val;
	esp_attr_control_t *descr_control;
	uint16_t descr_handle;
}gatts_descr_t;




void BleManager_init(char *deviceName, uint8_t applicationID);
const char* BleManager_getDeviceName(void);


// gap
esp_ble_adv_data_t * BleManager_getDefaultAdvData(void);
esp_ble_adv_params_t * BleManager_getDefaultAdvertiseParam(void);
esp_ble_adv_data_t * BleManager_getDefaultAdvRespData(void);
void BleManager_setAdvParams(esp_ble_adv_params_t* param);
esp_err_t BleManager_setAdvData(esp_ble_adv_data_t* advData);
esp_err_t BleManager_setAdvRespData(esp_ble_adv_data_t* scanRespData);

// gatt
esp_gatt_profile_t* BleManager_createProfile(void);
int BleManager_addProfile(esp_gatt_profile_t* profile);
//esp_gatt_service_t* BleManager_createService(void);
//esp_gatt_char_t* BleManager_createCharacteristic(void);
//esp_gatt_description_t* BleManager_createDescription(void);


#endif 
