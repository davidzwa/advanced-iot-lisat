# Swarm Got Talent

## Folders
- `src`: code for ESP, MSP and Python (named accordingly)
- `src/MSP432LISAT`: MSP LISAT project and TI-RTOS library dependency project
- `src/ESP8266LISAT`: flexible ESP8266 .ino code for mic measurements and MSP432 MQTT-over-Serial bridge
- `src/python_list`: python multi-functional command-line-interface (MQTT, Serial, Terminal asynchronous CLI)
- `tools/Matlab_tdoa_simulation`: microphone audio localization simulation based on TDOA (Matlab)
- `tools/argos3-lisat`: (TBD) argos simulation (currently in root `\argos3-lisat`, will move)
- `tools/ccs10-projects`: backup projects, graph settings and example projects from [TI Resource explorer](dev.ti/tirex) which lead up to current MSP432 code in `src/MSP432LISAT` and subsequent experiments
- `tools/ros_docker`: small example how to up ROS with docker compose
- `tools/TDOA_direction_estimation_gen`: generator cli script for outputting C code for TDOA algorithms with 2x2 matrix inversion
- `tools/tests-python-esp8266`: small and bigger projects which lead up to extracting microphone data with Arduino UNO/ESP8266. Advice: take a look at `receive_buffer.py` in `tools/tests-python-esp8266/python-audio-tool` especially as this was a major step up to automating and speeding up sound pressure (RMS) measurements through Serial and logging this to .json files ready to be plotted. Also sound/tone generator scripts are to be found in `tests-python-esp8266/`.
- `documentation`: papers, datasheets, diagrams and small write-ups. To be cleaned up later.
- `data`: json RMS measurement data, and short audio recordings

## ToDo list
- [x] done
- [ ] to be done

## Notes and ideas
- Note 1

## Sources
1. source 1

### Papers

- Audio-based localization for swarms of micro air vehicles [(link)](https://ieeexplore.ieee.org/document/6907551)
  - TDOA using four microphones packed tightly together
  - Single 'beacon' unit which transmits chirps for others to localize
  - Extended Kalman filter for fusing sound information with sensory movement information
- Sound based localization and identification in industrial environments [(link)](https://ieeexplore.ieee.org/abstract/document/8217063)
  - TDOA Source direction-only localization using two microphones
  - Beamforming to let microphones listen in specific direction
