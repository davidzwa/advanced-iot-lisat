function tdoa = determine_tdoa(env,y,Fs)
% compute time differences of arival compaired to mic 1

tdoa = zeros(env.num_robots, env.num_mics-1);

for r = 1:env.num_robots
    
    for m = 2:env.num_mics
        [c, lag] = xcorr(squeeze(y(r,1,:)), squeeze(y(r,m,:)));
        [~, idx] = max(c);
        tdoa(r,m-1) = lag(idx)/Fs;
    end
    
end

end

