#pragma once

#include "MCAudioWaveFormat.h"

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>



int initializeDevice(const char* strId);
void releaseDevice();
bool isDeviceInitialized();
int getnActiveSessions();

int getWaveFormat(WaveFormat* waveFormat);

int deviceListen(uint8_t* buffer, size_t sizeB);
int deviceListenMs(size_t timeMs, uint8_t** buffer, size_t* sizeResult);
int deviceListenB(size_t bytes, uint8_t** buffer);

