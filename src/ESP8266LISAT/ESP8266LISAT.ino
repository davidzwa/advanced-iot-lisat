// Intellisense
// https://stackoverflow.com/questions/51614871/why-wont-arduino-intellisense-work-in-vscode

#include "defines.h"
#include "pindefs.h"
#ifdef MIC3_MEASURE_SETUP
#include "audioAdcTimer.h"
#include "externalInterrupts.h"
#endif
#include "serialInterface.h"
#include <ESP8266WiFi.h>

#ifdef MQTT_CLIENT
#include "mqttClient.h"
WiFiClient espClient;
PubSubClient client(espClient);
#endif

void user_init(void)
{
    // LED as visual tool
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

#ifdef MIC3_MEASURE_SETUP
    // ADC timer
    initOsTimer(sampling_period_us);

    // Start listening for wake-on-sound
    pinMode(mic1LTriggerPin, INPUT_PULLUP);
    pinMode(wosModePin1L, OUTPUT); // https://www.puiaudio.com/media/SpecSheet/PMM-3738-VM1010-R.pdf
    pinMode(wosModePin2M, OUTPUT);
    pinMode(wosModePin3R, OUTPUT);
    resetWosMicMode();

    // Prepare runtime
#ifndef NO_BUFFER
    resetWosMicMode();
    enableMicTriggerInterrupts(); // Interrupts arm the timer
#else
    setNormalMicMode();
#endif // NO_BUFFER
#endif

    Serial.begin(serial_baud_rate);
#ifdef SERIAL_SWAP_MSP432
    // Swap to GPIO13/D7 RX, GPIO15/D8 TX
    Serial.swap();
    // Serial.setDebugOutput(0);
    // system_set_os_print(0);
#endif

#ifdef MQTT_CLIENT
    // Setup WiFi & MQTT pubsub client
    setup_wifi();
    client.setServer(mqtt_server, 18883);
    client.setCallback(callback);
#else
    WiFi.disconnect();
#endif
}

void setup()
{
    user_init();
}

void loop()
{
#ifdef MIC3_MEASURE_SETUP
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
        client.publish("outTopic", mqqt_msg);
    }
#endif

    processIncomingSerial();
    yield(); // or delay(0);
}
