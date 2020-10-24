function [U] = direction_estimation_Valin(env, c,tdoa)
% TDOA based direction estimation based on the following paper
% https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1248813&tag=1

U = zeros(env.num_robots, 2);

for Rob = 1:env.num_robots
    
    r = c * tdoa(Rob,:)';  % range differences
    
    X = [  env.mics_relative(2:end,1) - env.mics_relative(1,1), ... 
            env.mics_relative(2:end,2) - env.mics_relative(1,2)];
        
        
    u = pinv(X)*r;  
    
    U(Rob,:) = u/norm(u);
end


end

