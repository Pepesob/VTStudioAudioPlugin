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
#include <audiopolicy.h>

#define MAX_DEVICE_STRING_LEN 100

#define REFERENCE_TIME_UNIT_NS 100

#define S_TO_NS_SCALAR 1000000000
#define MS_TO_NS_SCALAR 1000000


#define RETURN_ON_ERROR(RESULT, RCODE) if (RESULT < 0) return RCODE
#define BREAK_ON_ERROR(RESULT) if (RESULT < 0) break
#define RETURN_ON_NULL(PTR, RCODE) if (PTR == NULL) return RCODE
#define GOTO_ON_ERROR(GOTOID, RESULT, RCODE) if (RESULT < 0) {errCode = RCODE; goto GOTOID;}

HRESULT fResult;
WCHAR* deviceId = NULL;
IMMDeviceEnumerator* deviceEnum = NULL;
IMMDevice* device = NULL;
IAudioClient* audioClient = NULL;
WAVEFORMATEX* audioFormat = NULL;
IAudioCaptureClient* audioCaptureClient = NULL;
uint32_t bufferFrameCount = -1;
uint8_t* audioBuffer = NULL;
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
    defaultDevicePeriod = -1;
    bufferFrameCount = -1;
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

    fResult = audioClient->lpVtbl->Initialize(audioClient, AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, audioFormat, NULL);
    RETURN_ON_ERROR(fResult, -5);

    fResult = audioClient->lpVtbl->GetService(audioClient, &IID_IAudioCaptureClient, (void **) &audioCaptureClient);
    RETURN_ON_ERROR(fResult, fResult);

    fResult = audioClient->lpVtbl->GetDevicePeriod(audioClient, &defaultDevicePeriod, NULL);
    RETURN_ON_ERROR(fResult, fResult);

    fResult = audioClient->lpVtbl->GetBufferSize(audioClient, &bufferFrameCount);

    deviceInitialized = true;
    return 0;
}

size_t getMinPacketSizeMS() {
    if (deviceInitialized){
        return defaultDevicePeriod * REFERENCE_TIME_UNIT_NS / MS_TO_NS_SCALAR;
    }
    return -1;
}

size_t getMinPacketSizeB() {
    if (deviceInitialized){
        return defaultDevicePeriod * REFERENCE_TIME_UNIT_NS * audioFormat->nSamplesPerSec * audioFormat->nBlockAlign / S_TO_NS_SCALAR;
    }
    return -1;
}


