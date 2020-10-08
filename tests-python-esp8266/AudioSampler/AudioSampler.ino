// Intellisense
// https://stackoverflow.com/questions/51614871/why-wont-arduino-intellisense-work-in-vscode

#include "pindefs.h"
#include "defines.h"
#include "audioAdcTimer.h"
#include "externalInterrupts.h"
#include "serialInterface.h"
#include <ESP8266WiFi.h>

#ifdef MQTT_CLIENT
#include "mqttClient.h"
WiFiClient espClient;
PubSubClient client(espClient);
#endif

void user_init(void)
{
    // ADC timer
    initOsTimer(sampling_period_us);

    // LED as visual tool
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    // Start listening for wake-on-sound
    pinMode(mic1LTriggerPin, INPUT_PULLUP);
    pinMode(wosModePin1L, OUTPUT); // https://www.puiaudio.com/media/SpecSheet/PMM-3738-VM1010-R.pdf
    pinMode(wosModePin2M, OUTPUT);
    pinMode(wosModePin3R, OUTPUT);
    resetWosMicMode();

#ifdef MQTT_CLIENT
    // Setup WiFi & MQTT pubsub client
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
#else
    WiFi.disconnect();
#endif
}

void setup()
{
    Serial.begin(serial_baud_rate);

    user_init();

    // Prepare runtime
#ifndef NO_BUFFER
    // resetWosMicMode();
    // enableMicTriggerInterrupts();
#else
    setNormalMicMode();
#endif // NO_BUFFER
}

void loop()
{
#ifdef NO_BUFFER
    transmitSerialValue(analogRead(analogInPin));
#else
    // Current time
    if (startAdcSampling)
    {
        digitalWrite(ledPin, LOW);
    }
    else
    {
        digitalWrite(ledPin, HIGH);
    }
#endif

#ifdef MQTT_CLIENT
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > 2000)
    {
        lastMsg = now;
        ++value;
        snprintf(mqqt_msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
        Serial.print("Publish message: ");
        Serial.println(mqqt_msg);
        client.publish("outTopic", mqqt_msg);
    }
#endif

    processIncomingSerial();
    yield(); // or delay(0);
}
