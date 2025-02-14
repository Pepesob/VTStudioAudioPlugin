
#include "MCAudioWaveFormat.h"

#define DATA_TYPE_INT16 1
#define DATA_TYPE_INT32 2
#define DATA_TYPE_FLOAT32 3

typedef float Complex[2];
typedef struct {
    float* freqArr;
    float* magnitudeArr;
    size_t size;
} WaveFrequencyInfo;

// int fft(void* buffer, size_t bufferSize, WaveFormat* waveInfo);
int soundFrequencyAnalysis(void* soundData, WaveFrequencyInfo** out, size_t soundDataSizeB, int dataType, int nChannels, int sampleRate);

void freeWaveFrequencyInfo(WaveFrequencyInfo* ptr);