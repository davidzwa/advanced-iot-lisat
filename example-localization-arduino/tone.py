import numpy
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
    arr = numpy.array([4096 * numpy.sin(2.0 * numpy.pi * freq * x / sampleRate)
                    for x in range(0, sampleRate)]).astype(numpy.int16)
    return numpy.c_[arr, arr]

freq = 100
while True:
    sound = pygame.sndarray.make_sound(get_sound(freq))
    sound.play(-1)
    pygame.time.delay(freq)
    sound.stop()
    pygame.time.delay(freq)

    freq += 50
