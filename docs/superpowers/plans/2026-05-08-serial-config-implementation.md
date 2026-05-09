# Serial Configuration Interface — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Migrate WiFi/MQTT config from compile-time `config.h` to persistent NVS storage with an interactive serial configuration menu triggered by holding GPIO 0 at boot.

**Architecture:** New `src/config.cpp/hpp` module wraps ESP32 Preferences (NVS) for reading/writing settings, exposes getters used by main.cpp, and hosts the `configMenu()` serial UI. `config.h` becomes a `#define`-only fallback-defaults header. `main.cpp` checks GPIO 0 in `setup()` and either enters config mode or proceeds with normal operation using stored (or default) values.

**Tech Stack:** ESP32 Arduino core (Preferences library, WiFi, Serial) — no new external dependencies.

---

### Task 1: Convert config.h to compile-time defaults

**Files:**
- Modify: `src/config.h`

- [ ] **Step 1: Replace variables with #define defaults**

Replace the entire content of `src/config.h` with:

```cpp
/**
 * RIPNRUN - @jbuchbinder
 *
 * "Rip and run" software to operate an ESC-POS printer connected to an MQTT
 * queue. Works with pocsag-monitor software for pager monitoring.
 *
 * DEFAULT SETTINGS — overridden by NVS-stored values set via serial config.
 */

#ifndef CONFIG_H
#define CONFIG_H

// WiFi defaults
#define DEFAULT_WIFI_SSID     "ssid"
#define DEFAULT_WIFI_PASS     "password"

// MQTT Broker defaults
#define DEFAULT_MQTT_BROKER   "mqtt.one"
#define DEFAULT_MQTT_PORT     1883
#define DEFAULT_MQTT_USER     "user"
#define DEFAULT_MQTT_PASS     "password"
#define DEFAULT_MQTT_TOPIC    "topic/subtopic"

// Agency
#define DEFAULT_AGENCY_NAME   "DAYVILLE FIRE COMPANY"

#endif
```

- [ ] **Step 2: Stage the file**

```bash
git add src/config.h
```

(Do not commit yet — will commit with the rest in Task 4.)

---

### Task 2: Create config.hpp with function declarations

**Files:**
- Create: `src/config.hpp`

- [ ] **Step 1: Write config.hpp**

```cpp
#ifndef CONFIG_INTERFACE_H
#define CONFIG_INTERFACE_H

#include <Arduino.h>

String getWifiSsid();
String getWifiPass();
String getMqttBroker();
int    getMqttPort();
String getMqttUser();
String getMqttPass();
String getMqttTopic();
String getAgencyName();

void configMenu();

#endif
```

- [ ] **Step 2: Stage**

```bash
git add src/config.hpp
```

---

### Task 3: Create config.cpp with storage layer and serial menu

**Files:**
- Create: `src/config.cpp`

- [ ] **Step 1: Write the Preferences-backed getters and the configMenu implementation**

