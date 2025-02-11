#include "MCAudioDiscover.h"
#include "MCAudioWaveFormat.h"
#include "MCAudioWaveSave.h"
#include "MCAudioListen.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <windows.h>


int testListen() {
    int res = initializeDevice("{0.0.0.00000000}.{6c661f6c-d8e3-4d63-b910-3683742085b4}");
    if (res < 0){
        printf("Error: %d\n", res);
        return -1;
    }


    int chunks =  20;
    unsigned char* buff = (unsigned char*) malloc(getAudioBufferByteSize() * chunks);
    for (int i=0; i<chunks; i++){
        printf("Iteracja %d\n", i);
        int result = deviceListen(buff+i*getAudioBufferByteSize());
        if (result < 0){
            printf("Device listen error!\n");
        }
    }

    WaveFormat simple;
    getWaveFormat(&simple);

    saveWav("mysound.wav", buff, getAudioBufferByteSize() * chunks, &simple);

    return 0;
}

int discoverDeviceTest() {
    int status = discoverDevices();

    for (int i=0; i<getAvailableDeviceCount(); i++){
        DeviceInfo* devInfo = getDeviceInfo(i);
        printf("%d Device id: %s, Device name: %s \n",status, devInfo->id, devInfo->name);
    }
}

int printWaveFormat() {
    WaveFormat format;
    getWaveFormat(&format);
    printf("WaveFormatExtensible Structure:\n");
    printf("  Format Tag: %u\n", format.wFormatTag);
    printf("  Channels: %u\n", format.nChannels);
    printf("  Sample Rate: %u Hz\n", format.nSamplesPerSec);
    printf("  Avg Bytes Per Sec: %u\n", format.nAvgBytesPerSec);
    printf("  Block Align: %u\n", format.nBlockAlign);
    printf("  Bits Per Sample: %u\n", format.wBitsPerSample);
    printf("  Extra Info Size: %u\n", format.cbSize);
    printf("  Channel Mask: 0x%X\n", format.dwChannelMask);
    printf("  SubFormat (GUID): ");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", format.subFormat[i]);
    }
    printf("\n");
}


int main() {
    discoverDeviceTest();
    initializeDevice("{0.0.0.00000000}.{6c661f6c-d8e3-4d63-b910-3683742085b4}");

    printWaveFormat();


    testListen();
    releaseDevice();

    printf("\n");
}
