from time import sleep

import serial
import traceback
import string
import random
import matplotlib.pyplot as plt
import json
from model.espdata import EspData
import dataclasses

def split_data(serial_line, tag):
    decoded_line = serial_line.decode().rstrip()
    split = decoded_line.split(tag + '.')
    return int(split[1])

def handle_parsing_error(serial_line):
    global received_sample_exceptions
    print('illegal line or parsing error', serial_line)
    received_sample_exceptions = received_sample_exceptions + 1
    if received_sample_exceptions > received_sample_exceptions_limit:
        exit('Too many wrong data samples received (>100). Please debug.')

def get_random_string(length):
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for i in range(length))

lastEspData = EspData()
espDataSet = list()

data_tag = 'v'
measure_tag = 'M'
RMS_tag = 'R'  # RMS value
algo_tag_dir = 'D'  # Direction output of TDOA inversion
param_tag = 'P'  # Can be made generic to specify 'what parameter'
separator = '.'
end_tag = '--Done'
overflow_tag = '!Buffer overflow'
ignore_tags = ['scandone', 'reconnect after', 'reconnect']
ser = serial.Serial('COM6', 115200)  # 1000000
ser.flushInput()

command_str= "command"
ser.write(command_str.encode())
while True:
    serial_line = ser.readline()
    if data_tag in str(serial_line):
        try:
            command_received = split_data(serial_line, data_tag)
            print(command_received)
        except Exception as e:
            handle_parsing_error(serial_line)
            pass
    elif algo_tag_dir in str(serial_line) and separator in str(serial_line):
            lastEspData.algoTdoaDir3 = split_data(
                serial_line, algo_tag_dir + '3')
    elif (measure_tag in str(serial_line) or param_tag in str(serial_line)) and separator in str(serial_line):
        lastEspData.mic1LTimeUs = split_data(
            serial_line, measure_tag + '1')
    elif (RMS_tag + separator in str(serial_line)):
        RMS = split_data(
                serial_line, RMS_tag)
    elif end_tag in str(serial_line):
        espDataSet.append(dataclasses.asdict(lastEspData))
        input("Press [enter] to continue.")
    else:
        if not any(ext in str(serial_line) for ext in ignore_tags):
            print('Unrecognized reception: ', serial_line)
            ser.write(command_str.encode())