```cpp
#include "config.hpp"
#include "config.h"
#include <Preferences.h>
#include <WiFi.h>

#define PREFS_NAMESPACE "ripnrun"

// --- Getters: read NVS, fall back to compile-time default ---

String getWifiSsid() {
    Preferences p;
    p.begin(PREFS_NAMESPACE, true);
    String v = p.getString("wifi_ssid", DEFAULT_WIFI_SSID);
    p.end();
    return v;
}

String getWifiPass() {
    Preferences p;
    p.begin(PREFS_NAMESPACE, true);
    String v = p.getString("wifi_pass", DEFAULT_WIFI_PASS);
    p.end();
    return v;
}

String getMqttBroker() {
    Preferences p;
    p.begin(PREFS_NAMESPACE, true);
    String v = p.getString("mqtt_broker", DEFAULT_MQTT_BROKER);
    p.end();
    return v;
}

int getMqttPort() {
    Preferences p;
    p.begin(PREFS_NAMESPACE, true);
    int v = p.getInt("mqtt_port", DEFAULT_MQTT_PORT);
    p.end();
    return v;
}

String getMqttUser() {
    Preferences p;
    p.begin(PREFS_NAMESPACE, true);
    String v = p.getString("mqtt_user", DEFAULT_MQTT_USER);
    p.end();
    return v;
}

String getMqttPass() {
    Preferences p;
    p.begin(PREFS_NAMESPACE, true);
    String v = p.getString("mqtt_pass", DEFAULT_MQTT_PASS);
    p.end();
    return v;
}

String getMqttTopic() {
    Preferences p;
    p.begin(PREFS_NAMESPACE, true);
    String v = p.getString("mqtt_topic", DEFAULT_MQTT_TOPIC);
    p.end();
    return v;
}

String getAgencyName() {
    Preferences p;
    p.begin(PREFS_NAMESPACE, true);
    String v = p.getString("agency_name", DEFAULT_AGENCY_NAME);
    p.end();
    return v;
}

// --- Helpers ---

static String maskPassword(const String &s) {
    if (s.length() == 0) return "";
    String m;
    for (unsigned int i = 0; i < s.length(); i++) m += '*';
    return m;
}

static String readLine(bool masked) {
    String input;
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') {
                Serial.println();
                break;
            }
            if (c == '\b' || c == 127) {
                if (input.length() > 0) {
                    input.remove(input.length() - 1);
                    Serial.print("\b \b");
                }
                continue;
            }
            if (c >= 32 && c <= 126) {
                input += c;
                Serial.print(masked ? '*' : c);
            }
        }
    }
    input.trim();
    return input.length() > 0 ? input : "";
}

static String promptField(const char *label, const String &current, bool masked) {
    Serial.print(label);
    Serial.print(" [");
    Serial.print(masked ? maskPassword(current) : current);
    Serial.print("]: ");
    String input = readLine(masked);
    if (input.length() == 0) return current;
    return input;
}

static int promptIntField(const char *label, int current) {
    Serial.print(label);
    Serial.print(" [");
    Serial.print(current);
    Serial.print("]: ");
    String input = readLine(false);
    if (input.length() == 0) return current;
    return input.toInt();
}

static void pressEnterToContinue() {
    Serial.println();
    Serial.println("(Press Enter to continue)");
    while (true) {
        if (Serial.available()) {
            char c = Serial.read();
            if (c == '\n' || c == '\r') break;
        }
    }
}

static void saveAllPrefs(const String &ssid, const String &pass,
                         const String &broker, int port,
                         const String &user, const String &mqttPass,
                         const String &topic, const String &agency) {
    Preferences p;
    p.begin(PREFS_NAMESPACE, false);
    p.putString("wifi_ssid", ssid);
    p.putString("wifi_pass", pass);
    p.putString("mqtt_broker", broker);
    p.putInt("mqtt_port", port);
    p.putString("mqtt_user", user);
    p.putString("mqtt_pass", mqttPass);
    p.putString("mqtt_topic", topic);
    p.putString("agency_name", agency);
    p.end();
}

static void testWifi(const String &ssid, const String &pass) {
    Serial.print("Testing WiFi connection to \"");
    Serial.print(ssid);
    Serial.println("\"...");

    WiFi.begin(ssid.c_str(), pass.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > 15000) {
            Serial.println("Failed: connection timeout");
            WiFi.disconnect();
            pressEnterToContinue();
            return;
        }
        delay(200);
    }

    Serial.print("Connected! IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("Disconnecting...");
    WiFi.disconnect();
    pressEnterToContinue();
}

static void configureAllWizard(String &ssid, String &pass, String &broker,
                               int &port, String &user, String &mqttPass,
                               String &topic, String &agency) {
    Serial.println("\n--- Configure All ---\n");
    ssid = promptField("WiFi SSID", ssid, false);
    pass = promptField("WiFi Password", pass, true);
    broker = promptField("MQTT Broker", broker, false);
    port = promptIntField("MQTT Port", port);
    user = promptField("MQTT Username", user, false);
    mqttPass = promptField("MQTT Password", mqttPass, true);
    topic = promptField("MQTT Topic", topic, false);
    agency = promptField("Agency Name", agency, false);
    Serial.println();
}

// --- Main config menu ---

void configMenu() {
    // Load current values into in-memory copies
    String ssid = getWifiSsid();
    String pass = getWifiPass();
    String broker = getMqttBroker();
    int port = getMqttPort();
    String user = getMqttUser();
    String mqttPass = getMqttPass();
    String topic = getMqttTopic();
    String agency = getAgencyName();

    while (true) {
        Serial.println("\n\nRIPNRUN Configuration");
        Serial.println("---------------------");
        Serial.print(" 1. WiFi SSID        [");
        Serial.print(ssid);
        Serial.println("]");
        Serial.print(" 2. WiFi Password    [");
        Serial.print(maskPassword(pass));
        Serial.println("]");
        Serial.print(" 3. MQTT Broker      [");
        Serial.print(broker);
        Serial.println("]");
        Serial.print(" 4. MQTT Port        [");
        Serial.print(port);
        Serial.println("]");
        Serial.print(" 5. MQTT Username    [");
        Serial.print(user);
        Serial.println("]");
        Serial.print(" 6. MQTT Password    [");
        Serial.print(maskPassword(mqttPass));
        Serial.println("]");
        Serial.print(" 7. MQTT Topic       [");
        Serial.print(topic);
        Serial.println("]");
        Serial.print(" 8. Agency Name      [");
        Serial.print(agency);
        Serial.println("]");
        Serial.println(" A. Configure All (wizard)");
        Serial.println(" T. Test WiFi Connection");
        Serial.println(" S. Save & Exit");
        Serial.println(" X. Exit without saving");
        Serial.print("\nChoice: ");

        String choice = readLine(false);
        choice.toUpperCase();

        if (choice == "1") {
            ssid = promptField("WiFi SSID", ssid, false);
        } else if (choice == "2") {
            pass = promptField("WiFi Password", pass, true);
        } else if (choice == "3") {
            broker = promptField("MQTT Broker", broker, false);
        } else if (choice == "4") {
            port = promptIntField("MQTT Port", port);
        } else if (choice == "5") {
            user = promptField("MQTT Username", user, false);
        } else if (choice == "6") {
            mqttPass = promptField("MQTT Password", mqttPass, true);
        } else if (choice == "7") {
            topic = promptField("MQTT Topic", topic, false);
        } else if (choice == "8") {
            agency = promptField("Agency Name", agency, false);
        } else if (choice == "A") {
            configureAllWizard(ssid, pass, broker, port, user, mqttPass, topic, agency);
        } else if (choice == "T") {
            testWifi(ssid, pass);
        } else if (choice == "S") {
            saveAllPrefs(ssid, pass, broker, port, user, mqttPass, topic, agency);
            Serial.println("Saved. Rebooting...");
            delay(500);
            ESP.restart();
        } else if (choice == "X") {
            Serial.println("Exiting without saving. Rebooting...");
            delay(500);
            ESP.restart();
        } else {
            Serial.print("Unknown option: ");
            Serial.println(choice);
        }
    }
}
```

