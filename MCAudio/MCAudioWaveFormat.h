#pragma once

#include <stdint.h>

// Disable memory alignment
#pragma pack(push,1)
typedef struct {
    uint16_t wFormatTag;        // Format type
    uint16_t nChannels;         // Number of channels
    uint32_t nSamplesPerSec;    // Sample rate in Hz
    uint32_t nAvgBytesPerSec;   // Average bytes per second (nSamplesPerSec * nChannels * wBitsPerSample / 8)
    uint16_t nBlockAlign;       // Block size of data (nChannles * wBitsPerSample / 8)
    uint16_t wBitsPerSample;    // Number of bits per sample
    uint16_t cbSize;            // Size of extra information if WaveFormatExtensible supported (wFormatTag == oxFFFE)
} WaveFormat;

typedef struct {
    WaveFormat Format;     // Base structure
    union {
        uint16_t wValidBitsPerSample; // Bits of precision (used when non-PCM)
        uint16_t wSamplesPerBlock;
        uint16_t wReserved;
    } Samples;
    uint32_t dwChannelMask;  // Speaker position mask
    uint8_t SubFormat[16];   // GUID 
} WaveFormatExtensible;
#pragma pack(pop)