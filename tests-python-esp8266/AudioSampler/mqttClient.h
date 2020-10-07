#ifndef MQTT_CLIENT__H
#define MQTT_CLIENT__H

const char *mqtt_server = "broker.mqtt-dashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char mqqt_msg[MSG_BUFFER_SIZE];
int value = 0;

#endif // !MQTT_CLIENT__H
