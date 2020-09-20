from playsound import playsound
import wave
import serial
import time
import traceback
import string, random

data_tag = '[i,t,v]:'
end_tag = '-- Done'
overflow_tag = '!Buffer overflow'
ignore_tags = ['scandone', 'reconnect after', 'reconnect']
ser = serial.Serial("COM5", 1000000)
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
    wf.setframerate(2000)
    wf.writeframes(bytes(audio_buffer))
    wf.close()

def normalize_integer(value_list):
    return [int(255*float(i)/max(value_list)) for i in value_list]

index_vector = list()
time_vector = list()
sound_vector = list()

while True:
    serial_line = ser.readline()
    if data_tag in str(serial_line):
        received_samples += 1
        try:
            values = split_data(serial_line)
            time_value = int(values[1])
            sound_value = int(values[2])
        except Exception as e:
            print(traceback.format_exc())
            handle_parsing_error(serial_line)
            pass
        
        # index_vector.append(values[0])
        time_vector.append(time_value)
        sound_vector.append(sound_value)
    elif end_tag in str(serial_line):
        received_samples = 0
        rng_filename = "audio/" + get_random_string(3) + base_filename
        if max(sound_vector) > 255:
            sound_vector = normalize_integer(sound_vector)
        frequency = sum(time_vector)/len(time_vector)        
        # Perform tasks
        write_audio(sound_vector, 1, frequency, rng_filename)
        playsound(rng_filename)
        
        sound_vector = []
        time_vector = []
    elif overflow_tag in str(serial_line):
        print('Buffer overflow exception!')
    else:
        if not any(ext in str(serial_line) for ext in ignore_tags):
            print('Unrecognized reception: ', serial_line)
