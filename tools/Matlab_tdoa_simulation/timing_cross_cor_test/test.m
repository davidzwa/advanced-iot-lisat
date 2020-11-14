clearvars
close all

%%
delta_t_signal = 1/44000;
frequency_pulse = 5000;
pulse_length = 0.002;

T_pulse = 0: delta_t_signal: pulse_length-delta_t_signal;
y_pulse = chirp(T_pulse,5000,pulse_length,20000);

plot(T_pulse, y_pulse)
%%
signal_length = 0.02;
inter_pulse_delay = 0.003;

T_signal = 0:delta_t_signal: signal_length - delta_t_signal;
y_signal = zeros(1,length(T_signal));

loc = floor((0.001:inter_pulse_delay:signal_length-pulse_length)/delta_t_signal);

for i = 1:length(loc)
    y_signal(loc(i)+1:loc(i) + length(y_pulse)) = y_pulse;
end

y_signal = y_signal * 0.5;

y_signal_noise = awgn(y_signal, 1);

figure
plot(T_signal,y_signal_noise)
hold on
plot(T_signal,y_signal)

% figure
% [cor_dat,cor_lag] = xcorr_simp(y_pulse, y_signal_noise);
% stem(cor_lag,cor_dat)
% hold on 
% [c1,lags1] = xcorr_simp(y_pulse, y_signal);
% stem(lags1,c1)


%% collect data from cross
inter_pulse_delay_samples = floor(inter_pulse_delay/delta_t_signal);
num_pulses = 6;

zero_range = 50;

cor_dat_test = cor_dat;
cor_dat_test(cor_dat_test < 5) = 0;

pulses_found = 0;
pulse_values = zeros(1, num_pulses);
pulse_delays = zeros(1, num_pulses);

while pulses_found < num_pulses
    [current_pulse_value, current_pulse_delay] = max(cor_dat_test);
    
    pulses_found = pulses_found + 1;
    
    pulse_values(pulses_found) = current_pulse_value;
    pulse_delays(pulses_found) = cor_lag(current_pulse_delay);
    
    cordat2 = cor_dat_test(current_pulse_delay-zero_range: current_pulse_delay+zero_range);   
    cor_dat_test(current_pulse_delay-zero_range: current_pulse_delay+zero_range) = 0;
    
%     figure
%     stem(cor_lag, cor_dat_test);
%     hold on
%     stem(pulse_delays, pulse_values)
end

[pulse_delays_sort, i] = sort(pulse_delays);
pulse_values_sort = pulse_values(i);

signal_delay = mean(pulse_delays);








