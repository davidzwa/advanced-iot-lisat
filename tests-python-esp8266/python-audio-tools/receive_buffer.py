from playsound import playsound
from typing import List
import wave
import serial
import time
import traceback
import string
import os
import random
import numpy as np
import matplotlib.pyplot as plt
import json
import datetime
from model.espdata import EspData
import dataclasses
import simpleaudio as sa

data_tag = 'v'
measure_tag = 'M'
param_tag = 'P'  # Can be made generic to specify 'what parameter'
separator = '.'
end_tag = '--Done'
overflow_tag = '!Buffer overflow'
ignore_tags = ['scandone', 'reconnect after', 'reconnect']
ser = serial.Serial('COM5', 250000)  # 1000000
ser.flushInput()
data = []
received_samples = 0
received_sample_exceptions = 0
received_sample_exceptions_limit = 100
last_sample = ''
base_filename = 'output2.wav'
dirname = os.path.dirname(__file__)
outputfile = 'output/' + datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S') + \
    '/dataset.json'
print(outputfile)
if not os.path.exists(os.path.dirname(outputfile)):
    try:
        os.makedirs(os.path.dirname(outputfile))
    except OSError as exc:  # Guard against race condition
        if exc.errno != errno.EEXIST:
            raise


def split_data(serial_line, tag):
    decoded_line = serial_line.decode().rstrip()
    values = decoded_line.split(tag)[1]
    return values.split('.')


def handle_parsing_error(serial_line):
    global received_sample_exceptions
    print('illegal line or parsing error', serial_line)
    received_sample_exceptions = received_sample_exceptions + 1
    if received_sample_exceptions > received_sample_exceptions_limit:
        exit('Too many wrong data samples received (>100). Please debug.')


def get_random_string(length):
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for i in range(length))


def write_audio(audio_buffer, sample_width, frequency, filename):
    channels = 1
    # Save the recorded data as a WAV file
    wf = wave.open(filename, 'wb')
    wf.setnchannels(channels)
    wf.setsampwidth(sample_width)
    wf.setframerate(frequency)
    wf.writeframes(bytes(audio_buffer))
    wf.close()


def dump_dataset(dataset: List, filename):
    with open(filename, 'w') as file:
        json.dump(dataset, file, indent=4, sort_keys=True)

# Scale a value given min and max of chunk's samples to [0, 255] scale


def map_value_8bit_unsigned(value, min, max):
    ratio = 255.0 / (max - min)

    # We dont want to fully compress nor normalize - we just want a normal range
    if (ratio < 10.0):
        return int((value - min) * ratio)
    else:
        # If the min/max difference is that small, the ratio is big and we shouldnt normalizeclip the signal
        return int(value - min)


def normalize_integer(value_list):
    max_value = max(value_list)
    min_value = min(value_list)
    mapped_list = [map_value_8bit_unsigned(
        i, min(value_list), max(value_list)) for i in value_list]
    return mapped_list


fig = plt.figure()
ax = fig.add_subplot(111)
sound_vector = list()
line1, = ax.plot([], 'r-')
plt.ion()
plt.show()

recording = False
lastEspData = EspData()
espDataSet = list()

while True:
    if recording is True:
        serial_line = ser.readline()
        if data_tag in str(serial_line):
            received_samples += 1
            try:
                values = split_data(serial_line, data_tag)
                sound_value = int(values[1])
                sound_vector.append(sound_value)
            except Exception as e:
                print(traceback.format_exc())
                handle_parsing_error(serial_line)
                pass
        elif (measure_tag in str(serial_line) or param_tag in str(serial_line)) and separator in str(serial_line):
            if measure_tag + '1' in str(serial_line):
                lastEspData.mic1LTimeUs = split_data(
                    serial_line, measure_tag + '1')
            elif measure_tag + '2' in str(serial_line):
                lastEspData.mic2MTimeUs = split_data(
                    serial_line, measure_tag + '2')
            elif measure_tag + '3' in str(serial_line):
                lastEspData.mic3RTimeUs = split_data(
                    serial_line, measure_tag + '3')
            elif param_tag in str(serial_line):
                lastEspData.samplingRate = split_data(serial_line, param_tag)
        elif end_tag in str(serial_line):
            received_samples = 0
            rng_filename = "audio/" + get_random_string(3) + base_filename
            # sound_vector = normalize_integer(sound_vector)

            lastEspData.soundData = sound_vector
            sound_array = np.array(sound_vector)

            espDataSet.append(dataclasses.asdict(lastEspData))
            dump_dataset(espDataSet, outputfile)

            rms = np.sqrt(np.mean(sound_array**2))
            # print(sound_array, rms)
            print(sound_array)
            plt.plot(sound_array)
            # plt.plot(np.fft.fft(sound_array).real**2 +
            #          np.fft.fft(sound_array).imag**2)
            plt.draw()
            plt.pause(3)


            # input("Press [enter] to continue.")
            # Perform tasks
            frequency = 2000
            # write_audio(sound_vector, 1, frequency, rng_filename)
            # playsound(rng_filename)

            sound_vector = []
            time_vector = []

            # Answer back
            # ser.write(bytes("ati", 'utf-8'))
            # ser.writelines("V".encode());
            recording = False
        # elif overflow_tag in str(serial_line): # wont occur anymore
        #     print('Buffer overflow exception!')
        else:
            if not any(ext in str(serial_line) for ext in ignore_tags):
                print('Unrecognized reception: ', serial_line)
    else:
        print("Playing sound")
        wave_obj = sa.WaveObject.from_wave_file("audio/fingers.wav")
        play_obj = wave_obj.play()
        play_obj.wait_done()
        recording=True
