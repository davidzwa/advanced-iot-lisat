function U = Cutting_the_plane_estim(env,TOA)
U = zeros(env.num_robots, 2);

for Rob = 1:env.num_robots
    
    u = [0,0];

    for i = 1:env.num_mics
        for j = i+1:env.num_mics
            temp = env.mics_relative(j,:) - env.mics_relative(i,:);
            vect = temp/norm(temp);
            
            if (TOA(i) > TOA(j))
                u = u + vect;
            else
                u = u - vect;
            end
        end
    end
    
    U(Rob,:) = u/norm(u);
    
end
    
end

