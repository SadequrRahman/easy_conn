#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "BleDevice.h"


 



uint8_t SERVICE_UUID[] = {0x00, 0x12};//{0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33};
uint8_t CHAR_UUID[] = {0x01, 0x12};//{0x00, 0x00, 0x00, 0x02, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33};
uint8_t CHAR_UUID1[] = {0x02, 0x12};//{0x00, 0x00, 0x00, 0x03, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33};
uint8_t CHAR_UUID2[] = {0x03, 0x12};
uint16_t DES_UUID =  0x2902;//{0x00, 0xA0, 0x01, 0x02, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33};
static const char *TAG = "main";
ble_profile_t* profile = (void*)0;
bleDevice_config_t *conf = (void*)0;
ble_char_t* character = NULL; 
ble_char_t* character1 = NULL;
ble_char_t* character2 = NULL;
ble_descrp_t* description = NULL;
static volatile uint32_t cnt = 0;
uint8_t char1_str[] =  "Hello Auto resp";
uint8_t *desValue = NULL;
uint8_t char1_str1[] = "Second Char";
uint8_t test_string[] = "This is a long string to check if the esp32 ble stack auto handle buffer larger then 22 bytes";
bool isConnected = false;
uint8_t ni_key = false;  // notification and indication key
bool isConfirm_needed = false;

// function declearation 
void descrp_write_handler(esp_ble_gatts_cb_param_t* param);
void notificationTask(void* pvParam);
void char2ReadCallback(esp_ble_gatts_cb_param_t* param);
void cha2WriteCallback(esp_ble_gatts_cb_param_t* param);



void app_main(void){

    esp_err_t ret;
	/* Initialize NVS. */
	ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK( ret );

    conf = BleDevice_getDefaultConfig();
	BleDevice_init(conf); 
    //BleDevice_setEventhandler(belDevice_events);
	// create default profile
	profile = BleProfiles_createProfile();
	BleDevice_addProfile(profile);
	ble_service_t* service =  BleProfiles_createService((uint8_t*)&SERVICE_UUID, ESP_UUID_LEN_16, 0x08, true);
	BleProfiles_addService(profile, service);

    character = BleProfiles_createCharacteristic((uint8_t*)&CHAR_UUID, ESP_UUID_LEN_16, ESP_GATT_AUTO_RSP);
	BleProfiles_setCharacteristicPermission(character, ESP_GATT_PERM_READ );
	BleProfiles_setCharacteristicProperty(character, ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY );
	BleProfile_setCharacteristicValue(character, (uint8_t*)&cnt, sizeof(cnt), 0x40);
	//
	character1 = BleProfiles_createCharacteristic((uint8_t*)&CHAR_UUID1, ESP_UUID_LEN_16, ESP_GATT_AUTO_RSP );
	BleProfiles_setCharacteristicPermission(character1, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE );
	BleProfiles_setCharacteristicProperty(character1, ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE );
	BleProfile_setCharacteristicValue(character1, char1_str1, sizeof(char1_str1), 0x40);

	character2 = BleProfiles_createCharacteristic((uint8_t*)&CHAR_UUID2, ESP_UUID_LEN_16, ESP_GATT_RSP_BY_APP );
	BleProfiles_setCharacteristicPermission(character2, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE );
	BleProfiles_setCharacteristicProperty(character2, ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE );
	BleProfile_setCharacteristicValue(character2, test_string, sizeof(test_string), 0xFF);
	character2->mReadEvent = char2ReadCallback;
	character2->mWriteEvent = cha2WriteCallback;
	//

    description = BleProfiles_createDescription((uint8_t*)&DES_UUID, ESP_UUID_LEN_16, ESP_GATT_AUTO_RSP);
	BleProfiles_setDescriptionPermission(description,  ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE );
	BleProfile_setDescriptionValue(description, (uint8_t*)&desValue, 0 , 0x40);
	description->mWriteEvent = NULL;//descrp_write_handler;
	

	BleProfiles_addCharacteristic(service, character);
    BleProfiles_addDescription(character,description);
	BleProfiles_addCharacteristic(service, character1);
	BleProfiles_addCharacteristic(service, character2);
    esp_err_t rt = BleDevice_activateProfiles();
	if( rt == ESP_ERR_TIMEOUT)
         ESP_LOGI(TAG, "ble activation timeout\r\n");
    else if(rt != ESP_OK)
        ESP_LOGI(TAG, "ble activation eror code: %d\r\n", (int)rt);
    else
        xTaskCreate(notificationTask, "notificationTask", 1024 * 4, NULL, 5, NULL);
}



void notificationTask(void* pvParam)
{
	while (true){
		
		if(description){
            cnt++;
			if(description->mNotifyKey == 1)
				esp_ble_gatts_send_indicate(profile->mGatt_if, profile->mConn_id, character->mhandle, sizeof(cnt), (uint8_t*)&cnt, description->mIsConfirmNeeded);
			else if(description->mNotifyKey == 2)
				esp_ble_gatts_send_indicate(profile->mGatt_if, profile->mConn_id, character->mhandle, sizeof(cnt), (uint8_t*)&cnt, description->mIsConfirmNeeded);
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}


void descrp_write_handler(esp_ble_gatts_cb_param_t* param)
{
	uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
	if(descr_value == 0x0001){ // enable notification
		description->mNotifyKey = 1;
		description->mIsConfirmNeeded = false;
	}
	else if (descr_value == 0x0002){ // enable indication
		description->mNotifyKey = 2;
		description->mIsConfirmNeeded = true;
	}
	else if (descr_value == 0x0000) // disable indication or notification.
		description->mNotifyKey = 0;
}


void char2ReadCallback(esp_ble_gatts_cb_param_t* param)
{
	// in gatts callback
	esp_gatt_rsp_t response;
	response.handle = param->read.handle;
	BleProfile_prepareLongRsp(&response, test_string, strlen((const char*)test_string), param->read.offset);
	esp_ble_gatts_send_response(profile->mGatt_if, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &response);
}

void cha2WriteCallback(esp_ble_gatts_cb_param_t* param)
{
    ESP_LOGI(TAG, "Write event");
}
