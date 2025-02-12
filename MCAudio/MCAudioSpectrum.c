
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


#define DATA_TYPE_INT16 1
#define DATA_TYPE_INT32 2
#define DATA_TYPE_FLOAT32 3




typedef struct {
    int nChannels;
    int dataType;
    size_t nDataPerChannel;
    void** channelsData;
} WaveDecomposition;


typedef struct {
    float* freqArr;
    float* magnitudeArr;
    size_t size;
} WaveFrequencyInfo;


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
int waveDecompose_f(float* buffer, WaveDecomposition** out, size_t bufferSize, int nChannels){
    if ((bufferSize / sizeof(float)) % nChannels != 0){
        return -4;
    }
    WaveDecomposition* decomposedData = (WaveDecomposition*) malloc(sizeof(WaveDecomposition));
    if (decomposedData == NULL) {return -1;}
    decomposedData->nDataPerChannel = (bufferSize / nChannels) / sizeof(float);
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

int fft_f(float* array, Complex* result, size_t bufferSize){
    if (array == NULL || result == NULL) return -1;
    fftwf_plan p = fftwf_plan_dft_r2c_1d(bufferSize, array, result, FFTW_ESTIMATE);
    if (p == NULL) return -1;
    fftwf_execute(p);
    fftwf_destroy_plan(p);
    return 0;
}

void fftToMagnitude(Complex* fftResult, float* magnitudes, size_t size){
    float real, img;
    for(int i=0; i<size; i++){
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


int soundFrequencyAnalysis(void* soundData, WaveFrequencyInfo** out, size_t soundDataSize, int dataType, int nChannels, int sampleRate){
    if (soundData == NULL) return -1;
    if (dataType != DATA_TYPE_FLOAT32) return -2;
    WaveDecomposition* waveDecomp;
    int fResult = waveDecompose_f((float*) soundData, &waveDecomp, soundDataSize, nChannels);
    float* freqArr = calloc(waveDecomp->nDataPerChannel, sizeof(float));
    float* magnitudeArr = calloc(waveDecomp->nDataPerChannel, sizeof(float));
    Complex* fftResult = calloc(waveDecomp->nDataPerChannel, sizeof(Complex));

    if (fResult < 0 || freqArr == NULL || magnitudeArr == NULL || fftResult == NULL){
        freeWaveDecomposeData(waveDecomp);
        FREE_IFN_NULL(freqArr);
        FREE_IFN_NULL(magnitudeArr);
        FREE_IFN_NULL(fftResult);
        return -3;
    }
    fResult = fft_f(waveDecomp->channelsData[0], fftResult, waveDecomp->nDataPerChannel);

    if (fResult < 0){
        freeWaveDecomposeData(waveDecomp);
        FREE_IFN_NULL(freqArr);
        FREE_IFN_NULL(magnitudeArr);
        FREE_IFN_NULL(fftResult);
        return -4;
    }


    fftToMagnitude(fftResult, magnitudeArr, waveDecomp->nDataPerChannel);
    makeFrequencyArray(freqArr, sampleRate, waveDecomp->nDataPerChannel);

    WaveFrequencyInfo* wfi = malloc(sizeof(WaveFrequencyInfo));
    if (wfi == NULL){
        freeWaveDecomposeData(waveDecomp);
        FREE_IFN_NULL(freqArr);
        FREE_IFN_NULL(magnitudeArr);
        FREE_IFN_NULL(fftResult);
        return -5;
    }
    wfi->freqArr = freqArr;
    wfi->magnitudeArr = magnitudeArr;
    wfi->size = waveDecomp->nDataPerChannel;
    freeWaveDecomposeData(waveDecomp);
    *out = wfi;
    return 0;
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



int fft(){
    int res = initializeDevice("{0.0.0.00000000}.{5cf1419a-62f1-4c5a-be55-ff7d9fc4d059}");
    if (res < 0){
        printf("Error: %d\n", res);
        return -1;
    }

    FILE* f;

    f = fopen("temp.txt", "w");
    if (f == NULL) return -1;
    fwrite("{\ndata:[", 1,8,f);

    for (int i=0; i<40; i++){
        int chunks =  1;
        size_t buffSize = getAudioBufferByteSize() * chunks;
        unsigned char* buff = (unsigned char*) malloc(buffSize);
        for (int i=0; i<chunks; i++){
            printf("Iteracja %d\n", i);
            int result = deviceListen(buff+i*getAudioBufferByteSize());
            if (result < 0){
                printf("Device listen error!\n");
            }
        }
        WaveFormat format;
        getWaveFormat(&format);
        WaveFrequencyInfo* wfi;
        
        res = soundFrequencyAnalysis(buff, &wfi, 1024, DATA_TYPE_FLOAT32, format.nChannels, format.nSamplesPerSec);
        char b[2048];
        
        char* b_ptr = b;
        int n = 0;
        for (size_t i = 0; i < wfi->size; i++) {
            int ws = snprintf(b_ptr, 2048, "%e, ", wfi->magnitudeArr[i]);
            b_ptr+=ws;
            b_ptr--;
            n+=ws;
        }
        fwrite("[", 1, 1, f);
        fwrite(b, 1, 1663, f);
        fwrite("],", 1, 2, f);
        freeWaveFrequencyInfo(wfi);
        free(buff);
    }

    fwrite("]}\n",1, 3, f);
    fclose(f);
}

int main() {
    fft();
}

// int main(){

//     int n = 100;
//     fftwf_plan p;
//     fftwf_complex *out = fftwf_alloc_complex(n);
//     float in[100] = {
//         0.90985076, 0.134142, 0.61993111, 0.0939338 , 0.59640361,
//         0.79318277, 0.19183375, 0.61241273, 0.02533106, 0.80835048,
//         0.12470453, 0.73039137, 0.12412828597, 0.98099162, 0.39473245,
//         0.08180225, 0.12221674, 0.76417496, 0.71899882, 0.83068519,
//         0.97309147, 0.7652887, 0.56147256, 0.60377868, 0.42426079,
//         0.05180212, 0.71993944, 0.6753221 , 0.55329159, 0.3232711 ,
//         0.1581098 , 0.76426762, 0.81960917, 0.49455127, 0.95793991,
//         0.73809582, 0.99665514, 0.26421325, 0.52227488, 0.10092167,
//         0.44316728, 0.08275197, 0.05547524, 0.36701586, 0.98150885,
//         0.51785537, 0.31162865, 0.59550327, 0.84606771, 0.37537358,
//         0.76737643, 0.76707433, 0.06286652, 0.29290648, 0.27186843,
//         0.14092702, 0.79065741, 0.44164677, 0.50871114, 0.56711263,
//         0.77228984, 0.63935094, 0.91818799, 0.64076596, 0.39048657,
//         0.32829046, 0.52065639, 0.05790728, 0.39947999, 0.93692312,
//         0.01497371, 0.86152561, 0.48068869, 0.38256099, 0.46894855,
//         0.52950206, 0.95479497, 0.600049  , 0.91032703, 0.25183341,
//         0.71886541, 0.15285566, 0.81538678, 0.52855503, 0.75983379,
//         0.87374054, 0.62199849, 0.42316   , 0.76808058, 0.30141869,
//         0.88763789, 0.39128329, 0.91220372, 0.75444232, 0.05311182,
//         0.94061116, 0.06743344, 0.51067587, 0.77308366, 0.2915374 
//     };

//     p = fftwf_plan_dft_r2c_1d(n, in, out, FFTW_ESTIMATE);
//     fftwf_execute(p);

//     for (int i = 0; i < n; i++) {
//         printf("tab[%d] = %e + i*%e\n", i, out[i][0], out[i][1]);
//     }

//     fftwf_destroy_plan(p);
//     fftwf_free(out);

//     return 0;
// }
