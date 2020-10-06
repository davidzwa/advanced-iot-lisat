from playsound import playsound
import wave
import serial
import time
import traceback
import string, random

data_tag = '[v]'
end_tag = '-- Done'
overflow_tag = '!Buffer overflow'
ignore_tags = ['scandone', 'reconnect after', 'reconnect']
ser = serial.Serial("COM4", 1000000)
ser.flushInput()
data = []
received_samples = 0
received_sample_exceptions = 0
received_sample_exceptions_limit = 100
last_sample = ''
base_filename = 'output2.wav'

def split_data(serial_line):
    decoded_line = serial_line.decode().rstrip()
    values = decoded_line.split(data_tag)[1]
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
    mapped_list = [map_value_8bit_unsigned(i, min(value_list), max(value_list)) for i in value_list]
    return mapped_list

index_vector = list()
time_vector = list()
sound_vector = list()

while True:
    serial_line = ser.readline()
    if data_tag in str(serial_line):
        received_samples += 1
        try:
            values = split_data(serial_line)
            sound_value = int(values[1])
            sound_vector.append(sound_value)
        except Exception as e:
            print(traceback.format_exc())
            handle_parsing_error(serial_line)
            pass
        
        # index_vector.append(values[0])
        # time_vector.append(time_value)
    elif end_tag in str(serial_line):
        received_samples = 0
        rng_filename = "audio/" + get_random_string(3) + base_filename
        sound_vector = normalize_integer(sound_vector)
        # Perform tasks
        frequency = 1 / (500 * 1E-6)
        # write_audio(sound_vector, 1, frequency, rng_filename)
        # playsound(rng_filename)
        
        sound_vector = []
        time_vector = []
        ser.write(bytes("ati", 'utf-8'))
        # ser.writelines("V".encode());
    elif overflow_tag in str(serial_line):
        print('Buffer overflow exception!')
    else:
        if not any(ext in str(serial_line) for ext in ignore_tags):
            print('Unrecognized reception: ', serial_line)
