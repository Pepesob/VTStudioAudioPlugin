#include "MCAudioDiscover.h"
#include "MCAudioWaveFormat.h"
#include "MCAudioWaveSave.h"
#include "MCAudioListen.h"
#include "MCAudioSpectrum.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <windows.h>
#include <mmreg.h>
#include <guiddef.h>



int testListen() {
    int res = initializeDevice("{0.0.0.00000000}.{5cf1419a-62f1-4c5a-be55-ff7d9fc4d059}");
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

int testFFT(){
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
    printf("  Valid bits per sample: %u\n", format.wBitsPerSample);
    printf("  Channel Mask: 0x%X\n", format.dwChannelMask);
    printf("  SubFormat (GUID): ");
    printf("SubFormat GUID: {%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}\n",
        format.subFormat[3], format.subFormat[2], format.subFormat[1], format.subFormat[0],  // Data1 (Little-endian)
        format.subFormat[5], format.subFormat[4],                              // Data2 (Little-endian)
        format.subFormat[7], format.subFormat[6],                              // Data3 (Little-endian)
        format.subFormat[8], format.subFormat[9],                              // Data4 (Big-endian)
        format.subFormat[10], format.subFormat[11], format.subFormat[12], format.subFormat[13], format.subFormat[14], format.subFormat[15] // Data4 (Big-endian)
    );
    printf("\n");
}


int main() {
    discoverDeviceTest();
    int res = initializeDevice("{0.0.0.00000000}.{966b203e-3c32-4dd1-bb0a-bef2da7ed85a}");


    printWaveFormat();

    testListen();
    releaseDevice();

    printf("\n");
}
