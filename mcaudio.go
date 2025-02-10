package main

// #cgo CFLAGS: -I./MCAudio
// #cgo LDFLAGS: -L./MCAudio -lMCAudio -lole32 -lsetupapi -luuid
// #include "MCAudio/MCAudioDiscover.h"
// #include "MCAudio/MCAudioListen.h"
// #include "MCAudio/MCAudioWaveFormat.h"
// #include <stdlib.h>
import "C"

import (
	"fmt"
	"unsafe"
)

type DeviceInfo struct {
	Id   string
	Name string
}

type WaveFormat struct {
	wFormatTag      uint16 // Format type
	nChannels       uint16 // Number of channels
	nSamplesPerSec  uint32 // Sample rate in Hz
	nAvgBytesPerSec uint32 // Average bytes per second (nSamplesPerSec * nChannels * wBitsPerSample / 8)
	nBlockAlign     uint16 // Block size of data (nChannles * wBitsPerSample / 8)
	wBitsPerSample  uint16 // Number of bits per sample
	cbSize          uint16 // Size of extra information if WaveFormatExtensible supported (wFormatTag == oxFFFE)
	samples         uint16
	dwChannelMask   uint16    // Speaker position mask
	SubFormat       [16]uint8 // GUID
}

func InitializeDevice(deviceId string) error {
	cDeviceId := C.CString(deviceId)
	if cDeviceId == nil {
		return fmt.Errorf("CString creation error")
	}
	defer C.free(unsafe.Pointer(cDeviceId))

	retVal := C.initializeDevice(cDeviceId)
	if retVal < 0 {
		return fmt.Errorf("Error with initializing device")
	}

	return nil
}

func GetWaveFormat() (*WaveFormat, error) {
	if C.isDeviceInitialized() == false {
		return nil, fmt.Errorf("Device not initialized")
	}
	retVal := C.getWaveFormat()

	if retVal == nil {
		return nil, fmt.Errorf("Error in retriving wave format")
	}

	wf := WaveFormat{
		wFormatTag:      uint16(retVal.wFormatTag),
		nChannels:       uint16(retVal.nChannels),
		nSamplesPerSec:  uint32(retVal.nSamplesPerSec),
		nAvgBytesPerSec: uint32(retVal.nAvgBytesPerSec),
		nBlockAlign:     uint16(retVal.nBlockAlign),
		cbSize:          uint16(retVal.cbSize),
	}
	if retVal.wFormatTag == 0xFFFE {
		nRetVal := (*C.WaveFormatExtensible)(unsafe.Pointer(retVal))
		wf.samples = uint16(nRetVal.Samples)
	}

	fmt.Println(unsafe.Sizeof(WaveFormat{}))
	return wf, nil
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
