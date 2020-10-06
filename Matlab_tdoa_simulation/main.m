clearvars
close all

c = 345;    %m/s, speed of sound
%%
% Field setup
field = [-2, 2, -2, 2];
walls = false;   % for echos, not yet implemented

% source setup 
sources = [0,-1];    % [x,y] in meters

% Robot setup
%robots = [0,0,0];
robots = [ -2,-2,0; -2,0,0; -2,2,0; 0,-2,0; 0,2,0; 2,-2,0; 2,0,0; 2,2,0; 0.5, 0,0];
%robots =[zeros(20,2), (1:20)'/20*2*pi];    % [x,y,phi] with x and y in meters and phi in radians
%mic = [-1,0; 1,0; 0,0; 0,-1; 0,1];    %locations of the microfons relative to the robot

N_mic = 4;
%mic = [-2, -2; 2,-2;-2,2];
mic = .1*[cos(2*pi/N_mic*(0:N_mic-1)'), sin(2*pi/N_mic*(0:N_mic-1)')];
%mic = [0,0; mic];
%mic = [0.20,0; 0, 0.10; 0, -0.10];

% Generating environment
env = gen_environment(sources, robots, mic, field, walls);
fig = plot_env(env);

%%
% sound setup

Fs_send = 1000e3;      % sampeling frequency of the signal 
[u, t] = gen_sine( 400,0.03, Fs_send);
%[u, t] = gen_imp( 0.01,0.03, Fs_send);
%[u, t] = gen_chirp(200, 400, 0.1, 0.15, Fs_send);

% soundsc(u, Fs_send);
%%
%generate transfer functions for propogation[source, robot, mic]

env = gen_transfer(env, Fs_send, c);

%% 
% record outputs

Fs_record = 10e3;      % sampeling frequency of the recording
[y, yt] = Run_sim(env, u, t, Fs_record, Fs_send);
plot_result(env, u, t, y, yt);

%%
% determine TDOA

tdoa = determine_tdoa(env,y,Fs_record);

%%
% determine location
%est_loc = estimate_location(env, c,tdoa);
%plot_estimate(env, est_loc, fig);
%est_loc2 = estimate_location2(env, c, tdoa);

est_dir = direction_estimation_Valin(env, c, tdoa);
plot_est_dir(env, est_dir, fig);

%% look at RMSE
% error = sqrt( (env.sources(1) - est_loc(:,1)).^2 ...
%              +(env.sources(2) - est_loc(:,2)).^2);
% figure
% plot(env.robots(:,3),error);


