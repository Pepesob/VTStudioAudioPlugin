#include "MCAudioWaveFormat.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


void printHex(const void *buffer, size_t size) {
    const uint8_t *byteData = (const uint8_t *)buffer;
    for (size_t i = 0; i < size; i++) {
        printf("%02X ", byteData[i]);
    }
    printf("\n"); 
}

// This function allocates memory and writes address to headerBuffer, it is users responsibility to
// free this memory after it is no longer needed
int createWavHeader(uint8_t** headerBuffer, size_t* retHeaderSize, uint32_t dataSize, WaveFormat* waveFormat){
    if (headerBuffer == NULL || retHeaderSize  == NULL || waveFormat  == NULL) {
        return -1;
    }

    size_t baseHeaderSize = 44; // PCM format header size
    
    size_t extraSize = (waveFormat->wFormatTag == 0xFFFE) ? 24 : 0; // 0xFFFE info
    size_t dataFormatSize = 16 + extraSize;
    size_t headerSize = baseHeaderSize + extraSize;

    size_t totalSize = baseHeaderSize + extraSize + dataSize; // RIFF header + fmt chunk

    *headerBuffer = (uint8_t*)malloc(totalSize);
    if (!*headerBuffer) {
        return -1;
    }
    *retHeaderSize = headerSize;

    uint8_t* ptr = *headerBuffer;

    // RIFF Header
    memcpy(ptr, "RIFF", 4); ptr += 4;
    uint32_t fileSizeMinus8 = totalSize - 8;
    memcpy(ptr, &fileSizeMinus8, 4); ptr += 4;
    memcpy(ptr, "WAVE", 4); ptr += 4;

    // fmt Chunk
    memcpy(ptr, "fmt ", 4); ptr += 4;
    memcpy(ptr, &dataFormatSize, 4); ptr += 4;

    memcpy(ptr, waveFormat, 16); ptr += 16;
    
    if (waveFormat->wFormatTag == 0xFFFE) {
        memcpy(ptr, ((uint8_t*)waveFormat) + 16, 24); ptr += 24;
    }

    memcpy(ptr, "data", 4); ptr += 4;
    memcpy(ptr, &dataSize, 4); ptr += 4;

    return 0;
}


void saveWav(const char *filename, unsigned char *buffer, uint32_t bufferSize, WaveFormat *waveFormat) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Failed to open file");
        return;
    }

    size_t headerSize;
    uint8_t* headerBuffer;

    createWavHeader(&headerBuffer, &headerSize, bufferSize, waveFormat);
    fwrite(headerBuffer, 1, headerSize, file);
    fwrite(buffer, 1, bufferSize, file);
    free((void*)headerBuffer);
    
    fclose(file);
}



