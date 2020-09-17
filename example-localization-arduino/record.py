from playsound import playsound
import wave
import serial
import time

def get_time_ms():
    return int(round(time.time() * 1000))


def write_audio(audio_buffer, sample_width):
    chunk = 1024  # Record in chunks of 1024 samples
    channels = 1
    fs = 2300  # Record at 44100 samples per second
    seconds = 10
    filename = "output.wav"

    # print(bytes(audio_buffer))
    # print(b''.join(bytes(audio_buffer)))
    # Save the recorded data as a WAV file
    wf = wave.open(filename, 'wb')
    wf.setnchannels(channels)
    wf.setsampwidth(sample_width)
    wf.setframerate(fs)
    wf.writeframes(bytes(audio_buffer))
    wf.close()
    playsound(filename)


start_time = get_time_ms()
prev_time = get_time_ms()
sample_count = 0
ser = serial.Serial("COM3", 230400)
ser.flushInput()
data = []

while True:
    try:
        line = int(ser.readline().decode()[:-2]) # strip off any serial characters
    except UnicodeDecodeError:
        continue
    sample_count += 1
    abs_time = get_time_ms()
    print(abs_time - start_time, sample_count,
          1000 * sample_count/float(abs_time - start_time))
    
    data.append(line)
    if (abs_time - start_time > 10000):
        break
    
    prev_time = abs_time

sample_width = float(abs_time - start_time)/(1000 * sample_count)
write_audio(data, 1)
