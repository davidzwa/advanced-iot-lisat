function [env] = gen_environment(sources, robots, mic_setting, field, walls)

% orientation in radians, pointing north rotating anti clockwise
% relative orientation of the microfons on a robot
%   [x1,y1;x2,y2;etc]

if size(sources,2) ~= 2
    error("sources should contain the 2D corodinates of the sources in a colum [x1,y1; x2,y2; etd]")
end
if size(robots,2) ~= 3
    error("robots should contain the 2D corodinates of the robots and its orientation in a colum [x1,y1,o1; x2,y2,o2; etd]")
end
if size(mic_setting,2) ~= 2
    error("mics should contain the 2D corodinates of the mics in a colum [x1,y1; x2,y2; etd]")
end
if any(size(field) ~= [1,4])
    error("field sould contain the size of the field as a row vector [xmin, xmax, ymin, ymax]")
end
if ~islogical(walls)
    error("walls must be a logical value")
end


if any(sources(:,1) < field(1)) || any(sources(:,1) > field(2)) || any(sources(:,2) < field(3)) || any(sources(:,2) > field(4))
    error("sources outside of field")
end
if any(robots(:,1) < field(1)) || any(robots(:,1) > field(2)) || any(robots(:,2) < field(3)) || any(robots(:,2) > field(4))
    error("robots outside of field")
end


env = struct;

env.field = field;
env.walls = walls;

env.num_sources = size(sources,1);
env.sources = sources;

env.num_robots = size(robots,1);
env.robots = robots;

env.num_mics = size(mic_setting,1);
env.mics_relative = mic_setting;
env.mics = zeros(env.num_mics,2,env.num_robots);

for i = 1:env.num_robots
    for j = 1:env.num_mics
        
        theta = env.robots(i,3);
        R = [cos(theta) -sin(theta); sin(theta) cos(theta)]';
        
        env.mics(j,:,i) = mic_setting(j,:)*R + env.robots(i,1:2);
        
    end
end


end

