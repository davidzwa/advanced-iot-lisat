import numpy as np
import pygame

sampleRate = 44100
freq = 1000

pygame.mixer.init(44100, -16, 2, 512)
# sampling frequency, size, channels, buffer

# Sampling frequency
# Analog audio is recorded by sampling it 44,100 times per second,
# and then these samples are used to reconstruct the audio signal
# when playing it back.

# size
# The size argument represents how many bits are used for each
# audio sample. If the value is negative then signed sample
# values will be used.

# channels
# 1 = mono, 2 = stereo

# buffer
# The buffer argument controls the number of internal samples
# used in the sound mixer. It can be lowered to reduce latency,
# but sound dropout may occur. It can be raised to larger values
# to ensure playback never skips, but it will impose latency on sound playback.

def get_sound(freq):
    arr = np.array([4096 * np.sin(2.0 * np.pi * freq * x / sampleRate)
                    for x in range(0, sampleRate)]).astype(np.int16)
    return np.c_[arr, arr]


x = np.arange(0, 100, 0.1)


def triang(x, phase, length, amplitude):
    alpha = (amplitude)/(length/2)
    return -amplitude/2+amplitude*((x-phase) % length == length/2) \
        + alpha*((x-phase) % (length/2))*((x-phase) % length <= length/2) \
        + (amplitude-alpha*((x-phase) % (length/2))) * \
        ((x-phase) % length > length/2)


phase = -10
length = 5  # should be positive
amplitude = 10240
tr = triang(x, phase, length, amplitude).astype(np.int16)

freq = 1000
while True:
    snd = np.c_[tr, tr]
    sound = pygame.sndarray.make_sound(snd)
    sound.play(-1)
    pygame.time.delay(50)
    sound.stop()
    pygame.time.delay(200)

    freq += 50
