clearvars
close all

Fs = 44000;

%% Preamble

preamble_length = 0.02;
preamble_frequency = 11000;

%% Chirps
F0 = 1500;
F1 = 6000;

chirp_length = 0.017;
inter_chirp_length = 0.003;

num_chirps = 3;

%% Misc

inter_preamle_chirp_time = 0.003;

%% Signal gen

T_preamble = 0: 1/Fs: preamble_length-1/Fs;
Y_preamble = sin(2*pi*preamble_frequency*T_preamble);

T_chirp = 0: 1/Fs: chirp_length-1/Fs;
Y_chirp = chirp(T_chirp, F0, chirp_length, F1);

Y_inter_chirp = zeros(1,ceil(inter_chirp_length*Fs));

Y_inter_preamble_chirp = zeros(1,ceil(inter_preamle_chirp_time*Fs));


T_signal = 0: 1/Fs: (preamble_length + inter_preamle_chirp_time + ...
    num_chirps * chirp_length + (num_chirps-1)*inter_chirp_length ...
    - 1/Fs);

T_frec = [0,preamble_length,...
    preamble_length,preamble_length + inter_preamle_chirp_time,...
    preamble_length + inter_preamle_chirp_time];
Y_frec = [preamble_frequency, preamble_frequency...
    0, 0,...
    F0];

Y_signal = [Y_preamble, Y_inter_preamble_chirp];
for i = 1:num_chirps-1
    Y_signal = [Y_signal, Y_chirp, Y_inter_chirp];
    
    T_frec = [T_frec, T_frec(end)+chirp_length, ...
        T_frec(end)+chirp_length];
    T_frec = [T_frec, T_frec(end)+inter_chirp_length, ...
        T_frec(end)+inter_chirp_length];
    
    Y_frec = [Y_frec, F1, 0, 0, F0];
    
    
end
Y_signal = [Y_signal, Y_chirp];
T_frec = [T_frec, T_frec(end)+chirp_length];
Y_frec = [Y_frec, F1];
Y_frec = Y_frec/1000;

%% plot
figure('Renderer', 'painters', 'Position', [10 10 800 300])
yyaxis left
plot(T_signal, Y_signal)
ylim([-1.25, 1.25])
xlabel("Time [s]")
ylabel("Amplitude")

yyaxis right
plot(T_frec, Y_frec)
ylabel("Frequency [kHz]")
ylim([0 14]);
yyaxis left
y_text = 1.1;

x_preamble_text = T_preamble(end/2);
text(x_preamble_text, y_text, "Preamble", "HorizontalAlignment", "center");

x_chirps_text = T_signal(length(Y_preamble) + length(Y_inter_preamble_chirp) ...
    +(num_chirps * length(Y_chirp) + (num_chirps-1)*length(Y_inter_chirp))/2);
text(x_chirps_text, y_text, "Chirps", "HorizontalAlignment", "center");

x_divider = T_signal(length(Y_preamble) + length(Y_inter_preamble_chirp)/2);
text(x_divider, y_text,"|","HorizontalAlignment", "center");

