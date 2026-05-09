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
