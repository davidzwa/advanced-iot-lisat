#ifndef MQTT_CLIENT__H
#define MQTT_CLIENT__H

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define MSG_BUFFER_SIZE (50)

extern const char *mqtt_server;
extern char mqqt_msg[MSG_BUFFER_SIZE];
extern int value;
extern unsigned long lastMsg;

void setup_wifi();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();

#endif // !MQTT_CLIENT__H
