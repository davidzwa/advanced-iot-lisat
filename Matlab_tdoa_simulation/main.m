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
mic = [1,0; -0.7,0.7; -0.7,-0.7; 0,0];    %locations of the microfons relative to the robot

% Generating environment
env = gen_environment(sources, robots, mic, field, walls);
fig = plot_env(env);

%%
% sound setup

Fs_send = 1000e3;      % sampeling frequency of the signal 
%[u, t] = gen_sine( 400,0.03, Fs_send);
%[u, t] = gen_imp( 0.01,0.1, Fs_send);
[u, t] = gen_chirp(100, 500, 0.4, 1, Fs_send);

% soundsc(u, Fs_send);
%%
%generate transfer functions for propogation[source, robot, mic]

env = gen_transfer(env, Fs_send, c);

%% 
% record outputs

Fs_record = 300e3;      % sampeling frequency of the recording
[y, yt] = Run_sim(env, u, t, Fs_record, Fs_send);
plot_result(env, u, t, y, yt);

%%
% determine TDOA

tdoa = determine_tdoa(env,y,yt);

%%
% determine location
est_loc = estimate_location(env, c,tdoa);
plot_estimate(env, est_loc, fig);


