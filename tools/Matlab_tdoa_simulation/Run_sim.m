function [y, yt] = Run_sim(env, u, t, Fs_d, Fs_a)
%% Analog filter
a_a = 1;
b_a = 1;

f1_a = 100;     % lower cutoff frequency analog filter[Hz]
f2_a = 500;     % upper cutoff frequency analog filter[Hz]

o_a = 1;        % order of analog filter

%[b_a, a_a] = butter(o_a, [f1_a, f2_a]/Fs_a);

%% Digital filter

a_d = 1;
b_d = 1;

f1_d = 100;     % lower cutoff frequency digital filter[Hz]
f2_d = 500;     % upper cutoff frequency digital filter[Hz]

o_d = 1;        % order of analog filter

%[b_d, a_d] = butter(o_d, [f1_d, f2_d]/Fs_d);

%% Noise
SNR = 1;

%% Run sim

yt = 0:1/Fs_d: t(end);
y = zeros(env.num_robots, env.num_mics, length(yt));

for r = 1:env.num_robots
    for m = 1:env.num_mics
        prop = 0;
        for s = 1:env.num_sources
            
            prop = prop + lsim(env.trans(s,r,m),u,t); % simulate propagation
            
        end
        
        %prop = awgn(prop,SNR, 'measured');  %adding noise to the signal
        
        analog_in = filter(b_a, a_a, prop); %simulating hardware filtering
        
        rx_raw = interp1(t, analog_in, yt);  %sampling by the uc
        
        rx_filtered = filter(b_d, a_d, rx_raw); % software filtering
        
        y(r,m,:)= rx_filtered;
        
    end
end



end

