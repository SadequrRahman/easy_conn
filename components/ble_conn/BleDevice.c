/*
 * BleDevice.c
 *
 *  Created on: 14 Nov 2020
 *  Author: Mohammad Sadequr Rahman
 *  Email: sadequr.rahman.rabby@gmnail.com 
 */


#include "BleDevice.h"
#include "BleProfiles.h"
#include "freertos/task.h"


#define _wait(flag)	do{											\
						vTaskDelay( 50 / portTICK_PERIOD_MS);	\
					}while(!flag)									


// private function
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
static uint8_t defaultAdvUUID128[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0x89, 0x78, 0x67, 0x56, 0x45, 0x34, 0x23, 0x12 };

// private variable 
static volatile uint16_t _mHandlerContainer = 0;
static volatile uint8_t isJobDone = 0;
static const char *TAG = "BleDevice";
static bleDevice_handler_t *mDeviceHandler = NULL;




void BleDevice_init(bleDevice_config_t *config)
{
	if(!mDeviceHandler)
	{
		mDeviceHandler = (bleDevice_handler_t*) malloc(sizeof(bleDevice_handler_t));
		memset((void*)mDeviceHandler, 0, sizeof(bleDevice_handler_t));
		mDeviceHandler->mName = CONFIG_BLE_CONN_NAME;
		mDeviceHandler->mProfileCount = 0;
		mDeviceHandler->mProfileList =  uList_createList();
		mDeviceHandler->mConfig = config;
		// start init the ESP specific
		esp_err_t ret;
		ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
		esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
		ret = esp_bt_controller_init(&bt_cfg);
		if (ret) {
			ESP_LOGE(TAG, "%s enable controller failed: %s", __func__,
					esp_err_to_name(ret));
			return;
		}
		ESP_LOGI(TAG, "Bluetooth initializing done.");
		ret = esp_bt_controller_enable(ESP_BT_MODE_BTDM);
		if (ret) {
			ESP_LOGE(TAG, "%s enable controller failed: %s", __func__,
					esp_err_to_name(ret));
			return;
		}
		ESP_LOGI(TAG, "Bluetooth mode set to Low energy only");
		ret = esp_bluedroid_init();
		if (ret) {
			ESP_LOGE(TAG, "%s init bluetooth failed: %s", __func__,
					esp_err_to_name(ret));
			return;
		}
		ESP_LOGI(TAG, "bluedroid initialized");
		ret = esp_bluedroid_enable();
		if (ret) {
			ESP_LOGE(TAG, "%s enable bluetooth failed: %s", __func__,
					esp_err_to_name(ret));
			return;
		}
		ESP_LOGI(TAG, "bluedroid enabled");
		ret = esp_ble_gatts_register_callback(gatts_event_handler);
		if (ret) {
			ESP_LOGE(TAG, "gatts register error, error code = %x", ret);
			return;
		}
		ESP_LOGI(TAG, "GATT event handler registered");
		ret = esp_ble_gap_register_callback(gap_event_handler);
		if (ret) {
			ESP_LOGE(TAG, "gap register error, error code = %x", ret);
			return;
		}
		ESP_LOGI(TAG, "GAP event handler registered");
		esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(512);
		if (local_mtu_ret) {
			ESP_LOGE(TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
		}
		ESP_LOGI(TAG, "Local mtu set to 512");
		ret = esp_ble_gap_set_device_name(mDeviceHandler->mName);
		if (ret) {
			ESP_LOGE(TAG, "Set device name failed, error code = %x", ret);
		}
		ret = esp_ble_gap_config_adv_data(&mDeviceHandler->mConfig->mAdvData);
		if (ret) {
			ESP_LOGE(TAG, "config adv data failed, error code = %x", ret);
		}

	}

}


bleDevice_handler_t *BleDevice_getHandler(void)
{
	return mDeviceHandler;
}

bleDevice_config_t* BleDevice_getDefaultConfig(void)
{
	bleDevice_config_t * config = (bleDevice_config_t*) malloc(sizeof(bleDevice_config_t ));
	memset((void*)config, 0, sizeof(bleDevice_config_t));
	// adv data
	config->mAdvData.set_scan_rsp = true;
	config->mAdvData.include_name = true;
	config->mAdvData.include_txpower = true;
	config->mAdvData.min_interval = 0x0006;
	config->mAdvData.max_interval = 0x0010;
	config->mAdvData.appearance = 0x00;
	config->mAdvData.manufacturer_len = 0;
	config->mAdvData.p_manufacturer_data = NULL;
	config->mAdvData.service_data_len = 0;
	config->mAdvData.p_service_data = NULL;
	config->mAdvData.service_uuid_len = sizeof(defaultAdvUUID128);
	config->mAdvData.p_service_uuid = defaultAdvUUID128;
	config->mAdvData.flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);
	// adv params
	config->mAdvParams.adv_int_min = 0x20;
	config->mAdvParams.adv_int_max = 0x40;
	config->mAdvParams.adv_type = ADV_TYPE_IND;
	config->mAdvParams.own_addr_type = BLE_ADDR_TYPE_PUBLIC;
	config->mAdvParams.channel_map = ADV_CHNL_ALL;
	config->mAdvParams.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;

	return config;
}

