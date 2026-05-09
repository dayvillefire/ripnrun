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
