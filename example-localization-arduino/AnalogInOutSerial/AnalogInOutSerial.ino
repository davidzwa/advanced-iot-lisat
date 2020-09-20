
const int analogInPin = A0;
int sensorValue = 0;
int prevTime = 0;
void setup()
{
    Serial.begin(250000);
}

void loop()
{
    int timeDiff = micros() - prevTime;
    prevTime = micros();

    sensorValue = analogRead(analogInPin);
    
    Serial.print(timeDiff);
    Serial.println(map(sensorValue, 0, 1023, 0, 255));
    
    Serial.print("s = ");
    Serial.print(sensorValue);
    Serial.print("\t o = ");
    Serial.println(outputValue);
}


#define ADC_SAMPLES_COUNT 1000
int16_t analogBuffer[ADC_SAMPLES_COUNT];
int16_t abufPos = 0;

void IRAM_ATTR onTimer()
{
    portENTER_CRITICAL_ISR(&timerMux);

    analogBuffer[abufPos++] = local_adc1_read(ADC1_CHANNEL_0);

    if (abufPos >= ADC_SAMPLES_COUNT)
    {
        abufPos = 0;

        // Notify adcTask that the buffer is full.
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(adcTaskHandle, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken)
        {
            portYIELD_FROM_ISR();
        }
    }
    portEXIT_CRITICAL_ISR(&timerMux);
}
