clear vars
close all

c = 345;    %m/s, speed of sound
%%
% Field setup
field = [5,5];
walls = false;   % for echos, not yet implemented

% source setup 
sources = [0,0];    % [x,y] in meters

% Robot setup
robots = [2,3,pi; 5,1,0];    % [x,y,phi] with x and y in meters and phi in radians
mic = [0.5,0; -0.35,0.35; -0.35,-0.35; 0,0];    %locations of the microfons relative to the robot

% Generating environment
env = gen_environment(sources, robots, mic, field, walls);
fig = plot_env(env);

%%
% sound setup

Fs = 100e3;      % sampeling frequency of the signal 
[u, t] = gen_sine(0.03, 100, Fs);
%[u, t] = gen_imp(1, 0.5, Fs);

%%
%generate transfer functions [source, robot, mic]

env = gen_transfer(env, Fs, c);

%% 
% record outputs

y = Run_sim(env, u, t);
plot_result(env, y, u, t);

%%
% determine TDOA

tdoa = determine_tdoa(env,y,t);

%%
% determine location
est_loc = estimate_location(env, c,tdoa);
plot_estimate(env, est_loc, fig);


