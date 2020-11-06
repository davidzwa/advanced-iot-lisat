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
from model.dataset import DataSet
import dataclasses
import simpleaudio as sa
from scipy.signal import butter, lfilter, freqz
import math


# Recognizable parameters sent over Serial
valin_tag_dir = 'Dv'  # Unused Valin tag
ctp_tag_dir = 'Dp'  # Unused Ctp tag
data_tag = 'v'
measure_tag = 'M'
sampling_frequency_tag = 'F'
sample_size_tag = 'S'
RMS_tag = 'R'  # RMS value
param_tag = 'P'  # Can be made generic to specify 'what parameter'
data_max_tag = 'Ma'  # Maximum and Minimum value from buffer data
data_min_tag = 'Mi'

separator = '.'
end_tag = '--Done'
overflow_tag = '!Buffer overflow'
ignore_tags = ['scandone', 'reconnect after', 'reconnect']
ser = serial.Serial('COM4', 115200)  # 1000000
ser.flushInput()
data = []
received_samples = 0
received_sample_exceptions = 0
received_sample_exceptions_limit = 100
MAX_MIC_TIME = 20000
last_sample = ''
base_filename = 'output2.wav'
dirname = os.path.dirname(__file__)
outputfile = 'data/output_experiment/dataset-' + datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S') + \
    '.json'
print(outputfile)
if not os.path.exists(os.path.dirname(outputfile)):
    try:
        os.makedirs(os.path.dirname(outputfile))
    except OSError as exc:  # Guard against race condition
        if exc.errno != errno.EEXIST:
            raise


def split_data_float(serial_line, tag):
    decoded_line = serial_line.decode().rstrip()
    split = decoded_line.split(tag + '.')
    return float(split[1])


def split_data(serial_line, tag):
    return int(split_data_float(serial_line, tag))


def handle_parsing_error(serial_line):
    global received_sample_exceptions
    print('illegal line or parsing error', serial_line)
    received_sample_exceptions = received_sample_exceptions + 1
    # if received_sample_exceptions > received_sample_exceptions_limit:
    #     exit('Too many wrong data samples received (>100). Please debug.')


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


def check_tag_in_serialline(serial_line, tag):
    return tag in str(serial_line)

# def butter_lowpass(cutoff, fs, order=5):
#     nyq = 0.5 * fs
#     normal_cutoff = cutoff / nyq
#     b, a = butter(order, normal_cutoff, btype='low', analog=False)
#     return b, a

# def butter_lowpass_filter(data, cutoff, fs, order=5):
#     b, a = butter_lowpass(cutoff, fs, order=order)
#     y = lfilter(b, a, data)
#     return y

# # Filter requirements.
# order = 2
# fs = 10000       # sample rate, Hz
# cutoff = 1000  # desired cutoff frequency of the filter, Hz
# # Get the filter coefficients so we can check its frequency response.
# b, a = butter_lowpass(cutoff, fs, order)

# # Plot the frequency response.
# w, h = freqz(b, a, worN=8000)
# plt.subplot(2, 1, 1)
# plt.plot(0.5*fs*w/np.pi, np.abs(h), 'b')
# plt.plot(cutoff, 0.5*np.sqrt(2), 'ko')
# plt.axvline(cutoff, color='k')
# plt.xlim(0, 0.5*fs)
# plt.title("Lowpass Filter Frequency Response")
# plt.xlabel('Frequency [Hz]')
# plt.grid()


# fig = plt.figure()
# ax = fig.add_subplot(111)
# line1, = ax.plot([], 'r-')
# plt.ion()
# plt.show()

# espDataSet = list()

def calculate_theta(x, y):
    if (x == 0 and y > 0):
        return math.pi/2
    if (x == 0 and y < 0):
        return math.pi*(3/2)
    return (math.atan(y/x)*180/math.pi)


