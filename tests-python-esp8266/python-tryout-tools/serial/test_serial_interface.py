import random
import string

import serial


def split_data(serial_line, tag):
    decoded_line = serial_line.decode().rstrip()
    split = decoded_line.split(tag + '.')
    return int(split[1])


def get_random_string(length):
    letters = string.ascii_lowercase
    return ''.join(random.choice(letters) for i in range(length))


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

command_str = "command"
ser.write(command_str.encode())
while True:
    serial_line = ser.readline()
    if not any(ext in str(serial_line) for ext in ignore_tags):
        print('Unrecognized reception: ', serial_line)
        ser.write(command_str.encode())
