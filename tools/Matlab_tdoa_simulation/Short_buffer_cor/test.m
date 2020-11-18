clearvars
close all

Fs = 44000;
buffer_length = 12;

%% Pulse generation
pulse_length = 0.02;
pulse_frequency = 11000;

T_pulse = 0: 1/Fs: pulse_length-1/Fs;
Y_pulse = sin(2*pi*pulse_frequency*T_pulse);
%Y_pulse = zeros(1,length(T_pulse));

plot(T_pulse, Y_pulse);

%% Propogate signal (time shift, attinuation and noise)
signal_length = ceil((length(T_pulse) + 500)/buffer_length)*buffer_length;

sample_delay = randi(500);
snr = 10;

% True TDOAs form sample delays, with value 1 as reference

signal = zeros(1,signal_length);
signal(sample_delay:sample_delay + length(T_pulse) - 1) = Y_pulse;
signal = awgn(signal,snr);

figure
plot(signal)

%% Record short buffers

num_recorded_buffers = signal_length/buffer_length;
recorded_buffers = zeros(num_recorded_buffers, buffer_length);


for i = 0:num_recorded_buffers-1
    start_idx = i*buffer_length+1;
    if start_idx + buffer_length - 1 >= signal_length
        1+1;
    end
    
    recorded_buffers(i+1,:) = signal(start_idx:start_idx+buffer_length-1);
    
    if mod(i,12) == 0
        figure
    end
    subplot(2,6,mod(i,12)+1)
    plot(recorded_buffers(i+1,:))    
    
end

%% Test vectors

num_test_phases = 3;
test_phases = pi* (0:1/num_test_phases: 1-1/num_test_phases);

T_test_vector = 0: 1/Fs: (buffer_length - 1)/Fs;

test_vectors = zeros(num_test_phases, buffer_length);

figure
for i = 1:num_test_phases
    test_vectors(i,:) = sin(2*pi*pulse_frequency*T_test_vector + test_phases(i));
    plot(T_test_vector,test_vectors(i,:),"DisplayName","phase = " + num2str(test_phases(i)));
    hold on
end
legend

%% buffer based coorrelation

corr_values = zeros(num_test_phases, num_recorded_buffers);

figure
for i = 1:num_test_phases
    for j = 1:num_recorded_buffers
        corr_values(i,j) = recorded_buffers(j,:) * test_vectors(i,:)';
    end
    stem(corr_values(i,:),"DisplayName","phase = " + num2str(test_phases(i)))
    hold on
end
legend

%% Make single value feature

feature_values = max(abs(corr_values));

figure
stem(feature_values)



