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
    int res = initializeDevice("{0.0.0.00000000}.{2adae94c-0d5d-4585-bfe6-6bcaf7421347}");
    if (res < 0){
        printf("Error: %d\n", res);
        return -1;
    }

    uint8_t* buff;
    size_t buffSize;
    deviceListenB(1000000, &buff);

    WaveFormat simple;
    getWaveFormat(&simple);

    saveWav("mysound.wav", buff, buffSize, &simple);

    free(buff);

    return 0;
}

int testFFT(){
    int res = initializeDevice("{0.0.0.00000000}.{2adae94c-0d5d-4585-bfe6-6bcaf7421347}");
    if (res < 0){
        printf("Error: %d\n", res);
        return -1;
    }
    WaveFormat format;
    getWaveFormat(&format);
    WaveFrequencyInfo* wfi;

    for(int j=0; j<100; j++){
        uint8_t* buff;
        deviceListenB(128, &buff);
        soundFrequencyAnalysis(buff, &wfi, 128, DATA_TYPE_FLOAT32, format.nChannels, format.nSamplesPerSec);

        for (size_t i = 0; i < wfi->size; i++) {
            printf("%e, ", wfi->magnitudeArr[i]);
        }
        printf("\n");
        freeWaveFrequencyInfo(wfi);
    }

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
    printf("Test\n");
    // discoverDeviceTest();
    
    // int res = initializeDevice("{0.0.0.00000000}.{2adae94c-0d5d-4585-bfe6-6bcaf7421347}");

    // printf("Result: %d\n", res);

    // printf("Num active sessions: %d\n", getnActiveSessions());

    // printWaveFormat();

    // testListen();
    // releaseDevice();
    testFFT();
}
