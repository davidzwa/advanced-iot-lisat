function y = Run_sim(env, u, t)



for r = 1:env.num_robots
    for m = 1:env.num_mics
        out = 0;
        for s = 1:env.num_sources
            
            out = out + lsim(env.trans(s,r,m),u,t);
            
        end
        y(r,m,:) = out;
    end
end

end

