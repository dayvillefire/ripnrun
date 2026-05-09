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
