%script to genereate butterworth bandpass filter


Fs = 1000e3;   % sampling frequency [Hz]

F1 = 100;   % lower cutoff frequency [Hz]
F2 = 500;   % upper cutoff frequency [Hz]

order = 2;

[b, a] = butter(order, [F1, F2]/Fs);

freqz(b,a);

duration = 0.01
data_length = duration*Fs;
x = (rand(1,data_length+1)-0.5) + gen_sine(250, duration, Fs);


y = filter(b,a,x);

plot(x);
hold on 
plot(y);