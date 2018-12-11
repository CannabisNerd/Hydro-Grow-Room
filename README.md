Web based IoT Hydroponics Grow Room sketch using Arduino IDE for the ESP8266. Originally run on the Wemos D1 R2 & Wemos D1 Mini. But should be completely compatible with other ESP8266 and ESP32 boards chips.

Sketch relies on a free account at https://mydevices.com/ Once account is created you will add a new device and will be given a username, password, and client ID that is device specific. Edit this sketch to include your information.

This is what a simple dashboard looks like https://imgur.com/a/5VZf8U3

Features:
Uses a DS18b20 and onewire bus to monitor water reservoir temperature (Can be adopted for DWC systems with a sensore per site)
DHT11 or DHT22 sensor for Air Temp and humidity, DHT11 has a humidity range that maxes at 80% so I suggest the DHT22
5V relay wired to 110V AC outlet for pump control
Photoresistor light sensor 

Configurable timers for relay to control water pump for feeding (timers and alarms can be used for other features)
Web dashboard that displays current Air temp/humidty as well as water reservoir temperature


Code is still a little sloppy I am aware and I will be cleaning it up, but this is a start. Future upgrades to this release will include resrovoir water level, automatic fan control based on temperature, EC, and PH readings. As well as web based configuration of some sort.
