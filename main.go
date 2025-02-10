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
}
