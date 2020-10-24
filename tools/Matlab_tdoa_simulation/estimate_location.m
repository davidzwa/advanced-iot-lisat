function est_location = estimate_location(env, c, tdoa)

%based on WLS algorithm

est_location = zeros(env.num_robots, 2);

for Rob = 1:env.num_robots
    
    r = c * tdoa(Rob,:)';  % range differences
    
    A = [  env.mics_relative(2:end,1) - env.mics_relative(1,1), ... 
            env.mics_relative(2:end,2) - env.mics_relative(1,2), r];
    
    theta = 0.5 * ((env.mics_relative(2:end,1) - env.mics_relative(1,1)).^2 +...
                (env.mics_relative(2:end,2) - env.mics_relative(1,2)).^2 - r.^2);
    
    W = eye(env.num_mics-1);
    
    est = (A'*W* A)\A'*W*theta;
    est_relative_loc = [est(1) + env.mics_relative(1,1), est(2) + env.mics_relative(1,2)];   

    
    %change from a robot centic frame to a global frame
     theta = env.robots(Rob,3);
     R = [cos(theta) -sin(theta); sin(theta) cos(theta)]';
     
     est_location(Rob,:) = est_relative_loc + env.robots(Rob,1:2);    
 
end

end

