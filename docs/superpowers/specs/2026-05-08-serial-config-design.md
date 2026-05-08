# Serial Configuration Interface — Design Spec

## Overview

Replace compile-time-only configuration (`config.h`) with persistent storage (ESP32 NVS via Preferences library), editable through an interactive serial menu. `config.h` values become compile-time fallback defaults.

## Activation

GPIO 0 held LOW during boot → enter config mode on serial (115200 baud). Normal boot otherwise.

While in config mode, all normal operation is suspended (no WiFi connect, no MQTT, no USB host, no printer). `setup()` blocks in the menu loop.

## Storage

ESP32 `Preferences` library, namespace `ripnrun`. One key per setting:

| Key | Type | config.h fallback |
|---|---|---|
| `wifi_ssid` | String | `DEFAULT_WIFI_SSID` |
| `wifi_pass` | String | `DEFAULT_WIFI_PASS` |
| `mqtt_broker` | String | `DEFAULT_MQTT_BROKER` |
| `mqtt_port` | Int | `DEFAULT_MQTT_PORT` |
| `mqtt_user` | String | `DEFAULT_MQTT_USER` |
| `mqtt_pass` | String | `DEFAULT_MQTT_PASS` |
| `mqtt_topic` | String | `DEFAULT_MQTT_TOPIC` |
| `agency_name` | String | `DEFAULT_AGENCY_NAME` |

## Menu

```
RIPNRUN Configuration
---------------------
 1. WiFi SSID        [myssid]
 2. WiFi Password    [****]
 3. MQTT Broker      [mqtt.one]
 4. MQTT Port        [1883]
 5. MQTT Username    [user]
 6. MQTT Password    [****]
 7. MQTT Topic       [station63/feed]
 8. Agency Name      [DAYVILLE FIRE COMPANY]
 A. Configure All (wizard)
 T. Test WiFi Connection
 S. Save & Exit
 X. Exit without saving
```

- Password fields show `****` instead of the stored value.
- Options 1-8: prompt for new value, show current in brackets, Enter keeps current.
- Option A: sequential walk through all 8 fields, same prompt style.
- Option T: `WiFi.begin()` with current in-memory SSID/password, 15-second timeout, reports "Connected, IP: x.x.x.x" or failure reason, then disconnects. Does not save.
- Option S: write all 8 values to NVS, print "Saved. Rebooting...", reboot.
- Option X: discard changes, reboot.

In-memory values are mutable copies — edits update the copy, NVS is only touched on Save.

## Code Structure

### New file: `src/config.cpp` / `src/config.hpp`

```cpp
// Getters — read NVS key, fall back to compile-time default if absent
String getWifiSsid();
String getWifiPass();
String getMqttBroker();
int    getMqttPort();
String getMqttUser();
String getMqttPass();
String getMqttTopic();
String getAgencyName();

// Serial config menu — blocks until Save or Exit
void configMenu();
```

### Modified file: `src/config.h`

Convert variables to `#define` defaults:

```cpp
#define DEFAULT_WIFI_SSID     "ssid"
#define DEFAULT_WIFI_PASS     "password"
#define DEFAULT_MQTT_BROKER   "mqtt.one"
#define DEFAULT_MQTT_PORT     1883
#define DEFAULT_MQTT_USER     "user"
#define DEFAULT_MQTT_PASS     "password"
#define DEFAULT_MQTT_TOPIC    "topic/subtopic"
#define DEFAULT_AGENCY_NAME   "DAYVILLE FIRE COMPANY"
```

### Modified file: `src/main.cpp`

- `#include "config.hpp"` replaces direct `config.h` references
- `setup()` checks GPIO 0: if LOW, call `configMenu()` and return (menu handles reboot)
- All code uses getter functions instead of `config.h` variables

### Modified file: `platformio.ini`

No new library dependencies — the `Preferences` library is part of the ESP32 Arduino core.

## CLI Interface Detail

Single-field prompt:
```
WiFi SSID [myssid]: _
```
- User types new value and presses Enter → stored in memory
- User presses Enter with empty input → keeps current value
- Leading/trailing whitespace is trimmed

Password fields echo `*` per character typed instead of the actual character.

Test WiFi output:
```
Testing WiFi connection to "myssid"...
Connected! IP: 192.168.1.42
Disconnecting...

(Press Enter to continue)
```
or:
```
Testing WiFi connection to "myssid"...
Failed: connection timeout

(Press Enter to continue)
```

## Verification

1. Flash with empty NVS: device boots normally using `config.h` defaults
2. Hold GPIO 0, reboot: config menu appears, verify all fields show defaults
3. Change WiFi SSID via option 1, test with T, verify connection succeeds
4. Save with S, verify device reboots into normal operation with new SSID
5. Re-enter config mode, verify all saved values persist
6. Exit without saving (X), verify NVS unchanged
