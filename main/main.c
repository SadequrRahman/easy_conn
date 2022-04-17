#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "freertos/event_groups.h"
#include "BleDevice.h"


 



uint8_t SERVICE_UUID[] = {0x12, 0x02};//{0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33};
uint8_t CHAR_UUID[] = {0x12, 0x01};//{0x00, 0x00, 0x00, 0x02, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33};
uint8_t CHAR_UUID1[] = {0x12, 0x02};//{0x00, 0x00, 0x00, 0x03, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33};
uint16_t DES_UUID =  0x2902;//{0x00, 0xA0, 0x01, 0x02, 0x10, 0x00, 0x20, 0x00, 0x30, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33};
static const char *TAG = "main";
ble_profile_t* profile = (void*)0;
bleDevice_config_t *conf = (void*)0;
ble_char_t* ch = NULL; 
ble_char_t* ch1 = NULL;
ble_descrp_t* des = NULL;
uint16_t cnt = 0;
uint8_t char1_str[] = "First Char";
uint8_t desValue[] = "Notification Disable";
uint8_t char1_str1[] = "Second Char";
bool isConnected = false;

void notificationTask(void* args)
{
	while (true)
	{
		cnt++;
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
	
}

// void belDevice_events(ble_event_t e)
// {
//     switch (e._id)
//     {
//     case ble_device_connect:
//         ESP_LOGI(TAG, "ble_device_connect\n");
//         isConnected = true;
//         break;
//     case ble_device_disconnect:
//         ESP_LOGI(TAG, "ble_device_disconnect\n");
//         isConnected = false;
//         break;
//     case ble_device_start_advertising: 
//         ESP_LOGI(TAG, "ble_device_start_advertising\n");
//         break;
//     case ble_device_stop_advertising:
//         ESP_LOGI(TAG, "ble_device_stop_advertising\n");
//         break;
//     case ble_device_received_confirm:
//         ESP_LOGI(TAG, "ble_device_received_confirm\n");
//         break;
//     default:
//         break;
//     }
// }

void desWriteCallback(uint8_t* value, uint16_t len)
{
	ESP_LOGI(TAG, "%s\n", __func__);
	uint16_t descr_value= value[1]<<8 | value[0];
	if(descr_value == 0x0001)
	{
		ESP_LOGI(TAG, "notify enable\n");
		//the size of notify_data[] need less than MTU size
		esp_ble_gatts_send_indicate(profile->mGatt_if, profile->mConn_id,  
									des->mAssociate_char_handle,  
									sizeof(cnt),  
									(uint8_t*)&cnt, false);
	}
	else if (descr_value == 0x0002)
	{
		ESP_LOGI(TAG, "indicate enable\n");
		esp_ble_gatts_send_indicate(profile->mGatt_if, profile->mConn_id,  
									des->mAssociate_char_handle,  
									sizeof(cnt),  
									(uint8_t*)&cnt, true);
	}
	else if (descr_value == 0x0000)
	{
		ESP_LOGI(TAG, "notify/indicate disable\n");
	}	
	else
		ESP_LOGE(TAG, "unknown value\n");
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
	BleDevice_init(conf); 
    //BleDevice_setEventhandler(belDevice_events);
	// create default profile
	profile = BleProfiles_createProfile();
	BleDevice_addProfile(profile);
	ble_service_t* service =  BleProfiles_createService((uint8_t*)&SERVICE_UUID, ESP_UUID_LEN_16, 0x06, true);
	BleProfiles_addService(profile, service);

    ch = BleProfiles_createCharacteristic((uint8_t*)&CHAR_UUID, ESP_UUID_LEN_16, ESP_GATT_AUTO_RSP);
	BleProfiles_setCharacteristicPermission(ch, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE );
	BleProfiles_setCharacteristicProperty(ch, ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY );
	BleProfile_setCharacteristicValue(ch, (uint8_t*)&cnt, sizeof(cnt), 0x40);
	//
	ch1 = BleProfiles_createCharacteristic((uint8_t*)&CHAR_UUID1, ESP_UUID_LEN_16, ESP_GATT_AUTO_RSP );
	BleProfiles_setCharacteristicPermission(ch1, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE );
	BleProfiles_setCharacteristicProperty(ch1, ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY );
	BleProfile_setCharacteristicValue(ch1, char1_str1, sizeof(char1_str1), 0x40);
	
	//

    des = BleProfiles_createDescription((uint8_t*)&DES_UUID, ESP_UUID_LEN_16, ESP_GATT_AUTO_RSP);
	BleProfiles_setDescriptionPermission(des,  ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE );
	BleProfile_setDescriptionValue(des, (uint8_t*)&desValue, sizeof(desValue), 0x40);
	des->mWriteEvent = desWriteCallback;
	

	BleProfiles_addCharacteristic(service, ch);
    BleProfiles_addDescription(ch,des);
	BleProfiles_addCharacteristic(service, ch1);
	BleDevice_activateProfiles();
	xTaskCreate(notificationTask, "notificationTask", 1024 * 4, NULL, 5, NULL);
}
