# ESP32-PCF85063A

## Environment

**Build environment:** Linux Mint 20 x86_64<br>
**Target device:** ESP32 NodeMCU<br>
**ESPIDF version:** v4.4


## Description

This is a basic driver for the NXP RTC PCF85063A for ESP32 using ESP-IDF, it also contains a simple example using the PCF85063A driver and some timing manipulations. 
Hope it helps :)

## Reproduce steps
Check out your `I2C_MASTER_CLK` and change `SDA`, `SCL` pins if necessary in the `PCF85063A.h` file , as follows:
```c
#define SDA_PIN         21
#define SCL_PIN         22
#define I2C_MASTER_CLK  100000
```

To run this example, make sure you have ESP-IDF installed and run the command:
```
idf.py build flash monitor
```