int getnActiveSessions(){
    // int sessionCount;
    // IAudioSessionControl* ctrl;
    // HRESULT res = audioSessionEnumerator->lpVtbl->GetCount(audioSessionEnumerator, &sessionCount);
    // printf("-----------------------------\n");
    // for (int i=0; i<sessionCount; i++){
    //     LPWSTR name;
    //     AudioSessionState state;
    //     audioSessionEnumerator->lpVtbl->GetSession(audioSessionEnumerator, i, &ctrl);
    //     ctrl->lpVtbl->GetDisplayName(ctrl, name);
    //     ctrl->lpVtbl->GetState(ctrl, &state);
    //     printf("Id: %d,   Device name: %ls,      Device state: %d\n", i, name, state);
    // }

    // LPWSTR name;
    // AudioSessionState state;
    // audioSessionControl->lpVtbl->GetDisplayName(audioSessionControl, &name);
    
    // audioSessionControl->lpVtbl->GetState(audioSessionControl, &state);
    // printf("Device name: %ls   State: %d\n", name, state);

    // float level;
    // int mute;
    // simpleAudioVolume->lpVtbl->GetMasterVolume(simpleAudioVolume, &level);
    // simpleAudioVolume->lpVtbl->GetMute(simpleAudioVolume, &mute);

    // printf("Audio master volume: %f,            Is mute: %d\n", level, mute);

    // uint32_t padding;
    // REFERENCE_TIME latency;
    // audioClient->lpVtbl->GetCurrentPadding(audioClient, &padding);
    // audioClient->lpVtbl->GetStreamLatency(audioClient, &latency);
    // LARGE_INTEGER time_prev;
    // LARGE_INTEGER time_after;


    // NtQuerySystemTime(&time_prev);


    // printf("Padding %u\n", padding);

    // printf("Default device period: %lld,             In ms: %lld,               Latency: %lld\n", defaultDevicePeriod,  defaultDevicePeriod * 100 / 1000000, latency);

    // NtQuerySystemTime(&time_after);

    // printf("printf duration:  %lld\n", (time_after.QuadPart - time_prev.QuadPart)*100 / 1000000);

    return 1;
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

int deviceListen(uint8_t* buffer, size_t sizeB) {
    if (!deviceInitialized){
        return -1;
    }
    // // Trunkate the amout of bytes to write to the integer block size
    // if (sizeB % audioFormat->nBlockAlign != 0){
    //     sizeB -= sizeB % audioFormat->nBlockAlign;
    // }

    int errCode = 0;

    UINT32 framesToRead = 0;
    BYTE* data;
    DWORD flags;
    size_t bufferIndex = 0;

    fResult = audioClient->lpVtbl->Start(audioClient);
    GOTO_ON_ERROR(device_listen_error, fResult, -5);

    // now it works, make something to write silence to a buffer and it will be g, make sleep for half the buffer size
    while (bufferIndex < sizeB) {
        Sleep(getMinPacketSizeMS());
        uint32_t nextPacketSize = 0;
        fResult = audioCaptureClient->lpVtbl->GetNextPacketSize(audioCaptureClient, &nextPacketSize);
        GOTO_ON_ERROR(device_listen_error, fResult, -2);
        if (nextPacketSize == 0){
            bufferIndex += fillBuffer(buffer, NULL, bufferIndex, sizeB, getMinPacketSizeB());
            // printf("Min packet size in bytes:%d,  Buff size: %lu,    BufferIndex: %lu\n", getMinPacketSizeB(), sizeB, bufferIndex);
        }
        else {
            while (nextPacketSize != 0 && bufferIndex < sizeB){
                fResult = audioCaptureClient->lpVtbl->GetBuffer(audioCaptureClient, &data, &framesToRead, &flags, NULL, NULL);
                GOTO_ON_ERROR(device_listen_error, fResult, -3);
                bufferIndex += fillBuffer(buffer, data, bufferIndex, sizeB, framesToRead * audioFormat->nBlockAlign);
                fResult = audioCaptureClient->lpVtbl->ReleaseBuffer(audioCaptureClient, framesToRead);
                GOTO_ON_ERROR(device_listen_error, fResult, -4);
                fResult = audioCaptureClient->lpVtbl->GetNextPacketSize(audioCaptureClient, &nextPacketSize);
                GOTO_ON_ERROR(device_listen_error, fResult, -5);
            }
        }
    }

    audioClient->lpVtbl->Stop(audioClient);
    // audioClient->lpVtbl->Reset(audioClient);
    return 0;

device_listen_error:
    audioCaptureClient->lpVtbl->ReleaseBuffer(audioCaptureClient, 0);
    audioClient->lpVtbl->Stop(audioClient);
    audioClient->lpVtbl->Reset(audioClient);
    return errCode;
}

// User is responsible to free buffer after it is no longer needed
int deviceListenB(size_t bytes, uint8_t** buffer){
    *buffer = NULL;
    size_t buffSizeB = bytes - (bytes % audioFormat->nBlockAlign);
    if (buffSizeB <= audioFormat->nBlockAlign) return -2;
    uint8_t* oBuff = (uint8_t*) malloc(bytes);
    RETURN_ON_NULL(oBuff, -3);
    int result = deviceListen(oBuff, buffSizeB);
    if (result < 0) {
        free(oBuff);
        return -4;
    }
    memset(oBuff+buffSizeB, 0, (bytes % audioFormat->nBlockAlign));
    *buffer = oBuff;
    return 0;
}

// User is responsible to free buffer after it is no longer needed
int deviceListenMs(size_t timeMs, uint8_t** buffer, size_t* sizeResult) {
    size_t buffSizeB =  timeMs * audioFormat->nAvgBytesPerSec / 1000;
    *sizeResult = buffSizeB;
    return deviceListenB(buffSizeB, buffer);
}


int getWaveFormat(WaveFormat* waveFormat) {
    if (!deviceInitialized){
        return -1;
    }
    if (audioFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE){
        memcpy(waveFormat, audioFormat, sizeof(WaveFormat));
    }
    else {
        memcpy(waveFormat, audioFormat, sizeof(WaveFormat) - 22);
        memset(((uint8_t*)waveFormat) + sizeof(WaveFormat) - 22, 0, 22);
    }
    return 0;
}

bool isDeviceInitialized() {
    return deviceInitialized;
}

