// Here we place the function which communicates all data over serial with python

// Single value printout
// void printAdcValue(int value) {
//     //https://forum.arduino.cc/index.php?topic=448426.0
//     Serial.print(-300); // To freeze the lower limit
//     Serial.print(" ");
//     Serial.print(700); // To freeze the upper limit
//     Serial.print(" ");
//     if (averaging)
//     {
//         averagedAmplitude = ((float)value + numAveragedSamples * averagedAmplitude) / (numAveragedSamples + 1);
//         integratedAmplitude += bandpassEMA(value);
//         maxAmplitude = max(value, maxAmplitude);
//         minAmplitude = min(value, minAmplitude);
//     }
//     Serial.println(value);
//     Serial.println(" ");
//     Serial.println(bandpassEMA(value));
// }
