%%generating training data

Num_train = 5000;

X_train = zeros(Num_train, 4);  % [tdoa12, tdoa13, tdoa14, orientation]
Y_train = zeros(Num_train, 2);  % [x,y]

parfor i= 1: Num_train
    
    [X_train(i,:), Y_train(i,:)] = DL_data_gathering();
    disp("training: "+i)
    
end

T = array2table([X_train, Y_train],'VariableNames',...
    ["TDOA_12", "TDOA_13", "TDOA_14", "Orientation","x", "y"]);

writetable(T,"Training_data_5x5_4mic_30cm.csv")

%% Generate validation data

Num_train = 1000;

X_train = zeros(Num_train, 4);  % [tdoa12, tdoa13, tdoa14, orientation]
Y_train = zeros(Num_train, 2);  % [x,y]

parfor i= 1: Num_train
    
    [X_train(i,:), Y_train(i,:)] = DL_data_gathering();
    disp("validation: "+i)
    
end

T = array2table([X_train, Y_train],'VariableNames',...
    ["TDOA_12", "TDOA_13", "TDOA_14", "Orientation","x", "y"]);

writetable(T,"Validation_data_5x5_4mic_30cm.csv")