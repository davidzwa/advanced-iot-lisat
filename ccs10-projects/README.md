Download code composer studio 10 and plug in your MSP432 board.

Actual code:
- The `mic3-trigger-tdoa` is the most simplistic piece of code to take some mic samples with a timer+ADC setup. Very rough, but most important project in this folder

Reference projects:
- The `reference-files` folder contains raw code snippets used during development.
- The `analogmic-fir-fft` folder contains working code for sampling 1 mic on pin 4_3. Using debug you can see the output vector and plot the graphs.
- The `analogmic3-fir-fft` folder contains non-working code to attempt sampling 4 mics at the same time. Abandoned due to another chosen approach (using trigger based mic + 1 recording mic)
