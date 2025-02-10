#pragma once 
#include "MCAudioWaveFormat.h"


void saveWav(const char* filename, unsigned char* buffer, uint32_t bufferSize, WaveFormat* waveFormat);