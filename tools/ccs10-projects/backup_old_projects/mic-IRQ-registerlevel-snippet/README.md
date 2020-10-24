# Mic IRQ at registerlevel
A small code snippet slapped together by Amjad and David in order to get a very simplistic interrupt + ADC sampling scheme:

## Idea
External interrupt on a pin (DigitalOut from Mic) triggers a timer.
This timer is setup at init to sample at a specific frequency at 'quite a high priority' (we know that in future different timers will have higher priority, like the tachometer/PWM of motors.). Once the timer triggers, we busy-wait a bit to get a valid ADC sample. This we do until we reach a certain amount of samples.
Then we stop the timer and listen again for the external interrupt.

## Design parameters:
- (Nice) sampling frequency => 'rounded' timer period fitting in MSCLK
- Amount of samples -> source length (+ reflections?)
- Priority of timer
- Which timer, which interrupt pin, which mic/analogIn ADC pin.

Signal frequency: estimated 2kHz or higher. Sampling freq: 4kHz or higher. 250uS per sample.
Signal length: estimated 50ms or more (=> influences cosine rollof/hann window...).
