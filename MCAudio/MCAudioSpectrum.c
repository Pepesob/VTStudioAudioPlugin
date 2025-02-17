
// Dedicated header

#include "MCAudioSpectrum.h"
#include "MCAudioListen.h"


// Standard library
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>


// Windows API

// FFTW
#include "../fftw-3.3.5-dll64/fftw3.h"

#define FREE_IFN_NULL(PTR) if (PTR != NULL) free((void*) PTR)
#define GOTO_ON_ERROR(GOTOID, RESULT, RCODE) if (RESULT < 0) {errCode = RCODE; goto GOTOID;}
#define GOTO_ON_NULL(GOTOID, PTR, RCODE) if (PTR==NULL) {errCode = RCODE; goto GOTOID;}


typedef struct {
    int nChannels;
    int dataType;
    size_t nDataPerChannel;
    void** channelsData;
} WaveDecomposition;


void freeWaveDecomposeData(WaveDecomposition* data){
    if (data == NULL) return;
    for (int i=0; i<data->nChannels; i++){
        FREE_IFN_NULL(data->channelsData[i]);
    }
    FREE_IFN_NULL(data->channelsData);
    FREE_IFN_NULL(data);
}

void freeWaveFrequencyInfo(WaveFrequencyInfo* ptr){
    if (ptr == NULL) return;
    FREE_IFN_NULL(ptr->freqArr);
    FREE_IFN_NULL(ptr->magnitudeArr);
    FREE_IFN_NULL(ptr);
}

// BUFFER SIZE IN BYTES! THINK OF A CONVENTION FOR THIS
int waveDecompose_f(float* buffer, WaveDecomposition** out, size_t bufferSizeB, int nChannels){
    if ((bufferSizeB / sizeof(float)) % nChannels != 0){
        return -4;
    }
    WaveDecomposition* decomposedData = (WaveDecomposition*) malloc(sizeof(WaveDecomposition));
    if (decomposedData == NULL) {return -1;}
    decomposedData->nDataPerChannel = (bufferSizeB / nChannels) / sizeof(float);
    decomposedData->nChannels = nChannels;
    decomposedData->dataType = DATA_TYPE_FLOAT32;
    decomposedData->channelsData = calloc(nChannels, sizeof(void*));
    if (decomposedData->channelsData == NULL) {
        freeWaveDecomposeData(decomposedData); 
        return -2;
    };
    
    for(int i=0; i<nChannels; i++){
        decomposedData->channelsData[i] = calloc(decomposedData->nDataPerChannel, sizeof(float));
        if (decomposedData->channelsData[i] == NULL) {
            freeWaveDecomposeData(decomposedData); 
            return -3;
        };
    }
    
    for (int i=0; i<decomposedData->nDataPerChannel; i++){
        for (int j=0; j<nChannels; j++){
            ((float**)decomposedData->channelsData)[j][i] = buffer[nChannels*i + j];
        }
    }
    *out = decomposedData;

    return 0;
}

int fft_f(float* array, Complex* result, size_t nElements){
    if (array == NULL || result == NULL) return -1;
    fftwf_plan p = fftwf_plan_dft_r2c_1d(nElements, array, result, FFTW_ESTIMATE);
    if (p == NULL) return -1;
    fftwf_execute(p);
    fftwf_destroy_plan(p);
    return 0;
}

void fftToMagnitude(Complex* fftResult, float* magnitudes, size_t nElements){
    float real, img;
    for(int i=0; i<nElements; i++){
        real = fftResult[i][0];
        img = fftResult[i][1];
        magnitudes[i] = sqrtf((real*real) + (img*img));
    }
}

void makeFrequencyArray(float* freqArr, size_t sampleRate, size_t nSamples){
    float df = sampleRate / nSamples;
    for(int i=0; i<nSamples; i++){
        freqArr[i] = i * df;
    }
}


int soundFrequencyAnalysis(void* soundData, WaveFrequencyInfo** out, size_t soundDataSizeB, int dataType, int nChannels, int sampleRate){
    if (soundData == NULL) return -1;
    if (dataType != DATA_TYPE_FLOAT32) return -2;

    int errCode = 0;
    WaveDecomposition* waveDecomp;
    int fResult = waveDecompose_f((float*) soundData, &waveDecomp, soundDataSizeB, nChannels);
    GOTO_ON_ERROR(sound_frequency_analysis_error, fResult, -3);

    float* freqArr = calloc(waveDecomp->nDataPerChannel, sizeof(float));
    GOTO_ON_NULL(sound_frequency_analysis_error, freqArr, -4);

    float* magnitudeArr = calloc(waveDecomp->nDataPerChannel, sizeof(float));
    GOTO_ON_NULL(sound_frequency_analysis_error, magnitudeArr, -5);

    Complex* fftResult = calloc(waveDecomp->nDataPerChannel, sizeof(Complex));
    GOTO_ON_NULL(sound_frequency_analysis_error, fftResult, -6);

    fResult = fft_f(waveDecomp->channelsData[0], fftResult, waveDecomp->nDataPerChannel);
    GOTO_ON_ERROR(sound_frequency_analysis_error, fResult, -7);

    fftToMagnitude(fftResult, magnitudeArr, waveDecomp->nDataPerChannel);
    makeFrequencyArray(freqArr, sampleRate, waveDecomp->nDataPerChannel);

    WaveFrequencyInfo* wfi = malloc(sizeof(WaveFrequencyInfo));
    GOTO_ON_NULL(sound_frequency_analysis_error, wfi, -6);


    wfi->freqArr = freqArr;
    wfi->magnitudeArr = magnitudeArr;
    wfi->size = waveDecomp->nDataPerChannel;

    *out = wfi;
    freeWaveDecomposeData(waveDecomp);
    return 0;

    // TODO - consider just free without checking for NULL
sound_frequency_analysis_error:
    freeWaveDecomposeData(waveDecomp);
    FREE_IFN_NULL(freqArr);
    FREE_IFN_NULL(magnitudeArr);
    FREE_IFN_NULL(fftResult);
    return errCode;
}

void printWaveFrequencyInfo(const WaveFrequencyInfo* info) {
    if (!info || !info->freqArr || !info->magnitudeArr) {
        printf("Invalid WaveFrequencyInfo structure.\n");
        return;
    }

    printf("WaveFrequencyInfo (Size: %u)\n", info->size);
    printf("    Frequency (Hz)   Magnitude\n");
    printf("----------------------------\n");

    for (size_t i = 0; i < info->size; i++) {
        printf("%d:  %e    %e\n",i, info->freqArr[i], info->magnitudeArr[i]);
    }

    for (size_t i = 0; i < info->size; i++) {
        printf("%e, ", info->magnitudeArr[i]);
    }

    printf("\n\n");

    for (size_t i = 0; i < info->size; i++) {
        printf("%e, ", info->freqArr[i]);
    }
}
