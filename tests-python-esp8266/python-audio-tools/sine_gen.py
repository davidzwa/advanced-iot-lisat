import pygame
import pygame.sndarray
import numpy
import scipy.signal
from time import sleep

sample_rate = 48000
pygame.mixer.pre_init(sample_rate, -16, 1, 1024)
pygame.init()


def square_wave(hz, peak, duty_cycle=.5, n_samples=sample_rate):
    t = numpy.linspace(0, 1, 500 * 440/hz, endpoint=False)
    wave = scipy.signal.square(2 * numpy.pi * 5 * t, duty=duty_cycle)
    wave = numpy.resize(wave, (n_samples,))
    return (peak / 2 * wave.astype(numpy.int16))


def audio_freq(freq=800):
    global sound
    sample_wave = square_wave(freq, 4096)
    sound = pygame.sndarray.make_sound(sample_wave)


# TEST
audio_freq()
sound.play(-1)
sleep(0.5)
sound.stop()
audio_freq(1000)
#sleep(1)
sound.play(-1)
sleep(2)
sound.stop()
sleep(1)
sound.play(-1)
sleep(0.5)
