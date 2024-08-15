#include <Arduino.h>
#include <usb/usb_host.h>
#include "usbhelp.hpp"
#include <cstring>
#include "printer.hpp"
#include "escpos.hpp"

#include <WiFi.h>
#include <PubSubClient.h>

#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);
EscPos esc;

void pubSubCallback(char *topic, byte *raw, unsigned int length)
{
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");

    // Header
    esc.set_printmode(ESCPOS_PRINTMODE_DOUBLEHEIGHT);
    esc.align(ESCPOS_ALIGN_CENTER);
    esc.text(agency_name);
    esc.text("\r\n");
    esc.text("---------------------\r\n");
    esc.text(" \r\n");
    esc.align(ESCPOS_ALIGN_LEFT);
    esc.set_printmode(ESCPOS_PRINTMODE_OFF);

    for (int i = 0; i < length; i++)
    {
        esc.text("" + (char)raw[i]);
        Serial.print((char)raw[i]);
    }
    esc.text("\r\n");

    // Footer
    esc.align(ESCPOS_ALIGN_CENTER);
    esc.text("---------------------\r\n");
    esc.text(" \r\n");
    esc.text(" \r\n");
    esc.align(ESCPOS_ALIGN_LEFT);

    Serial.println();
    Serial.println("-----------------------");

    esc.flush();
    // print("\n\x0a\x4a\x04");
}

void setup()
{
    Serial.begin(115200);
    Serial.setTimeout(0);
    while (!Serial)
        ; // wait for serial port to connect.

    // Connecting to a WiFi network
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the Wi-Fi network");

    // connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(pubSubCallback);
    while (!client.connected())
    {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password))
        {
            Serial.println("Public EMQX MQTT broker connected");
        }
        else
        {
            Serial.print("Failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }

    usbh_setup(show_config_desc_full);

    // Publish and subscribe
    Serial.println("- Subscribe to topic");
    client.subscribe(topic);
}

void loop()
{
    usbh_task();
    client.loop();
#ifdef KEEPALIVE_PRINTER
    Serial.println(" - Keep alive printer");
    printPayload(esc.initialize());
    delay(1000);
#endif /* KEEPALIVE_PRINTER */
}