- [ ] **Step 2: Stage**

```bash
git add src/config.cpp
```

---

### Task 4: Update main.cpp to use getters and add GPIO 0 config trigger

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Rewrite main.cpp**

Replace the current `src/main.cpp` content with:

```cpp
#include <Arduino.h>
#include <usb/usb_host.h>
#include "usbhelp.hpp"
#include <cstring>
#include "printer.hpp"
#include "escpos.hpp"

#include <WiFi.h>
#include <ESPPubSubClientWrapper.h>
#include "config.hpp"

#define CONFIG_PIN 0
#define WIFICHECK_INTERVAL 1000L

ESPPubSubClientWrapper *client = nullptr;
String agency_name;

void pubSubCallback(char *topic, byte *raw, unsigned int length)
{
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");

    String payload = String();

    // Header
    payload += escpos_set_printmode(ESCPOS_PRINTMODE_DOUBLEHEIGHT);
    payload += escpos_align(ESCPOS_ALIGN_CENTER);
    payload += agency_name;
    payload += "\r\n";
    payload += "---------------------\r\n";
    payload += " \r\n";
    payload += escpos_align(ESCPOS_ALIGN_LEFT);
    payload += escpos_set_printmode(ESCPOS_PRINTMODE_OFF);

    for (unsigned int i = 0; i < length; i++)
    {
        payload += (char)raw[i];
        Serial.print((char)raw[i]);
    }
    payload += "\r\n";

    // Footer
    payload += escpos_align(ESCPOS_ALIGN_CENTER);
    payload += "---------------------\r\n";
    payload += " \r\n";
    payload += " \r\n";
    payload += escpos_align(ESCPOS_ALIGN_LEFT);

    Serial.println();
    Serial.println("-----------------------");

    printPayload(payload);
}

void connectWifi()
{
    String ssid = getWifiSsid();
    String pass = getWifiPass();

    if (ssid.length() == 0)
    {
        Serial.println(F("WiFi SSID not set, rebooting..."));
        delay(1000);
        ESP.restart();
    }

    WiFi.begin(ssid.c_str(), pass.c_str());

    const unsigned long timeout_ms = 30000;
    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        if (millis() - start > timeout_ms)
        {
            Serial.println(F("\nWiFi connection timeout, rebooting..."));
            delay(1000);
            ESP.restart();
        }
        Serial.print(".");
        delay(300);
    }

    Serial.print(" IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
}

void checkWifi()
{
    if ((WiFi.status() != WL_CONNECTED))
    {
        Serial.println(F("\nWiFi lost. Reconnecting..."));
        connectWifi();
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.setTimeout(0);
    while (!Serial)
        ; // wait for serial port to connect.

    // GPIO 0 held LOW at boot -> config mode
    pinMode(CONFIG_PIN, INPUT_PULLUP);
    delay(50);
    if (digitalRead(CONFIG_PIN) == LOW)
    {
        Serial.println(F("\nEntering configuration mode..."));
        delay(200); // debounce
        configMenu(); // blocks until reboot
        return;      // unreachable, but explicit
    }

    // Reset connection
    WiFi.disconnect();
    delay(1000);

    // Connecting to a WiFi network
    connectWifi();

    // Load agency name from storage
    agency_name = getAgencyName();

    // Create MQTT client
    String broker = getMqttBroker();
    int port = getMqttPort();
    client = new ESPPubSubClientWrapper(broker.c_str(), port);

    client->setServer(broker.c_str(), port);
    client->setCallback(pubSubCallback);

    String client_id = "esp32-client-";
    client_id += String((char *)WiFi.macAddress(NULL));
    client->connect(client_id.c_str(),
                    getMqttUser().c_str(),
                    getMqttPass().c_str());

    usbh_setup(show_config_desc_full);

    // Publish and subscribe
    Serial.println("- Subscribe to topic");
    client->on(getMqttTopic().c_str(), pubSubCallback);
    usbh_setup(show_config_desc_full);
}

void check_status()
{
    static uint32_t checkwifi_timeout = 0;
    uint32_t current_millis = millis();

    if ((current_millis > checkwifi_timeout) || (checkwifi_timeout == 0))
    {
        checkWifi();
        checkwifi_timeout = current_millis + WIFICHECK_INTERVAL;
    }
}

void loop()
{
    check_status();
    usbh_task();
    if (client) client->loop();
    delay(100);
#ifdef KEEPALIVE
    Serial.println(" - Keep alive printer");
    printPayload(escpos_initialize());
    delay(1000);
#endif
}
```

