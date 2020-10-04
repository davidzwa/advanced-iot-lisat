from playsound import playsound
import wave
import serial
import time

def get_time_ms():
    return int(round(time.time() * 1000))

seconds = 10
frequency = 8000

def write_audio(audio_buffer, sample_width, frequency):
    channels = 1
    filename = "output.wav"

    # Save the recorded data as a WAV file
    wf = wave.open(filename, 'wb')
    wf.setnchannels(channels)
    wf.setsampwidth(sample_width)
    wf.setframerate(frequency)
    wf.writeframes(bytes(audio_buffer))
    wf.close()
    playsound(filename)


start_time = get_time_ms()
prev_time = get_time_ms()
sample_count = 0
ser = serial.Serial("COM4", 250000)
ser.flushInput()
data = []

while True:
    try:
        line = int(ser.readline().decode()[:-2]) # strip off any serial characters
    except UnicodeDecodeError:
        continue
    except ValueError:
        continue
    sample_count += 1
    abs_time = get_time_ms()
    print(abs_time - prev_time, sample_count,
          1000 * sample_count/float(abs_time - start_time))
    
    data.append(line)
    if (abs_time - start_time > 1000 * seconds):
        break
    prev_time = abs_time

# print(data)
# print(int(2 * 1000 * sample_count/float(abs_time - start_time)))
# frequency = 4000 # 1000 * sample_count/float(abs_time - start_time)
# sample_width = float(abs_time - start_time)/(1000 * sample_count)
# write_audio(data, 1, frequency)

frequency = 4500  # 1000 * sample_count/float(abs_time - start_time)
write_audio(data, 1, frequency)
