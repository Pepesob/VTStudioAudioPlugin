#include "MCAudioWaveFormat.h"

#include <stdio.h>


void printHex(const void *buffer, size_t size) {
    const uint8_t *byteData = (const uint8_t *)buffer;
    for (size_t i = 0; i < size; i++) {
        printf("%02X ", byteData[i]);
    }
    printf("\n"); 
}

void saveWav(const char *filename, unsigned char *buffer, uint32_t bufferSize, WaveFormat *waveFormat) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file");
        return;
    }
    
    // RIFF Header
    fwrite("RIFF", 1, 4, file);
    uint32_t chunkSize = 36 + bufferSize + (waveFormat->wFormatTag == 0xFFFE ? 24 : 0);
    fwrite(&chunkSize, sizeof(uint32_t), 1, file);
    fwrite("WAVE", 1, 4, file);
    
    // fmt chunk
    fwrite("fmt ", 1, 4, file);
    uint32_t subchunk1Size = (waveFormat->wFormatTag == 0xFFFE) ? 40 : 16;
    fwrite(&subchunk1Size, sizeof(uint32_t), 1, file);
    fwrite(waveFormat, sizeof(WaveFormat), 1, file);
    
    if (waveFormat->wFormatTag == 0xFFFE) {
        // Write WaveFormatExtensible extra fields
        WaveFormatExtensible *ext = (WaveFormatExtensible *)waveFormat;
        fwrite(&ext->Samples, sizeof(uint16_t), 1, file);
        fwrite(&ext->dwChannelMask, sizeof(uint32_t), 1, file);
        fwrite(ext->SubFormat, sizeof(uint8_t), 16, file);
    }
    
    // data chunk
    fwrite("data", 1, 4, file);
    fwrite(&bufferSize, sizeof(uint32_t), 1, file);
    fwrite(buffer, 1, bufferSize, file);
    
    fclose(file);
}

