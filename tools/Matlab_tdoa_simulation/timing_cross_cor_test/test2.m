clearvars
close all
% opengl('software')
Fs = 44000;
%% Pulse generation
pulse_length = 0.017;
chirp_f0 = 1500;
chirp_f1 = 6000;

T_pulse = 0: 1/Fs: pulse_length-1/Fs;
Y_pulse = chirp(T_pulse,chirp_f0,pulse_length,15000);

%% Signal generation
signal_length = 0.06;
inter_pulse_time = 0.003;

pulse_times =1 + floor((0: (pulse_length + inter_pulse_time): (signal_length - pulse_length)) * Fs);

num_pulses = length(pulse_times);

T_signal = 0: 1/Fs: signal_length-1/Fs;
Y_signal = zeros(1, length(T_signal));

for i = 1:num_pulses
    Y_signal(pulse_times(i):pulse_times(i) + length(Y_pulse) -1) = Y_pulse;
end

plot(T_signal, Y_signal);

%% Propogate signal (time shift, attinuation and noise)
buffer_length_samples = 3000;

buffers = zeros(3,buffer_length_samples);

sample_delays = randi([50, 100]) + [0, randi(75,1,2)];
snr = 0.0001*ones(1,3);%[1,5,10];
attenuation = [1, 0.8, 0.6];

% True TDOAs form sample delays, with value 1 as reference
TDOAs_true = sample_delays(2:end) - sample_delays(1);

for i = 1:3
    buffers(i,sample_delays(i): sample_delays(i) + length(Y_signal) -1) = Y_signal;
    buffers(i,:) = awgn(buffers(i,:),snr(i));
    buffers(i,:) = attenuation(i).*buffers(i,:);
end

figure
for i = 1:3
    subplot(3,1,i)
    plot(buffers(i,:))
end

%% Finding the TDOAs
inter_pulse_samples = inter_pulse_time*Fs;

zero_range = floor(0.4*inter_pulse_samples);

signal_centers = zeros(1,3);

f(1) = figure;
f(2) = figure;
f(3) = figure;
f(4) = figure('Renderer', 'painters', 'Position', [10 10 800 300]);

for i = 1:3
    % Per buffer (= per microphone)
    % crosscorrelate with chirp pulse
    [cor_dat,cor_lag] = xcorr_simp(Y_pulse, buffers(i,:));
    
    figure(f(4))
    subplot(3,1,i)
    stem(cor_lag, cor_dat,'Marker','none', "DisplayName", "Correlation results")
    
    
    figure(f(i))
    subplot(2,1,1)
    stem(cor_lag, cor_dat)
    
    pulse_values = zeros(1,num_pulses);
    pulse_delays = zeros(1,num_pulses);
    
    for j = 1:num_pulses
        [current_pulse_value, current_pulse_delay] = max(cor_dat);
        
        
        pulse_values(j) = current_pulse_value;
        pulse_delays(j) = cor_lag(current_pulse_delay);
        
        cordat2 = cor_dat(current_pulse_delay-zero_range: current_pulse_delay+zero_range);
        cor_dat(current_pulse_delay-zero_range: current_pulse_delay+zero_range) = 0;
        
    end
    
    signal_centers(i) = mean(pulse_delays);
    
    subplot(2,1,2);    
    stem(cor_lag, cor_dat);
    hold on
    stem(pulse_delays, pulse_values)    
    
    figure(f(4))
    subplot(3,1,i)
    hold on
    stem(pulse_delays, pulse_values,':o', "DisplayName", "Found chirps")
    xline(mean(pulse_delays),"DisplayName", "Determined arrival time","LineWidth", 1)
%     ylabel("Corrolation value")
    if i == 3
        xlabel("Sample delay")
    end
    legend('show')
    ylim([-100, 500])
    xlim([-700, 3500])
end

TDOAs_result = signal_centers(2:end) - signal_centers(1);

disp(TDOAs_true)
disp(TDOAs_result)


