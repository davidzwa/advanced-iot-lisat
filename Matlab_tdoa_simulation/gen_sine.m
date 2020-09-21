function [y, t] = gen_sine(duration,frequency, Fs)
% frequency in hertz
t = 0:1/Fs:duration;
y = sin(2*pi*frequency * t);
end