- [ ] **Step 2: Stage main.cpp**

```bash
git add src/main.cpp
```

---

### Task 5: Build verification

**Files:**
- None (read-only verification)

- [ ] **Step 1: Run PlatformIO build**

```bash
cd /home/jbuchbinder/Code/Arduino/ripnrun && pio run
```

Expected: Build succeeds with no errors.

- [ ] **Step 2: Fix any compilation errors**

If the build fails, examine errors and fix. Common issues to watch for:
- `String((char *)WiFi.macAddress(NULL))` may need a different cast on some ESP32 core versions. If it errors, try `WiFi.macAddress()` (returns String) instead.
- `client->on(...)` — verify `ESPPubSubClientWrapper` exposes `on()` with the same signature as the old `client.on()`.

---

### Task 6: Update CLAUDE.md

**Files:**
- Modify: `CLAUDE.md`

- [ ] **Step 1: Update architecture section**

Replace the `src/main.cpp` line in the key modules table and add the config module:

Old:
```
| `src/main.cpp` | Entry point: WiFi, MQTT subscribe (via `ESPPubSubClientWrapper`), USB host init, main loop |
```

New:
```
| `src/main.cpp` | Entry point: GPIO 0 config mode check, WiFi, MQTT subscribe, USB host init, main loop |
| `src/config.cpp/hpp` | NVS-backed config storage via `Preferences`, serial configuration menu |
```

