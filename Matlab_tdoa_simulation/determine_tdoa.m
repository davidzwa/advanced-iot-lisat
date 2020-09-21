function tdoa = determine_tdoa(env,y,t)

tdoa = zeros(env.num_robots, env.num_mics-1);



for r = 1:env.num_robots
    [~, t1] = findpeaks(squeeze(y(r,1,:)),t,'NPeaks', 1);
    
    for m = 2:env.num_mics
        [~, ti] = findpeaks(squeeze(y(r,m,:)),t,'NPeaks', 1);
        tdoa(r,m-1) = t1 - ti;
    end
    
end

end

