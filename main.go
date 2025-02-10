package main

import "fmt"

func main() {
	devices, err := DiscoverDevices()

	if err != nil {
		fmt.Println("Error:", err)
		return
	}

	for _, device := range devices {
		fmt.Printf("Device ID: %s, Device Name: %s\n", device.Id, device.Name)
	}

	InitializeDevice("{0.0.0.00000000}.{6c661f6c-d8e3-4d63-b910-3683742085b4}")
	GetWaveFormat()
}
