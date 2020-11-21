
%[file, path] = uigetfile("*.csv");
file = convertCharsToStrings(file);
path = convertCharsToStrings(path);

addpath("..");
addpath("../timing_cross_cor_test");

Fs = 39000;
[u, ~] = gen_chirp(1000, 2000, 0.017, 0.017, Fs);
figure(1)
subplot(4,1,4)
plot(u)

num_pulses = 2;
signal_centers = zeros(1,2);
zero_range = ceil(0.005 * Fs);

full_path = path + file;

data = readmatrix(full_path);
buffer_length = length(data) / 3;

D = zeros(3,buffer_length);
for i = 1:3
    D(i,:) = data((i-1)*buffer_length + 1:   i*buffer_length ,2);
    figure(1)
    subplot(4,1,i)
    plot(D(i,:))
    
    [cor_dat,cor_lag] = xcorr_simp(u,D(i,:));
    figure(2)
    subplot(3,1,i)
    stem(cor_lag,cor_dat,'marker','none')
    
    pulse_values = zeros(1,num_pulses);
    pulse_delays = zeros(1,num_pulses);
    
    for j = 1:num_pulses
        [current_pulse_value, current_pulse_delay] = max(cor_dat);
        
        
        pulse_values(j) = current_pulse_value;
        pulse_delays(j) = cor_lag(current_pulse_delay);
        
        cordat2 = cor_dat(max(1,current_pulse_delay-zero_range): min(current_pulse_delay+zero_range,length(cor_dat)));
        cor_dat(max(1,current_pulse_delay-zero_range): min(current_pulse_delay+zero_range,length(cor_dat))) = 0;
        
    end
    
    signal_centers(i) = mean(pulse_delays);
    
    figure(2)
    hold on
    stem(pulse_delays, pulse_values) 
    
end

TDOAs_result = signal_centers(2:end) - signal_centers(1);






