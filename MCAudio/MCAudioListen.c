// Dedicated header
#include "MCAudioListen.h"
#include "MCAudioWaveFormat.h"
#include "MCAudioInit.h"

// Standard library
#include <stdbool.h>
#include <stdio.h>

// Windows API
#include <windows.h>
#include <setupapi.h>
#include <initguid.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <audioclient.h>

#define MAX_DEVICE_STRING_LEN 100


#define REFERENCE_TIME_UNIT_NS 100
#define REFERENCE_TIME_BUFFER_SEC 0.2
#define REFERENCE_TIME_BUFFER_SIZE (long long) (REFERENCE_TIME_BUFFER_SEC * 1000000000 / REFERENCE_TIME_UNIT_NS)
#define AUDIO_BUFFER_WAIT_DIVIDER 2


#define RETURN_ON_ERROR(RESULT, RCODE) if ((uint32_t) RESULT < 0) return RCODE
#define BREAK_ON_ERROR(RESULT) if ((uint32_t) RESULT < 0) break
#define RETURN_ON_NULL(PTR, RCODE) if (PTR == NULL) return RCODE

HRESULT fResult;
WCHAR* deviceId = NULL;
IMMDeviceEnumerator* deviceEnum = NULL;
IMMDevice* device = NULL;
IAudioClient* audioClient = NULL;
WAVEFORMATEX* audioFormat = NULL;
IAudioCaptureClient* audioCaptureClient = NULL;
uint32_t bufferFrameCount = -1;
uint8_t* audioBuffer = NULL;
size_t audioBufferByteSize = -1;
REFERENCE_TIME defaultDevicePeriod = -1;

bool deviceInitialized = false;


void releaseDevice() {
    if (deviceId != NULL){
        free((void*) deviceId);
    }
    if (audioBuffer != NULL){
        free((void*) audioBuffer);
    }
    if (deviceEnum != NULL){
        deviceEnum->lpVtbl->Release(deviceEnum);
    }
    if (device != NULL){
        device->lpVtbl->Release(device);
    }
    if (audioClient != NULL){
        audioClient->lpVtbl->Release(audioClient);
    }
    if (audioCaptureClient != NULL){
        audioCaptureClient->lpVtbl->Release(audioCaptureClient);
    }
    if (audioFormat != NULL){
        CoTaskMemFree(audioFormat);
    }
    audioBufferByteSize = -1;
    defaultDevicePeriod = -1;
    deinitComLib();
    deviceInitialized = false;
}


int initializeDevice(const char* strId) {
    releaseDevice();

    fResult = initComLib();
    RETURN_ON_ERROR(fResult, -10);

    deviceId = (WCHAR*) calloc(MAX_DEVICE_STRING_LEN, sizeof(WCHAR));
    RETURN_ON_NULL(deviceId, -1);
    mbstowcs(deviceId, strId, MAX_DEVICE_STRING_LEN);

    fResult = CoCreateInstance(&CLSID_MMDeviceEnumerator, NULL, CLSCTX_ALL, &IID_IMMDeviceEnumerator, (void**)&deviceEnum);
    RETURN_ON_ERROR(fResult, -2);

    fResult = deviceEnum->lpVtbl->GetDevice(deviceEnum, deviceId, &device);
    RETURN_ON_ERROR(fResult, -2);

    fResult = device->lpVtbl->Activate(device, &IID_IAudioClient, CLSCTX_ALL, NULL, (void**) &audioClient);
    RETURN_ON_ERROR(fResult, -3);

    fResult = audioClient->lpVtbl->GetMixFormat(audioClient, &audioFormat);
    RETURN_ON_ERROR(fResult, -4);

    fResult = audioClient->lpVtbl->Initialize(audioClient, AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, REFERENCE_TIME_BUFFER_SIZE, 0, audioFormat, NULL);
    RETURN_ON_ERROR(fResult, -5);

    fResult = audioClient->lpVtbl->GetService(audioClient, &IID_IAudioCaptureClient, (void **) &audioCaptureClient);
    RETURN_ON_ERROR(fResult, fResult);

    fResult = audioClient->lpVtbl->GetDevicePeriod(audioClient, &defaultDevicePeriod, NULL);
    RETURN_ON_ERROR(fResult, fResult);

    fResult = audioClient->lpVtbl->GetBufferSize(audioClient, &bufferFrameCount);

    audioBufferByteSize = REFERENCE_TIME_BUFFER_SEC * audioFormat->nAvgBytesPerSec;
    audioBuffer = malloc(audioBufferByteSize);

    deviceInitialized = true;
    return 0;
}

