function [est_loc] = single_source_localisation(env)

c = 345;    %m/s, speed of sound

%% generating beacon sound
Fs_send = 1000e3;      % sampeling frequency of the signal 
[u, t] = gen_sine( 400,0.03, Fs_send);

%% generate transfer functions
env = gen_transfer(env, Fs_send, c);

%% record outputs
Fs_record = 10e3;      % sampeling frequency of the recording
[y, yt] = Run_sim(env, u, t, Fs_record, Fs_send);

%% determine TDOA
tdoa = determine_tdoa(env,y,Fs_record);

%% estimate direction
est_loc = direction_estimation_Valin(env, c, tdoa);
end

