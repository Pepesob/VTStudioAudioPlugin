#include "MCAudioInit.h"

#include <stdbool.h>

#include <windows.h>
#include <objbase.h>

bool comLibInitialized = false;

int initComLib() {
    if (comLibInitialized) {
        return 0;
    }
    if (CoInitialize(NULL) == S_OK) {
        comLibInitialized = true;
        return 0;
    }
    return -1;
}

void deinitComLib() {
    if (comLibInitialized) {
        CoUninitialize();
        comLibInitialized = false;
    }
}

bool isComLibInitialized() {
    return comLibInitialized;
}