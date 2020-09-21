function [y, t] = gen_imp(duration,width, Fs)
t = 0:1/Fs:duration;
y = zeros(1,length(t));
y(t<=width) = 1;
end

