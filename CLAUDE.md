# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Commands

- **Build & flash:** `pio run -t upload` (or `make`, which also opens the serial monitor)
- **Serial monitor:** `pio device monitor -b 115200` (or `make monitor`)
- **Build only:** `pio run`

## Architecture

This is a PlatformIO project for an ESP32-S3 microcontroller. It subscribes to an MQTT topic for pager messages (fed by [pocsag-monitor](https://github.com/dayvillefire/pocsag-monitor)) and prints them on a USB-connected ESC/POS thermal receipt printer.

**Data flow:** MQTT message → `pubSubCallback()` in `src/main.cpp` → ESC/POS formatting via `src/escpos.cpp` → USB bulk OUT transfer via `src/printer.cpp` → thermal printer.

**Key modules:**

| Module | Role |
|---|---|
| `src/main.cpp` | Entry point: GPIO 0 config mode check, WiFi, MQTT, USB host init, main loop |
| `src/config.cpp/hpp` | NVS-backed config storage via `Preferences`, serial configuration menu |
| `src/escpos.cpp/hpp` | ESC/POS byte commands: alignment, font sizing, bold, reverse, image printing |
| `src/printer.cpp/hpp` | USB printer class detection (class 7, subclass 1), endpoint allocation, bulk transfer submission via `printPayload()` |
| `src/usbhelp.cpp/hpp` | USB host driver setup (`usbh_setup`) and event loop (`usbh_task`) — derived from ESP-IDF examples |
| `src/show_desc.cpp/hpp` | USB descriptor logging utilities (device, config, interface, endpoint) |
| `src/logo.h` | 300×333px monochrome bitmap buffer (`logoBuffer`) for logo printing |

**External libraries:** `PubSubClient` (MQTT), `ESPPubSubClientWrapper`.

## Configuration

`src/config.h` provides compile-time `#define` defaults. Actual settings are stored in ESP32 NVS (Preferences) and set interactively by holding GPIO 0 at boot to enter the serial configuration menu. Run `make && make monitor`, then reboot with GPIO 0 grounded to configure.
