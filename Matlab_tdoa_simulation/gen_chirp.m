function [u, t] = gen_chirp(f0, f1, t1, duration, Fs)

t = 0:1/Fs:duration;
u = chirp(t, f0, t1, f1);
u(t>t1) = 0;

end

