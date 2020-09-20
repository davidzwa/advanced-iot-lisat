// Source: https://www.switchdoc.com/2015/10/iot-esp8266-timer-tutorial-arduino-ide/

extern "C"
{
#include "user_interface.h"
}

os_timer_t myTimer;

bool tickOccured;
void timerCallback(void *pArg)
{
    tickOccured = true;
}

void user_init(void)
{
    os_timer_setfn(&myTimer, timerCallback, NULL);
    os_timer_arm(&myTimer, 1000, true);
}

void setup()
{
    Serial.begin(250000);
    Serial.println();
    Serial.println();
    Serial.println("");
    Serial.println("--------------------------");
    Serial.println("ESP8266 Timer Test");
    Serial.println("--------------------------");
    tickOccured = false;
    user_init();
}

void loop()
{
    if (tickOccured == true)
    {

        Serial.println("Tick Occurred");
        tickOccured = false;
    }
    yield(); // or delay(0);
}
