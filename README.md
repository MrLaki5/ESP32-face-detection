# ESP32 Web Camera Stream
Camera streaming with web server on ESP32 microcontroller

## Setup
git submodule update --init --recursive
cd components/esp-dl && git apply ../../esp-dl.patch
Flash SD card to FAT and create file called wifi.txt, and put WiFi credentials inside. First row network SSID, second row network password.

## Build only
idf.py build

## Flash with building
idf.py flash -b 115200

## Monitor run
idf.py monitor
