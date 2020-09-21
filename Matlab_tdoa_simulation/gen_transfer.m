function env = gen_transfer(env, Fs, c)
%generate transfer functions [source, robot, mic]
% c = wave velocity in m/s

for s = 1:env.num_sources
    for r = 1:env.num_robots
        for m = 1:env.num_mics
            dist = norm(env.sources(s,:)- env.mics(m,:,r));
            sample_delay = ceil(dist*Fs/c);
            
            env.trans(s,r,m) = tf(1,[dist, zeros(1,sample_delay)],1/Fs);  
            
        end
    end
end

end

