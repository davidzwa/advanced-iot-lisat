clear vars

configuration = "5x5_4mic_30cm";

train_file = "Training_data_" + configuration + ".csv";
validation_file = "Validation_data_" + configuration + ".csv";

opts = detectImportOptions(train_file);
opts.SelectedVariableNames = opts.VariableNames(1:end-2);
XTrain = readmatrix(train_file, opts);
XValidation = readmatrix(validation_file, opts);
num_features = length(opts.VariableNames(1:end-2));

opts.SelectedVariableNames = opts.VariableNames(end-1:end);
YTrain = readmatrix(train_file, opts);
YValidation = readmatrix(validation_file, opts);


layers = [
    featureInputLayer(num_features)
    fullyConnectedLayer(50)
    batchNormalizationLayer
    fullyConnectedLayer(20)
    batchNormalizationLayer
    fullyConnectedLayer(20)
    batchNormalizationLayer
    fullyConnectedLayer(20)
    batchNormalizationLayer
    fullyConnectedLayer(20)
    batchNormalizationLayer
    clippedReluLayer(5)
    fullyConnectedLayer(2)
    regressionLayer];

miniBatchSize  = 128;
validationFrequency = floor(numel(YTrain)/miniBatchSize);
options = trainingOptions('sgdm', ...
    'MiniBatchSize',miniBatchSize, ...
    'MaxEpochs',30, ...
    'InitialLearnRate',1e-3, ...
    'LearnRateSchedule','piecewise', ...
    'LearnRateDropFactor',0.1, ...
    'LearnRateDropPeriod',20, ...
    'Shuffle','every-epoch', ...
    'ValidationData',{XValidation,YValidation}, ...
    'ValidationFrequency',validationFrequency, ...
    'Plots','training-progress', ...
    'Verbose',false);

net = trainNetwork(XTrain,YTrain,layers,options);

save("network"+ configuration, net)