uint8_t BleDevice_getProfileCount(void)
{
	uint8_t ret = 0;
	if(mDeviceHandler)
	 	ret = mDeviceHandler->mProfileCount;
	return ret;
}

const char* BleDevice_getDeviceName(void)
{
	char* ret = NULL;
	if(mDeviceHandler)
		ret = mDeviceHandler->mName;
	return (const char*) ret;
}

void BleDevice_addProfile(ble_profile_t* pProfile)
{
	if(pProfile!=0 && mDeviceHandler!=0)
	{
		uNode_t *node = uList_createNode((void*)pProfile, sizeof(ble_profile_t),NODE_NON_SELF_ALLOC);
		uList_append(mDeviceHandler->mProfileList, node);
		mDeviceHandler->mProfileCount++;
	}

}

void BleDevice_activateProfiles(void)
{
	if(mDeviceHandler)
 	{
		void* value = NULL;
		uint16_t len;
		ITERATE_LIST(mDeviceHandler->mProfileList, value, len, 
			isJobDone = 0;
			ble_profile_t* profile = (ble_profile_t*)value;
			esp_ble_gatts_app_register(profile->mId);
			_wait(isJobDone);
			profile->mGatt_if = _mHandlerContainer;
		ITERATE_LIST(profile->mServiceList, value, len,
			isJobDone = 0;
			ble_service_t* service = (ble_service_t*)value;
			esp_ble_gatts_create_service(profile->mGatt_if, service->mService_id, service->mNumHandle);
			_wait(isJobDone);
			service->mHandle = _mHandlerContainer;
		ITERATE_LIST(service->mCharList, value, len,
				isJobDone = 0;
				ble_char_t* characteristic = (ble_char_t*)value;
				characteristic->mAssociateService_handle = service->mHandle;
				esp_ble_gatts_add_char(service->mHandle, characteristic->mChar_uuid, characteristic->mPerm, characteristic->mProperty, characteristic->mAtt, &characteristic->mRsp);
				_wait(isJobDone);
				characteristic->mhandle = _mHandlerContainer;
		ITERATE_LIST(characteristic->mDescrList, value, len,
				isJobDone = 0;
				ble_descrp_t* descrp = (ble_descrp_t*)value;
				descrp->mAssociateChar_handle = characteristic->mhandle;
				esp_ble_gatts_add_char_descr( service->mHandle, descrp->mDescr_uuid, descrp->mPerm, descrp->mAtt, &descrp->mRsp);
				_wait(isJobDone);
				descrp->mhandle = _mHandlerContainer;
		);
		);
		);
		);
	 }
}



void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
	switch (event) 
	{
		case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
			esp_ble_gap_set_device_name(mDeviceHandler->mName);
			esp_ble_gap_start_advertising(&mDeviceHandler->mConfig->mAdvParams);
			break;
		case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
			ESP_LOGI(TAG, "ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT");
			esp_ble_gap_start_advertising(&mDeviceHandler->mConfig->mAdvParams);
			break;
		case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
			//advertising start complete event to indicate advertising start successfully or failed
			if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) 
			{
				ESP_LOGE(TAG, "Advertising start failed\n");
			} else 
			{
				ESP_LOGI(TAG, "Ble Advertising");
			}
			break;
		case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
			if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) 
			{
				ESP_LOGE(TAG, "Advertising stop failed\n");
			} else 
			{
				ESP_LOGI(TAG, "Stop adv successfully\n");
			}
			break;
		case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
			ESP_LOGI(TAG,
			"update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
			param->update_conn_params.status,
			param->update_conn_params.min_int,
			param->update_conn_params.max_int,
			param->update_conn_params.conn_int,
			param->update_conn_params.latency,
			param->update_conn_params.timeout);
			break;
		case ESP_GAP_BLE_GET_BOND_DEV_COMPLETE_EVT:
			ESP_LOGI(TAG, "ESP_GAP_BLE_GET_BOND_DEV_COMPLETE_EVT");
			break;
		case ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT:
			ESP_LOGI(TAG, "ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT");
			break;
		case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT:
			ESP_LOGI(TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT");
			break;
		default:
			break;
		
	}
}


