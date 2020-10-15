clearvars
close all
addpath("..")

%% setup environment
field = 2*[-1, 1, -1, 1];
walls = false;   % for echos, not yet implemented

% source setup
sources_per_ring = 2000;
ring_distances = 0.11:0.01:1;
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

N_mic = 7;
mic = 0.1*[cos(2*pi/N_mic*(0:N_mic-1)'), sin(2*pi/N_mic*(0:N_mic-1)')];
% mic = [0,0; mic];
%mic = [0,5; 5,5; 5,0; 0,0];
%mic = [0.05,0;0,0.1;0,-0.1;-0.1,0];

est_loc_Valin = zeros(sources_per_ring * length(ring_distances),2);
est_loc_cut = zeros(sources_per_ring * length(ring_distances),2);

err_Valin = zeros(sources_per_ring * length(ring_distances),2);
err_cut = zeros(sources_per_ring * length(ring_distances),2);

parfor i = 1:sources_per_ring * length(ring_distances)
    
    %% Generating environment
    env = gen_environment(sources(i,:), robots, mic, field, walls);
    
    
    %% determine TOA and TDOA (in micoseconds)
    
    TOA  = zeros(env.num_mics,1);
    TDOA = zeros(env.num_mics-1,1);
    c = 343;    %m/s
    Fs = 1000000;
    sigma = 0.0;     %Standard deviation of the jitter in TOA in seconds
    
    for j = 1:env.num_mics
        TOA(j) = norm(env.sources(1,:) - env.mics(j,:))/c;
        
         TOA(j) = TOA(j) + random('Uniform',50e-6, 150e-6);
        
        TOA(j) = round(TOA(j)*Fs);
        
        if(j > 1)
            TDOA(j-1) = TOA(1) - TOA(j);
        end
        
    end
    
    %% Direction estimation 
    est_loc_Valin(i,:) = direction_estimation_Valin(env, c, TDOA');
    
    est_loc_cut(i,:) = Cutting_the_plane_estim(env, TOA');
    
    
    
    %% error determination
    err_Valin(i,:) = error_determination(env, est_loc_Valin(i,:));
    
    err_cut(i,:) = error_determination(env, est_loc_cut(i,:));
    
    disp(i)
    
end

env = gen_environment(sources, robots, mic, field, walls);

%plot_est(sources_per_ring, ring_distances, est_loc_Valin, env);

plot_err(sources_per_ring, ring_distances, err_Valin, env);
plot_err(sources_per_ring, ring_distances, err_cut, env);
