#ifndef EXTERNAL_INTERRUPTS__H
#define EXTERNAL_INTERRUPTS__H

// Timer: Auxiliary variables
boolean startTimer = false;
unsigned long now = millis();
unsigned long lastTrigger = 0;
unsigned long lastTrigger2 = 0;
unsigned long lastTrigger3 = 0;

// CASPER's PLAYGROUND
long getCurrentPreciseTime() {
    return micros();
}

ICACHE_RAM_ATTR void interruptMicTriggered()
{
#ifdef PRINT_TRIGGER_INTERRUPTS
    Serial.println("m3");
#endif
    digitalWrite(ledPin, HIGH);
    startTimer = true;
    lastTrigger = getCurrentPreciseTime();
}

ICACHE_RAM_ATTR void interruptMic2Triggered()
{
#ifdef PRINT_TRIGGER_INTERRUPTS
    Serial.println("m2");
#endif
    lastTrigger2 = getCurrentPreciseTime();
}

ICACHE_RAM_ATTR void interruptMic3Triggered()
{
#ifdef PRINT_TRIGGER_INTERRUPTS
    Serial.println("m3");
#endif
    lastTrigger3 = getCurrentPreciseTime();
}

#endif // !EXTERNAL_INTERRUPTS__H
