#include "MCAudioDiscover.h"
#include "MCAudioWaveFormat.h"
#include "MCAudioWaveSave.h"
#include "MCAudioListen.h"

#include <stdio.h>
#include <stdlib.h>



int testListen() {
    int res = initializeDevice("{0.0.0.00000000}.{6c661f6c-d8e3-4d63-b910-3683742085b4}");
    if (res < 0){
        printf("Error: %d\n", res);
        return -1;
    }


    int chunks =  100;
    unsigned char* buff = (unsigned char*) malloc(getAudioBufferByteSize() * chunks);
    for (int i=0; i<chunks; i++){
        printf("Iteracja %d\n", i);
        int result = deviceListen(buff+i*getAudioBufferByteSize());
        if (result < 0){
            printf("Device listen error!\n");
        }
    }

    saveWav("mysound.wav", buff, getAudioBufferByteSize() * chunks, (WaveFormat*) getWaveFormatExtensible());

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
    WaveFormatExtensible* wfExt = getWaveFormatExtensible();
    printf("WaveFormatExtensible Structure:\n");
    printf("  Format Tag: %u\n", wfExt->Format.wFormatTag);
    printf("  Channels: %u\n", wfExt->Format.nChannels);
    printf("  Sample Rate: %u Hz\n", wfExt->Format.nSamplesPerSec);
    printf("  Avg Bytes Per Sec: %u\n", wfExt->Format.nAvgBytesPerSec);
    printf("  Block Align: %u\n", wfExt->Format.nBlockAlign);
    printf("  Bits Per Sample: %u\n", wfExt->Format.wBitsPerSample);
    printf("  Extra Info Size: %u\n", wfExt->Format.cbSize);

    // Depending on the format, print the appropriate sample data
    if (wfExt->Format.wFormatTag == 0xFFFE) {
        // WaveFormatExtensible (non-PCM format)
        printf("  Valid Bits Per Sample: %u\n", wfExt->Samples.wValidBitsPerSample);
    } else {
        // PCM or other formats
        printf("  Samples Per Block: %u\n", wfExt->Samples.wSamplesPerBlock);
    }

    printf("  Channel Mask: 0x%X\n", wfExt->dwChannelMask);
    printf("  SubFormat (GUID): ");
    for (int i = 0; i < 16; i++) {
        printf("%02X ", wfExt->SubFormat[i]);
    }
    printf("\n");
}


int main() {
    discoverDeviceTest();
    initializeDevice("{0.0.0.00000000}.{6c661f6c-d8e3-4d63-b910-3683742085b4}");

    printWaveFormat();
    releaseDevice();
}
