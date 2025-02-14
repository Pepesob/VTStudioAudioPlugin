#pragma once

#include "MCAudioWaveFormat.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>



int initializeDevice(const char* strId);
void releaseDevice();
bool isDeviceInitialized();
int getnActiveSessions();

size_t getAudioBufferByteSize();
int getWaveFormat(WaveFormat* waveFormat);

int deviceListen(uint8_t* buffer);

