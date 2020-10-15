clearvars
close all

%% setup environment
field = 5*[-1, 1, -1, 1];
walls = false;   % for echos, not yet implemented

% source setup 
sources_per_ring = 2000;
ring_distances = 0.11:0.1:5;
sources = zeros(sources_per_ring * length(ring_distances), 2);

for i =  1:length(ring_distances)
sources((i-1)*sources_per_ring +1:i*sources_per_ring,:) = ...
    ring_distances(i)* ...
    [cos(2*pi/sources_per_ring*(0:sources_per_ring-1)'),...
     sin(2*pi/sources_per_ring*(0:sources_per_ring-1)') ];    % [x,y] in meters
end

% Robot setup
robots = [0,0,0];
%robots = [0,0,0; 2,0,0; 4,0,0; 4,2,0; 4,4,0; 2,4,0; 0,4,0; 0,2,0; 2,2,0];
%robots =[zeros(20,2), (1:20)'/20*2*pi];    % [x,y,phi] with x and y in meters and phi in radians
%mic = [-1,0; 1,0; 0,0; 0,-1; 0,1];    %locations of the microfons relative to the robot

N_mic = 3;
mic = 0.1*[cos(2*pi/N_mic*(0:N_mic-1)'), sin(2*pi/N_mic*(0:N_mic-1)')];
% mic = [0,0; mic];
%mic = [0,5; 5,5; 5,0; 0,0];
%mic = [0,0.1;0.1,0;-0.1,0];

est_loc = zeros(sources_per_ring * length(ring_distances),2);
err = zeros(sources_per_ring * length(ring_distances),2);

parfor i = 1:sources_per_ring * length(ring_distances)
    
    %%% Generating environment
    env = gen_environment(sources(i,:), robots, mic, field, walls);


    %% simulation
    est_loc(i,:) = single_source_localisation(env);
    
    %% error determination
    err(i,:) = error_determination(env, est_loc(i,:));
    
    disp(i)
    
end

env = gen_environment(sources, robots, mic, field, walls);

plot_est(sources_per_ring, ring_distances, est_loc, env);

plot_err(sources_per_ring, ring_distances, err, env);