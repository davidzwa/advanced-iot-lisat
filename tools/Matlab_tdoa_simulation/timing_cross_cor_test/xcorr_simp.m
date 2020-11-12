function [c,lags] = xcorr_simp(x_pulse, x_signal)

lags = -length(x_pulse):(length(x_signal))-1;
c = -30*ones(1,length(lags));

padding_length = length(x_pulse);

x_signal_padded = [zeros(1,padding_length), x_signal, zeros(1,padding_length)];

for i = 1:length(lags)
    x_signal_part = x_signal_padded(i:i+length(x_pulse)-1);
    c(i) = sum(x_signal_part .* x_pulse);
    
end


end

