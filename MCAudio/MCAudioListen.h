#pragma once

#include "MCAudioWaveFormat.h"

#include <stdint.h>



int initializeDevice(const char* strId);
void releaseDevice();
int isDeviceInitialized();

size_t getAudioBufferByteSize();
WaveFormat* getWaveFormat();
WaveFormatExtensible* getWaveFormatExtensible();

int deviceListen(uint8_t* buffer);

