#pragma once 

#include "MCAudioWaveFormat.h"

#include <stddef.h>


void saveWav(const char* filename, unsigned char* buffer, uint32_t bufferSize, WaveFormat* waveFormat);
int createWavHeader(uint8_t** headerBuffer, size_t* retHeaderSize, uint32_t dataSize, WaveFormat* waveFormat);