def do_experiment():
    num_rounds = 0
    num_rounds_max = 10  # number of trials per single location
    received_samples = 0
    recording = False
    espDataSet = list()
    # do something with `num_rounds to repeat x times`

    lastEspData = EspData()
    previousEspData = None
    sound_vector = list()
    while num_rounds < num_rounds_max:
        if recording is True:
            print("entered recording")
            serial_line = ser.readline()
            print(serial_line)
            if check_tag_in_serialline(serial_line, data_max_tag):
                lastEspData.max = split_data(
                    serial_line, data_max_tag)
            elif check_tag_in_serialline(serial_line, data_min_tag):
                lastEspData.min = split_data(
                    serial_line, data_min_tag)
            elif check_tag_in_serialline(serial_line, sample_size_tag):
                lastEspData.sampleSize = split_data(
                    serial_line, sample_size_tag)
            elif check_tag_in_serialline(serial_line, sampling_frequency_tag):
                lastEspData.samplingRate = split_data(
                    serial_line, sampling_frequency_tag)
            elif check_tag_in_serialline(serial_line, valin_tag_dir):
                #print('valin dir: ', str(serial_line))
                if valin_tag_dir + '1' in str(serial_line):
                    lastEspData.algoTdoaDirX = split_data_float(
                        serial_line, valin_tag_dir + '1')
                elif valin_tag_dir + '2' in str(serial_line):
                    lastEspData.algoTdoaDirY = split_data_float(
                        serial_line, valin_tag_dir + '2')
            elif check_tag_in_serialline(serial_line, ctp_tag_dir):
                #print('ctp dir: ', serial_line)
                if ctp_tag_dir + '1' in str(serial_line):
                    lastEspData.algoCTPDirX = split_data_float(
                        serial_line, ctp_tag_dir + '1')
                elif ctp_tag_dir + '2' in str(serial_line):
                    lastEspData.algoCTPDirY = split_data_float(
                        serial_line, ctp_tag_dir + '2')
            elif check_tag_in_serialline(serial_line, data_tag):
                received_samples += 1
                try:
                    sound_value = split_data(serial_line, data_tag)
                    if sound_value < 16383:
                        sound_vector.append(sound_value)
                except Exception as e:
                    print(e)
                    # print(traceback.format_exc())
                    handle_parsing_error(serial_line)
                    pass
            elif check_tag_in_serialline(serial_line, measure_tag) or check_tag_in_serialline(serial_line, param_tag):
                #print('measure_tag, param_tag: ', str(serial_line))
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
                    lastEspData.samplingRate = split_data(
                        serial_line, param_tag)
            elif (check_tag_in_serialline(serial_line, RMS_tag + separator)):
                # print('RMS_tag')
                RMS = split_data(
                    serial_line, RMS_tag)
                # if RMS >0:
            elif check_tag_in_serialline(serial_line, end_tag):
                #print("valin end", lastEspData.algoTdoaDirX)
                received_samples = 0
                rng_filename = "audio/" + get_random_string(3) + base_filename
                # sound_vector = normalize_integer(sound_vector)

                if len(sound_vector) < lastEspData.sampleSize and len(sound_vector) > 0:
                    recording = False
                    print('sound_vector length too short')
                elif len(sound_vector) > lastEspData.sampleSize and len(sound_vector) > 0:
                    print("samplesize: {} len(soundvector) {}".format(
                        lastEspData.sampleSize, len(sound_vector)))
                    recording = False
                    print('sound_vector length too long')
                elif lastEspData.mic1LTimeUs > MAX_MIC_TIME or lastEspData.mic2MTimeUs > MAX_MIC_TIME:
                    recording = False
                    print('sample started too late, dropped')
                else:
                    lastEspData.soundData = sound_vector

                    # Calculate RMS
                    sound_array = np.array(sound_vector)
                    lastEspData.rms = np.sqrt(np.mean(sound_array ** 2))
                    # If sample was valid but the same as previous sample, skip
                    # if lastEspData == previousEspData:5
                    #     print('current sample was the same as previous sample')
                    # else:
                    previousEspData = lastEspData

                    print("NEW SAMPLE ---")
                    print("mic1 time", lastEspData.mic1LTimeUs)
                    print("mic2 time", lastEspData.mic2MTimeUs)
                    # print("mic3 time", lastEspData.mic3RTimeUs)
                    # print("valin x", lastEspData.algoTdoaDirX)
                    # print("valin y", lastEspData.algoTdoaDirY)
                    # print("ctp x", lastEspData.algoCTPDirX)
                    # print("ctp y", lastEspData.algoCTPDirY)
                    #print("min", lastEspData.min)
                    #print("max", lastEspData.max)
                    #print("sampling rate", lastEspData.samplingRate)
                    #print("sample size", lastEspData.sampleSize)
                    print('RMS:', lastEspData.rms)
                    # print('dir valin:', calculate_theta(lastEspData.algoTdoaDirX, lastEspData.algoTdoaDirY))
                    # print('dir ctp:', calculate_theta(lastEspData.algoCTPDirX, lastEspData.algoCTPDirY))

                    print('\n')

                    espDataSet.append(dataclasses.asdict(lastEspData))
                    # dump_dataset(espDataSet, outputfile)
                    num_rounds += 1
                    # sound_array_filtered = butter_lowpass_filter(sound_array, cutoff, fs, order)
                    # plt.subplot(2, 1, 2)

                    # plt.plot(sound_array)

                    # plt.plot(sound_array_filtered)

                    # plt.plot(np.fft.fft(sound_array).real**2 +
                    #          np.fft.fft(sound_array).imag**2)

                    # plt.draw()
                    # plt.pause(0.01)  # speakertest

                # input("Press [enter] to continue.")
                # Perform tasks
                frequency = 10000
                # write_audio(sound_vector, 1, frequency, rng_filename)
                # playsound(rng_filename)

                sound_vector = []

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
            wave_obj = sa.WaveObject.from_wave_file(
                "data/audio/fingers_short.wav")
            play_obj = wave_obj.play()
            play_obj.wait_done()
            # time.sleep(1)
            recording = True
    # num_rounds done
    return espDataSet


if __name__ == '__main__':
    # number of different locations at which measurements are performed
    number_of_locations = 5
    for experimentIndex in range(0, number_of_locations):
        print("Provide dir:")
        dir = input()
        # validate x_value being valid float
        # print("Provide Y:")
        # y_value = input()
        # validate y_value being valid float

        singleLocationSet: List[EspData] = do_experiment()
        # print("Provide info:")

        # create new DataSet
        dataset = DataSet()
        dataset.espDataSets = singleLocationSet  # lijst toekennen: = espDataSet
        dataset.direction = dir
        # print(dataset)
        dump_dataset(dataclasses.asdict(dataset), outputfile)
