package main

// #cgo CFLAGS: -I./MCAudio
// #cgo LDFLAGS: -L./MCAudio -lMCAudio -lole32 -lsetupapi -luuid
// #include "MCAudio/MCAudioDiscover.h"
import "C"

import "fmt"

type DeviceInfo struct {
	Id   string
	Name string
}

func DiscoverDevices() ([]DeviceInfo, error) {
	if C.discoverDevices() != 0 {
		return nil, fmt.Errorf("failed to discover devices")
	}

	// Get the count of available devices
	deviceCount := int(C.getAvailableDeviceCount())

	// Create a slice to store the device information
	devices := make([]DeviceInfo, deviceCount)

	// Iterate through each device and retrieve its information
	for i := 0; i < deviceCount; i++ {
		deviceInfo := C.getDeviceInfo(C.int(i))

		// Convert C strings to Go strings
		id := C.GoString(deviceInfo.id)
		name := C.GoString(deviceInfo.name)

		// Store the information in the Go slice
		devices[i] = DeviceInfo{
			Id:   id,
			Name: name,
		}
	}

	return devices, nil
}
