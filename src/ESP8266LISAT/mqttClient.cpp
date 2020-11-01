#include "mqttClient.h"
#include "defines.h"

// WiFi/AP config ESP
#include <ESP8266WiFi.h> // Include the Wi-Fi library

const char *ssid = "Brus";
const char *password = "Tackle1963";
const char *mqtt_server = "192.168.1.10";
const char *mqtt_user = "david";
const char *mqtt_password = "Davido12";
char mqqt_msg[MSG_BUFFER_SIZE];
int value = 0;
unsigned long lastMsg = 0;

extern WiFiClient espClient;
extern PubSubClient client;

void setup_wifi()
{
    delay(10);
    // We start by connecting to a WiFi network
    // Serial.println();
    // Serial.print("Connecting to ");
    // Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        // Serial.print(".");
    }

    randomSeed(micros());

    // Serial.println("");
    // Serial.println("WiFi connected");
    // Serial.println("IP address: ");
    // Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
#ifndef SERIAL_SWAP_MSP432
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    // #else
    // Serial.print(topic);
#endif
    if (strncmp((char *)topic, "MSP", 3) == 0)
    {
        for (int i = 0; i < length; i++)
        {
            Serial.print((char)payload[i]);
        }
        Serial.print("\n");
        digitalWrite(BUILTIN_LED, !digitalRead(BUILTIN_LED));
    }
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        // Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str(), mqtt_user, mqtt_password))
        {
            Serial.print("MSP!CONN1\n");
            // Once connected, publish an announcement...
            // client.publish("outTopic", "hello world");
            // ... and resubscribe
            client.subscribe("inTopic");
            client.subscribe("MSP");
        }
        else
        {
            Serial.print("MSP!CONN0\n");
#ifndef SERIAL_SWAP_MSP432
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
#endif
            delay(MQTT_DISCONNECT_RETRY);
        }
    }
}
