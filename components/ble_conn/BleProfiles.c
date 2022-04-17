/*
 * BleProfiles.c
 *
 *  Created on: 15 Nov 2020
 *      Author: Rabby
 */


#include "include/BleProfiles.h"

static uint16_t mProfileId = 0;


ble_profile_t* BleProfiles_createProfile(void)
{
	ble_profile_t* np = (ble_profile_t*) malloc(sizeof(ble_profile_t));
	memset((void*)np, 0 , sizeof(ble_profile_t));
	np->mGatt_if = ESP_GATT_IF_NONE;
	np->mId = mProfileId++;
	np->mGatts_cb = (void*)0;
	np->mServiceList = uList_createList();
	return np;
}


void BleProfiles_addService(ble_profile_t *pf, ble_service_t *service)
{
	if(pf != 0 && service!= 0)
	{
		uNode_t *node = uList_createNode((void*)service, sizeof(ble_service_t), NODE_NON_SELF_ALLOC);
		uList_append(pf->mServiceList, node);
	}
}

/*
 * nHandle  there are three characteristic in a service, where the first characteristic has two descriptors,
 * and the second characteristic has only one descriptor, and the third characteristic has no descriptor.
 * Then the number of handles needed should be
 * 1 (svc attr) + 4 (1 char declaration attr, 1 char value attr, 2 descriptors attr) + 3 (1 char declaration attr, 1 char value attr, 1 Descriptor attr) + 2 (1 char declaration attr, 1 char value attr) = 10
 */

ble_service_t* BleProfiles_createService(uint8_t *uuid, uint8_t len, uint8_t nHandle, bool isPrimary)
{
	ble_service_t* ns = (ble_service_t*) malloc(sizeof(ble_service_t));
	memset((void*)ns, 0 , sizeof(ble_service_t));
	ns->mCharList = uList_createList();
	ns->mService_id = (esp_gatt_srvc_id_t*) malloc(sizeof(esp_gatt_srvc_id_t));
	memset((void*)ns->mService_id, 0 , sizeof(esp_gatt_srvc_id_t));
	memcpy((void*)&ns->mService_id->id.uuid.uuid,(void*)uuid, len);
	ns->mService_id->id.uuid.len = len;
	ns->mService_id->is_primary = isPrimary;
	ns->mNumHandle = nHandle;

	return ns;
}

ble_char_t* BleProfiles_createCharacteristic(uint8_t *uuid, uint8_t len, uint8_t autoRsp)
{
	ble_char_t* ch = (ble_char_t*) malloc(sizeof(ble_char_t));
	esp_bt_uuid_t* id = (esp_bt_uuid_t*) malloc(sizeof(esp_bt_uuid_t));
	memset((void*)ch, 0 , sizeof(ble_char_t));
	memset((void*)id, 0 , sizeof(esp_bt_uuid_t));
	ch->mDescrList = uList_createList();
	id->len = len;
	memcpy((void*)&id->uuid, (void*)uuid, len);
	ch->mChar_uuid = id;
	ch->mRsp.auto_rsp = autoRsp;
	ch->mReadEvent = NULL;
	ch->mWriteEvent = NULL;
	return ch;
}

void BleProfiles_setCharacteristicPermission(ble_char_t *ch, uint16_t pem)
{
	if(ch)
		ch->mPerm = pem;
}

void BleProfiles_setCharacteristicProperty(ble_char_t *ch, uint8_t property)
{
	if(ch)
		ch->mProperty = property;
}

void BleProfile_setCharacteristicValue(ble_char_t *ch, uint8_t *value, uint8_t len, uint8_t maxLen)
{
	if(ch)
	{
		esp_attr_value_t* at = (esp_attr_value_t*) malloc(sizeof(esp_attr_value_t));
		memset((void*)at, 0, sizeof(esp_attr_value_t));
		at->attr_len = len;
		at->attr_max_len = maxLen;
		at->attr_value = value;
		ch->mAtt = at;
	}
}

void BleProfiles_addCharacteristic(ble_service_t *service, ble_char_t *characteristic)
{
	if(service && characteristic)
	{
		uNode_t *node = uList_createNode((void*)characteristic, sizeof(ble_char_t), NODE_NON_SELF_ALLOC);
		uList_append(service->mCharList, node);
	}
}


ble_descrp_t* BleProfiles_createDescription(uint8_t *uuid, uint8_t uuidLen, uint8_t autoRsp)
{
	ble_descrp_t* ds = (ble_descrp_t*) malloc(sizeof(ble_descrp_t));
	esp_bt_uuid_t* id = (esp_bt_uuid_t*) malloc(sizeof(esp_bt_uuid_t));
	memset((void*)ds, 0, sizeof(ble_descrp_t));
	memset((void*)id, 0 , sizeof(esp_bt_uuid_t));
	id->len = uuidLen;
	memcpy((void*)&id->uuid, (void*)uuid, id->len);
	ds->mRsp.auto_rsp = autoRsp;
	ds->mDescr_uuid = id;
	return ds;
}

void BleProfiles_setDescriptionPermission(ble_descrp_t *ds, uint16_t pem)
{
	if(ds)
		ds->mPerm = pem;
}


void BleProfile_setDescriptionValue(ble_descrp_t *ds, uint8_t *value, uint8_t len, uint8_t maxLen)
{
	if(ds)
	{
		esp_attr_value_t* at = (esp_attr_value_t*) malloc(sizeof(esp_attr_value_t));
		memset((void*)at, 0, sizeof(esp_attr_value_t));
		at->attr_len = len;
		at->attr_max_len = maxLen;
		at->attr_value = value;
		ds->mAtt = at;
	}
}

void BleProfiles_addDescription(ble_char_t *charactersitic, ble_descrp_t *ds)
{
	if(charactersitic && ds)
		{
			uNode_t *node = uList_createNode((void*)ds, sizeof(ble_descrp_t), NODE_NON_SELF_ALLOC);
			uList_append(charactersitic->mDescrList, node);
		}
}

void BleProfiles_setReadCallback(ble_char_t *charactersitic, readEvent_t callback)
{
	if (charactersitic && callback)
	{
		charactersitic->mReadEvent = callback;
	}
	
}
