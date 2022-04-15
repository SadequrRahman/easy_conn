#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "BleDevice.h"


uint8_t CHAR_UUID1[] = {0x00, 0x00, 0x00, 0x03, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33};
uint8_t SERVICE_UUID[] = {0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33};
uint8_t CHAR_UUID[] = {0x00, 0x00, 0x00, 0x02, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33};
uint8_t DES_UUID[] = {0x00, 0x00, 0x00, 0x04, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33};
static const char *TAG = __FILE__;
ble_profile_t* profile = (void*)0;
bleDevice_config_t *conf = (void*)0;
ble_char_t* ch = NULL; 
ble_char_t* ch1 = NULL;
uint8_t cnt = 0;
uint8_t char1_str[] = "First Char";
uint8_t desValue[] = "This is a test description";
uint8_t char1_str1[] = "Second Char";

void notificationTask(void* args)
{
	while (true)
	{
		cnt++;
		if(profile && ch)
		  esp_ble_gatts_send_indicate(profile->mGatt_if, profile->mConn_id, ch->mChar_handle, 1, &cnt, false);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	
}

void char1ReadCallback(ble_eventParam_t param)
{
	ESP_LOGI(TAG, "%s\n", __func__);
	esp_gatt_rsp_t rsp;  
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));  
	rsp.attr_value.handle = ch->mChar_handle;  
	rsp.attr_value.len = 1;  
	rsp.attr_value.value[0] = cnt; 
	esp_ble_gatts_send_response(param.mGatts_if,  
								param.mConn_id,  
								param.mTrans_id,  
								ESP_GATT_OK, &rsp);
}

void char2ReadCallback(ble_eventParam_t param)
{
	ESP_LOGI(TAG, "%s\n", __func__);
	esp_gatt_rsp_t rsp; 
	memset(&rsp, 0, sizeof(esp_gatt_rsp_t));  
	rsp.handle = ch1->mChar_handle;
	memcpy((void*)&rsp.attr_value, (void*)ch1->mAtt, sizeof(esp_attr_value_t));
	esp_ble_gatts_send_response(param.mGatts_if,  
								param.mConn_id,  
								param.mTrans_id,  
								ESP_GATT_OK, &rsp);
}


void app_main(void)
{
    printf("%s\r\n", CONFIG_IDF_TARGET);
    printf("%s\r\n", CONFIG_BLE_CONN_NAME);
    esp_err_t ret;
	/* Initialize NVS. */
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );

    conf = BleDevice_getDefaultConfig();
	BleDevice_init("OnTest", conf);
	// create default profile
	profile = BleProfiles_createProfile();
	BleDevice_addProfile(profile);
	ble_service_t* service =  BleProfiles_createService((uint8_t*)&SERVICE_UUID, ESP_UUID_LEN_128, 0x06, true);
	BleProfiles_addService(profile, service);

    ch = BleProfiles_createCharacteristic((uint8_t*)&CHAR_UUID, ESP_UUID_LEN_128, ESP_GATT_RSP_BY_APP);
	BleProfiles_setCharacteristicPermission(ch, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE );
	BleProfiles_setCharacteristicProperty(ch, (ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY) );
	// BleProfile_setCharacteristicValue(ch, &cnt, sizeof(cnt), 0x01);
	BleProfiles_setReadCallback(ch, char1ReadCallback);
	//
	ch1 = BleProfiles_createCharacteristic((uint8_t*)&CHAR_UUID1, ESP_UUID_LEN_128, ESP_GATT_AUTO_RSP );
	BleProfiles_setCharacteristicPermission(ch1, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE );
	BleProfiles_setCharacteristicProperty(ch1, (ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY) );
	BleProfile_setCharacteristicValue(ch1, char1_str1, sizeof(char1_str1), 0x40);
	BleProfiles_setReadCallback(ch1,char2ReadCallback);
	//
	ble_descrp_t* des = BleProfiles_createDescription((uint8_t*)&DES_UUID, ESP_UUID_LEN_128, ESP_GATT_AUTO_RSP);
	BleProfiles_setDescriptionPermission(des, ESP_GATT_PERM_READ );
	BleProfile_setDescriptionValue(des, desValue, sizeof(desValue), 0x40);

	BleProfiles_addCharacteristic(service, ch);
	BleProfiles_addCharacteristic(service, ch1);
	BleProfiles_addDescription(ch,des);
	BleDevice_activateProfiles();

	xTaskCreate(notificationTask, "notificationTask", 1024 * 4, NULL, 5, NULL);

}
