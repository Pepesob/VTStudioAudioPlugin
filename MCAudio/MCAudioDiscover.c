
#include "MCAudioDiscover.h"
#include "MCAudioInit.h"

#include <windows.h>
#include <setupapi.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <stdbool.h>
#include <stdio.h>


#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->lpVtbl->Release(punk); (punk) = NULL; }

#define EXIT_ON_ERROR(hres)  \
                if (FAILED(hres)) { goto ExitOnError; }

#define START_AVDEV_BUFFER_SIZE 2
#define MAX_DEVICE_STRING_LEN 100



typedef struct {
    size_t capacity;
    size_t size;
    DeviceInfo* buffer;
} AvDev;

AvDev availableDevices = {
    (size_t) -1,
    (size_t) -1,
    NULL
};



int initAvDev() {
    if (availableDevices.buffer != NULL) {
        for (size_t i=0; i<availableDevices.size; i++){
            free(availableDevices.buffer[i].id);
            free(availableDevices.buffer[i].name);
        }
        free(availableDevices.buffer);
        availableDevices.buffer = NULL;
    }
    availableDevices.buffer = (DeviceInfo*) calloc(START_AVDEV_BUFFER_SIZE, sizeof(DeviceInfo));
    if (availableDevices.buffer == NULL) {
        return -1;
    }

    availableDevices.capacity = START_AVDEV_BUFFER_SIZE;
    availableDevices.size = 0;

    return 0;
}

int addAvDev(LPWSTR id, LPWSTR name) {
    // expand buffer times 2 if capacity is not enough
    if (availableDevices.size == availableDevices.capacity){
        size_t newCapacity = availableDevices.capacity * 2;
        DeviceInfo* newBuff = (DeviceInfo*) calloc(newCapacity, sizeof(DeviceInfo));
        if (newBuff == NULL) return -1;
        memcpy(newBuff, availableDevices.buffer, sizeof(DeviceInfo)*availableDevices.capacity);
        free(availableDevices.buffer);
        availableDevices.capacity = newCapacity;
        availableDevices.buffer = newBuff;
    }
    size_t newIndex = availableDevices.size;
    availableDevices.buffer[newIndex].id = (char*) calloc(MAX_DEVICE_STRING_LEN, sizeof(char));
    availableDevices.buffer[newIndex].name = (char*) calloc(MAX_DEVICE_STRING_LEN, sizeof(char));
    WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, id, -1, availableDevices.buffer[newIndex].id, MAX_DEVICE_STRING_LEN, NULL, NULL);
    WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, name, -1, availableDevices.buffer[newIndex].name, MAX_DEVICE_STRING_LEN, NULL, NULL);
    availableDevices.size++;
    return 0;
}


int discoverDevices() {
    HRESULT fResult;
    IMMDeviceEnumerator* deviceEnum = NULL;
    IMMDeviceCollection* deviceCollection = NULL;
    IMMDevice* device = NULL;
    IPropertyStore* devicePropStore = NULL;
    
    LPWSTR devId = NULL;

    UINT deviceNum = 0;


    fResult = initComLib();
    EXIT_ON_ERROR(fResult);

    fResult = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void**)&deviceEnum);
    EXIT_ON_ERROR(fResult);

    fResult = deviceEnum->lpVtbl->EnumAudioEndpoints(deviceEnum, eRender, DEVICE_STATE_ACTIVE, &deviceCollection);
    EXIT_ON_ERROR(fResult);

    fResult = deviceCollection->lpVtbl->GetCount(deviceCollection, &deviceNum);
    EXIT_ON_ERROR(fResult);

    initAvDev();
    for (UINT i = 0; i < deviceNum; i++) {
        // if at any stage fResult is not 0, continue loop
        fResult = deviceCollection->lpVtbl->Item(deviceCollection, i, &device);
        EXIT_ON_ERROR(fResult);

        fResult = device->lpVtbl->OpenPropertyStore(device, STGM_READ, &devicePropStore);
        EXIT_ON_ERROR(fResult);

        fResult = device->lpVtbl->GetId(device, &devId);
        EXIT_ON_ERROR(fResult);


        PROPVARIANT prop;
        PropVariantInit(&prop);
        // NO CHECK WATCH OUT
        devicePropStore->lpVtbl->GetValue(devicePropStore, &PKEY_Device_FriendlyName, &prop);
        // NO CHECK WATCH OUT
        addAvDev(devId, prop.pwszVal);
        
        CoTaskMemFree(devId);
        devId = NULL;
        PropVariantClear(&prop);
        SAFE_RELEASE(device);
        SAFE_RELEASE(devicePropStore);
    }

    SAFE_RELEASE(deviceEnum);
    SAFE_RELEASE(deviceCollection);
    deinitComLib();
    return 0;

ExitOnError:
    initAvDev();
    CoTaskMemFree(devId);
    SAFE_RELEASE(deviceEnum);
    SAFE_RELEASE(deviceCollection);
    SAFE_RELEASE(device);
    SAFE_RELEASE(devicePropStore);
    deinitComLib();
    return -1;
}


int getAvailableDeviceCount() {
    return availableDevices.size;
}

// Do not use return value of this function after calling discoverDevices() again as 
// the buffer of availableDevices will be freed, copy any value needed and do not hold on to this structure
DeviceInfo* getDeviceInfo(int index){
    if (availableDevices.buffer == NULL) {
        return NULL;
    }
    if (0 < index && index > availableDevices.size) {
        return NULL;
    }
    return &availableDevices.buffer[index];
}