void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
	switch (event)
	{
		case ESP_GATTS_REG_EVT: /*!< When register application id, the event comes */
			_mHandlerContainer = gatts_if;
			isJobDone = 1;
			break;
		case ESP_GATTS_READ_EVT:			/*!< When gatt client request read operation, the event comes */
			ESP_LOGI(TAG, "GATT_READ_EVT, conn_id %d, trans_id %d, handle %d\n", param->read.conn_id, param->read.trans_id, param->read.handle);
			bool isFound = false;
			void* value = NULL;
			uint16_t len;
			ITERATE_LIST(mDeviceHandler->mProfileList, value, len,
			if (isFound) break;
			ESP_LOGI(TAG, "Profile iterator\n");
			ble_profile_t* profile = (ble_profile_t*)value;
				if(profile->mGatt_if == gatts_if)
				{
					ITERATE_LIST(profile->mServiceList, value, len, 
					ESP_LOGI(TAG, "Service iterator\n");
					ble_service_t* service = (ble_service_t*)value;
					ITERATE_LIST(service->mCharList, value, len,
					if (isFound) break;
					ESP_LOGI(TAG, "Characteristics iterator\n");
					ble_char_t* characteristic = (ble_char_t*)value;
					if(characteristic->mhandle == param->read.handle){
						ESP_LOGI(TAG, "Found Char with proper handler\n");
						if(characteristic->mReadEvent && characteristic->mRsp.auto_rsp == ESP_GATT_RSP_BY_APP)
							characteristic->mReadEvent(param);
						isFound = true;
					}
					ITERATE_LIST(characteristic->mDescrList, value, len,
						if (isFound) break;
						ble_descrp_t* descrp = (ble_descrp_t*)value;
						ESP_LOGI(TAG, "Description iterator\n");
						if(descrp->mhandle == param->read.handle ){
							if (descrp->mReadEvent && descrp->mRsp.auto_rsp == ESP_GATT_RSP_BY_APP)
								descrp->mReadEvent(param);
							isFound = true;
						}
					);
					);
					);
				}
			);
			break;
		case ESP_GATTS_WRITE_EVT:			/*!< When gatt client request write operation, the event comes */
			if(mDeviceHandler)
			{
				bool isFound = false;
				void* value = NULL;
				uint16_t len;
				ITERATE_LIST(mDeviceHandler->mProfileList, value, len, 
					if (isFound) break;
					ble_profile_t* profile = (ble_profile_t*)value;
					if(profile->mGatt_if == gatts_if)
					{
						ITERATE_LIST(profile->mServiceList, value, len,
							if (isFound) break;
							ble_service_t* service = (ble_service_t*)value;
							ITERATE_LIST(service->mCharList, value, len,
								if (isFound) break;
								ble_char_t* characteristic = (ble_char_t*)value;
								if(characteristic->mhandle == param->write.handle)
								{
									if(characteristic->mWriteEvent && characteristic->mRsp.auto_rsp == ESP_GATT_RSP_BY_APP)
										characteristic->mWriteEvent(param);
									isFound = true;
								}
								ITERATE_LIST(characteristic->mDescrList, value, len,
									if (isFound) break;
									ble_descrp_t* descrp = (ble_descrp_t*)value;
									if(descrp->mhandle == param->write.handle)
									{
										if(descrp->mWriteEvent)
											descrp->mWriteEvent(param);
										isFound = true;
									}
								);
							);
						);
					}
				);
			}
			break;
		case ESP_GATTS_EXEC_WRITE_EVT:                 /*!< When gatt client request execute write, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_EXEC_WRITE_EVT");
			break;
		case ESP_GATTS_MTU_EVT:                        /*!< When set mtu complete, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_MTU_EVT");
			break;
		case ESP_GATTS_CONF_EVT:                       /*!< When receive confirm, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_CONF_EVT");
			break;
		case ESP_GATTS_UNREG_EVT:                      /*!< When unregister application id, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_UNREG_EVT");
			break;
		case ESP_GATTS_CREATE_EVT:                     /*!< When create service complete, the event comes */
			ESP_LOGI(TAG, "CREATE_SERVICE_EVT, status %d, service_handle %d\n", param->create.status, param->create.service_handle);
			_mHandlerContainer = param->create.service_handle;	
			isJobDone = 1;
			esp_ble_gatts_start_service(param->create.service_handle);
			break;
		case ESP_GATTS_ADD_INCL_SRVC_EVT:              /*!< When add included service complete, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_ADD_INCL_SRVC_EVT");
			break;
		case ESP_GATTS_ADD_CHAR_EVT:                   /*!< When add characteristic complete, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_ADD_CHAR_EVT. Serive Handle: %d", param->add_char.service_handle);
			uint16_t length = 0;
			const uint8_t *prf_char;
			esp_err_t get_attr_ret = esp_ble_gatts_get_attr_value(param->add_char.attr_handle, &length, &prf_char);
			if (get_attr_ret == ESP_FAIL)
			{
			   ESP_LOGE(TAG, "ILLEGAL HANDLE");
			}
			_mHandlerContainer = param->add_char.attr_handle;
			isJobDone = 1;
			break;
		case ESP_GATTS_ADD_CHAR_DESCR_EVT:            /*!< When add descriptor complete, the event comes */
			ESP_LOGI(TAG, "ADD_DESCR_EVT, status %d, attr_handle %d, service_handle %d\n",
			                  param->add_char_descr.status, param->add_char_descr.attr_handle,
			                  param->add_char_descr.service_handle);
			_mHandlerContainer = param->add_char_descr.attr_handle;
			isJobDone = 1;
			break;
		case ESP_GATTS_DELETE_EVT:                    /*!< When delete service complete, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_DELETE_EVT");
			break;
		case ESP_GATTS_START_EVT:                     /*!< When start service complete, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_START_EVT");
			break;
		case ESP_GATTS_STOP_EVT:                      /*!< When stop service complete, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_STOP_EVT");
			break;
		case ESP_GATTS_CONNECT_EVT: 	/*!< When gatt client connect, the event comes */
		{
			esp_ble_conn_update_params_t conn_params = { 0 };
			memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
			/* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
			conn_params.latency = 0;
			conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
			conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
			conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
			ESP_LOGI(TAG,
					"ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
					param->connect.conn_id, param->connect.remote_bda[0],
					param->connect.remote_bda[1], param->connect.remote_bda[2],
					param->connect.remote_bda[3], param->connect.remote_bda[4],
					param->connect.remote_bda[5]);
			ble_profile_t* profile = (void*)0;
			uNode_t * profileNode = mDeviceHandler->mProfileList->tail;
			while (profileNode)
			{
				/* code */
				profile = (ble_profile_t*)profileNode->value;
				if(profile->mGatt_if == gatts_if)
				{
					profile->mConn_id = param->connect.conn_id;
					break;
				}
				profileNode = profileNode->nextNode;
			}
			//start sent the update connection parameters to the peer device.
			esp_ble_gap_update_conn_params(&conn_params);
		}
			break;
		case ESP_GATTS_DISCONNECT_EVT: 					/*!< When gatt client disconnect, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_DISCONNECT_EVT.\r\n disconnect reason 0x%x", param->disconnect.reason);
			esp_ble_gap_start_advertising(&mDeviceHandler->mConfig->mAdvParams);
			break;
		case ESP_GATTS_OPEN_EVT:                      /*!< When connect to peer, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_OPEN_EVT");
			break;
		case ESP_GATTS_CANCEL_OPEN_EVT:               /*!< When disconnect from peer, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_CANCEL_OPEN_EVT");
			break;
		case ESP_GATTS_CLOSE_EVT:                     /*!< When gatt server close, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_CLOSE_EVT");
			break;
		case ESP_GATTS_LISTEN_EVT:                    /*!< When gatt listen to be connected the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_LISTEN_EVT");
			break;
		case ESP_GATTS_CONGEST_EVT:                   /*!< When congest happen, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_CONGEST_EVT");
			break;
		/* following is extra event */
		case ESP_GATTS_RESPONSE_EVT:                  /*!< When gatt send response complete, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_RESPONSE_EVT");
			break;
		case ESP_GATTS_CREAT_ATTR_TAB_EVT:            	/*!< When gatt create table complete, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_CREAT_ATTR_TAB_EVT");
			break;
		case ESP_GATTS_SET_ATTR_VAL_EVT:     			/*!< When gatt set attr value complete, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_SET_ATTR_VAL_EVT");
			break;
		case ESP_GATTS_SEND_SERVICE_CHANGE_EVT:	      	/*!< When gatt send service change indication complete, the event comes */
			ESP_LOGI(TAG, "ESP_GATTS_SEND_SERVICE_CHANGE_EVT");
			break;
		default:
			break;
	}

}