size_t getBufferSizeMs(){
    if (deviceInitialized){
        return bufferFrameCount * audioFormat->nBlockAlign * 1000 / audioFormat->nAvgBytesPerSec;
    }
    return -1;
}

size_t getWaitTimeMs() {
    if (deviceInitialized){
        return getBufferSizeMs() / AUDIO_BUFFER_WAIT_DIVIDER;
    }
    return -1;
}


void memcpy0(void* dest, void* src, size_t n){
    if (src == NULL) {
        memset(dest, 0, n);
    } else {
        memcpy(dest, src, n);
    }
}


size_t fillBuffer(uint8_t* buff, uint8_t* packet, size_t bufferIndex, size_t bufferSize, size_t packetSize){
    size_t bytesToCopy = (bufferIndex + packetSize <= bufferSize) ? packetSize : bufferSize - bufferIndex;
    if (packet == NULL){
        memset(buff+bufferIndex, 0, bytesToCopy);
    } else {
        memcpy(buff+bufferIndex, packet, bytesToCopy);
    }
    return bytesToCopy;
}

int deviceListen(uint8_t* buffer) {
    if (!deviceInitialized){
        return -1;
    }

    UINT32 framesToRead = 0;
    BYTE* data;
    DWORD flags;
    size_t bufferIndex = 0;

    fResult = audioClient->lpVtbl->Start(audioClient);
    RETURN_ON_ERROR(fResult, -1);

    // now it works, make something to write silence to a buffer and it will be g, make sleep for half the buffer size
    while (bufferIndex < audioBufferByteSize) {
        uint32_t nextPacketSize = 0;
        fResult = audioCaptureClient->lpVtbl->GetNextPacketSize(audioCaptureClient, &nextPacketSize);
        BREAK_ON_ERROR(fResult);
        if (nextPacketSize == 0){
            bufferIndex += fillBuffer(buffer, NULL, bufferIndex, audioBufferByteSize, getWaitTimeMs() * 1000 * audioFormat->nAvgBytesPerSec);
        }
        else {
            while (nextPacketSize != 0 && bufferIndex < audioBufferByteSize){
                fResult = audioCaptureClient->lpVtbl->GetBuffer(audioCaptureClient, &data, &framesToRead, &flags, NULL, NULL);
                BREAK_ON_ERROR(fResult);
                bufferIndex += fillBuffer(buffer, data, bufferIndex, audioBufferByteSize, framesToRead * audioFormat->nBlockAlign);
                fResult = audioCaptureClient->lpVtbl->ReleaseBuffer(audioCaptureClient, framesToRead);
                framesToRead = 0;
                BREAK_ON_ERROR(fResult);
                fResult = audioCaptureClient->lpVtbl->GetNextPacketSize(audioCaptureClient, &nextPacketSize);
                BREAK_ON_ERROR(fResult);
            }
        }
        Sleep(getWaitTimeMs());
    }

    audioClient->lpVtbl->Stop(audioClient);
    if (fResult < 0){
        audioCaptureClient->lpVtbl->ReleaseBuffer(audioCaptureClient, framesToRead);
        return -2;
    }
    return 0;
}


WaveFormat* getWaveFormat() {
    if (deviceInitialized){
        return (WaveFormat*) audioFormat;
    }
    return NULL;
}

WaveFormatExtensible* getWaveFormatExtensible() {
    if (deviceInitialized && audioFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE){
        return (WaveFormatExtensible*) audioFormat;
    }
    return NULL;
}

size_t getAudioBufferByteSize() {
    return audioBufferByteSize;
}

bool isDeviceInitialized() {
    return deviceInitialized;
}

