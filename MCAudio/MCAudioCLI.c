
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include "MCAudioCLI.h"
#include "MCAudioDiscover.h"
#include "MCAudioListen.h"
#include "MCAudioWaveSave.h"

#define MAX_ARG_KEYS 10
#define MAX_INPUT 100

int command_devicedisc() {
    int retVal = discoverDevices();
    if (retVal < 0) {
        printf("Error while discovering devices: %d\n", retVal);
        return -1;
    }
    int n_dev = getAvailableDeviceCount();
    if (n_dev == -1) {
        printf("Unknown error!\n");
        return -1;
    }
    printf("Available devices:\n");
    printf("-------------------------\n");
    for (int i=0; i<n_dev; i++){
        DeviceInfo* devInfo = getDeviceInfo(i);
        printf("%d.\nDevice name: %s\nDevice id: %s\n", i, devInfo->name, devInfo->id);
        printf("-------------------------\n");
    }
    return 0;
}

int command_deviceset(char* device_id){
    int retVal = initializeDevice(device_id);
    if (retVal < 0){
        printf("Failed to initialize device (probably wrong id)\n");
        return -1;
    }
    printf("Succesfully set device\n");

    return 0;
}

int command_devicelisten(char* st, char* filePath){
    int t = atoi(st);
    if (t == 0){
        printf("Invalid listen duration\n");
        return -1;
    }
    uint8_t* buffer;
    size_t buffer_size;
    printf("Listening...\n");
    int retVal = deviceListenMs(t * 1000, &buffer, &buffer_size);
    if (retVal < 0){
        printf("Error while listening to device\n");
        return -1;
    }
    WaveFormat wf;
    retVal = getWaveFormat(&wf);
    if (retVal < 0){
        printf("Error with getting wave format\n");
        free(buffer);
        return -1;
    }
    saveWav(filePath, buffer, buffer_size, &wf);
    free(buffer);
    printf("Done, file saved\n");
    return 0;
}


void mcaudio_cli() {
    char input[MAX_INPUT];

    char* arg_keys[MAX_ARG_KEYS];
    int n_arg_keys = 0;

    while (true){
        n_arg_keys = 0;
        arg_keys[0] = NULL;

        fgets(input, MAX_INPUT, stdin);
        input[strcspn(input, "\n")] = '\0';
        
        char* key = strtok(input, " ");
        arg_keys[n_arg_keys] = key;
        n_arg_keys++;
        while (((key = strtok(NULL, " ")) != NULL) && n_arg_keys < MAX_ARG_KEYS){
            arg_keys[n_arg_keys] = key;
            n_arg_keys++;
        }

        if (n_arg_keys == 0 || arg_keys[0] == NULL) {
            // printf("");
        }
        else if(strcmp("quit", arg_keys[0]) == 0) {
            break;
        }
        else if (strcmp("devicedisc", arg_keys[0]) == 0) {
            command_devicedisc();
        } 
        else if (strcmp("deviceset", arg_keys[0]) == 0){
            if (n_arg_keys < 1) {
                printf("Not enough arguments\n");
            } else {
                command_deviceset(arg_keys[1]);
            }
        }
        else if (strcmp("devicelisten", arg_keys[0]) == 0){
            if (n_arg_keys < 1) {
                printf("Not enough arguments\n");
            }
            else {
                command_devicelisten(arg_keys[1], arg_keys[2]);
            }
        }
        else {
            printf("Command not found\n");
        }
    }
}

