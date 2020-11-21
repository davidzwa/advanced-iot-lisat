clearvars
close all

Fs = 39000;
pulse_length = 0.017;
chirp_f0 = 1000;
chirp_f1 = 2000;

max_range = 2^15 - 1;

T_pulse = 0: 1/Fs: pulse_length-1/Fs;
Y_pulse = (max_range * chirp(T_pulse,chirp_f0,pulse_length,15000));
Y_pulse_int = floor(Y_pulse);

plot(T_pulse, Y_pulse)
hold on
plot(T_pulse, Y_pulse_int)

fprintf("const int16_t chirp[] = {")
for i = 1:length(Y_pulse_int)-1
   fprintf("%d, ", Y_pulse_int(i));
end
fprintf("%d};\n", Y_pulse_int(end));
