#pragma once

typedef struct {
	char* id;
	char* name;
} DeviceInfo;

int discoverDevices();
int getAvailableDeviceCount();
DeviceInfo* getDeviceInfo(int index);
