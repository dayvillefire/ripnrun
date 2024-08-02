# RIPNRUN

A "rip and run" generator for pages, passed through an MQTT queue, based on the [PlatformIO](https://platformio.org/) embedded framework. It relies on the [pocsag-monitor](https://github.com/dayvillefire/pocsag-monitor) software to listen to the pages, and an MQTT queue software (like the free [mqtt.one](https://mqtt.one/index.html)) to function.

The `src/config.h` file needs to be adjusted with your settings. 

## Required Hardware

* [ESP32 S3](https://www.amazon.com/dp/B0D4C7LJZX) - $18.99 for 3
* [Receipt Printer](https://www.amazon.com/gp/product/B0CQCT2NXX/) - $47.99
* [Power/Data USB C Splitter](https://www.amazon.com/dp/B0CB3M46Y5) - $12.99 
* (optional) [Case](https://www.amazon.com/gp/product/B07Q11F7DS/) - $6.49 for 5

You probably should have a USB micro power supply for the ESP32, as well.

## Build

With PlatformIO installed, use this to build and upload, then monitor:

`pio run -t upload && pio device monitor -b 115200`