Replace the config section at the bottom:

Old:
```
`src/config.h` contains WiFi credentials, MQTT broker settings, and agency name. It is **git-crypt encrypted** — copy `src/config.h.example` and fill in real values to set up a new device.
```

New:
```
`src/config.h` provides compile-time `#define` defaults. Actual settings are stored in ESP32 NVS (Preferences) and can be set interactively by holding GPIO 0 at boot to enter the serial configuration menu. Run `make && make monitor`, then reboot with GPIO 0 grounded to configure.
```

- [ ] **Step 2: Stage and commit all changes**

```bash
git add CLAUDE.md src/config.h src/config.hpp src/config.cpp src/main.cpp
git commit -m "$(cat <<'EOF'
Add NVS-backed config with interactive serial configuration menu

- Convert config.h from const variables to #define fallback defaults
- Add config.cpp/hpp: Preferences-backed getters with compile-time
  fallbacks, full interactive serial config menu
- GPIO 0 held LOW at boot enters config mode (blocks setup())
- Menu supports single-field edit, full wizard, WiFi connection test
- Password fields masked with * during input and display
- ESPPubSubClientWrapper now heap-allocated (config loaded at runtime)
- Update CLAUDE.md

Co-Authored-By: Claude Opus 4.7 <noreply@anthropic.com>
EOF
)"
```

---

### Task 7: Manual verification checklist

These require physical hardware and cannot be automated.

- [ ] **Step 1: Defaults boot** — Flash with cleared NVS (`esptool.py erase_flash` or first flash). Device boots normally using `config.h` defaults.
- [ ] **Step 2: Config mode entry** — Hold GPIO 0 to GND, power on. Config menu appears on serial at 115200 baud.
- [ ] **Step 3: Default values displayed** — All 8 menu fields show `config.h` default values.
- [ ] **Step 4: Single field edit** — Select option 1, enter a new SSID. Verify menu re-displays with new SSID.
- [ ] **Step 5: WiFi test** — Set valid credentials, press T. Verify "Connected! IP: x.x.x.x" appears.
- [ ] **Step 6: WiFi test failure** — Set invalid SSID, press T. Verify "Failed: connection timeout" appears.
- [ ] **Step 7: Save and reboot** — Press S. Verify "Saved. Rebooting..." and device boots with saved config.
- [ ] **Step 8: Persistence** — Re-enter config mode (GPIO 0). Verify saved values are shown, not defaults.
- [ ] **Step 9: Exit without save** — Change a value, press X. Re-enter config mode, verify old value persisted.
- [ ] **Step 10: Normal operation** — Boot without GPIO 0. Verify device connects to WiFi, subscribes to MQTT, and prints pager messages.
