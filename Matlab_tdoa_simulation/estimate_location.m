function est_location = estimate_location(env, c, tdoa)

%based on WLS algorithm

est_location = zeros(env.num_robots, 2);

for Rob = 1:env.num_robots
    
    r = c * abs(tdoa(Rob,:)');  % range differences
    
    A = [  env.mics_relative(2:end,1) - env.mics_relative(1,1), ... 
            env.mics_relative(2:end,2) - env.mics_relative(1,2), r];
    
    theta = 0.5 * ((env.mics_relative(2:end,1) - env.mics_relative(1,1)).^2 +...
                (env.mics_relative(2:end,2) - env.mics_relative(1,2)).^2 - r.^2);
    
    W = eye(env.num_mics-1);
    
    est_rel_loc = (A'*W* A)\A'*W*theta;
    est_relative_loc = [est_rel_loc(1) + env.mics_relative(1,1), est_rel_loc(2) + env.mics_relative(1,2)];   
    
    theta = env.robots(Rob,3);
    R = [cos(theta) -sin(theta); sin(theta) cos(theta)]';
    
    est_location(Rob,:) = -est_relative_loc *R;    
    
end

end

