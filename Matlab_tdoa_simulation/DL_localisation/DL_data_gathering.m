function [X,Y] = DL_data_gathering()

addpath('C:\Users\caspe\Documents\TU_Delft\Master\Y2\Q5\advanced_IoT\advanced-iot-lisat\Matlab_tdoa_simulation');

c = 345;    %m/s, speed of sound
%%
% Field setup
field = [5,5];
walls = false;   % for echos, not yet implemented

% source setup 
sources = [0,0];    % [x,y] in meters

% Robot setup
robots = [field(1)*rand(),field(2)*rand(), 2*pi*rand];    % [x,y,phi] with x and y in meters and phi in radians
%mic = [-1,0; 1,0; 0,0; 0,-1; 0,1];    %locations of the microfons relative to the robot


Y = [robots(1), robots(2)];

N_mic = 4;
mic = 0.3*[cos(2*pi/N_mic*(0:N_mic-1)'), sin(2*pi/N_mic*(0:N_mic-1)')];
% mic = [mic;0,0];

% Generating environment
env = gen_environment(sources, robots, mic, field, walls);

%%
% sound setup

Fs_send = 1000e3;      % sampeling frequency of the signal 
%[u, t] = gen_sine( 400,0.03, Fs_send);
[u, t] = gen_imp( 0.01,0.1, Fs_send);
%[u, t] = gen_chirp(100, 500, 0.3, 0.5, Fs_send);

% soundsc(u, Fs_send);
%%
%generate transfer functions for propogation[source, robot, mic]

env = gen_transfer(env, Fs_send, c);

%% 
% record outputs

Fs_record = 10e3;      % sampeling frequency of the recording
[y, yt] = Run_sim(env, u, t, Fs_record, Fs_send);

%%
% determine TDOA

tdoa = determine_tdoa(env,y,Fs_record);

X = [tdoa(1,:), robots(3)];

end

