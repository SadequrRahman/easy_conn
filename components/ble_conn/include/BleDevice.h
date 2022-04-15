/*
 * BleDevice.h
 *
 *  Created on: 14 Nov 2020
 *      Author: Mohammad Sadequr Rahman
 */

#ifndef INC_BLEDEVICE_H_
#define INC_BLEDEVICE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "BleProfiles.h"
#include "uList.h"


typedef struct{
	esp_ble_adv_data_t mAdvData;
	esp_ble_adv_params_t mAdvParams;
}bleDevice_config_t;

typedef struct{
	char* mName;
	uint8_t mProfileCount;
	bleDevice_config_t* mConfig;
	uList_t* mProfileList;
}bleDevice_handler_t;


typedef enum{
	connect_event,
	disconnnect_event
}BLE_EVENT_ID;

typedef struct {

}ble_event_t;

typedef void(*event_handler)(ble_event_t *param);



// init the ble device with device name
void BleDevice_init(char* pName, bleDevice_config_t* config);
bleDevice_config_t* BleDevice_getDefaultConfig(void);


// get the ble device handler. this will use to add profiles
// return null_ptr if the ble device does not initialised
bleDevice_handler_t* BleDevice_getHandler(void);


// get the number of profile that are added to this device
uint8_t BleDevice_getProfileCount(void);


void BleDevice_addProfile(ble_profile_t* pProfile);

// to activate all profile after configuring service and characteristics
void BleDevice_activateProfiles(void);



#endif /* INC_BLEDEVICE_H_ */
