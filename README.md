# Swarm Got Talent
This project is targeted on using Audio to enhance Swarm robots. This means we didnt take usual sensors for granted, instead we've decided to minimalize the peripherals on our robot to keep it light, understandable, repeatable and still very functional. 

An impression of the current state of the project:
<img src="https://user-images.githubusercontent.com/6005355/97089275-f9cdef00-1636-11eb-97ef-bdf1510ba561.png" width="300">

This repository requires you to be experienced with the following software/hardware:
- [Python (3.7/3.8)](https://www.python.org/downloads/release/python-370/) - serial and MQTT data processing, sound experiments and data plotting
- Python libraries numpy, matplotlib, asyncio
- [MatLab (2019+)](https://nl.mathworks.com/products/matlab.html) - audio signal TDOA simulation
- [ArgOS3](https://www.argos-sim.info/) - swarm simulation
- ESP8266 (see SDK link below) - ESP12F WiFi microcontroller based on (but not the same as) WeMos R1 D1
- ESP Arduino SDK, ESP8266WiFi library, PublishSubscriber library (MQTT)
- Code Composer Studio 10 and MSP432 SimpleLink RED (see TI RSLK software/hardware references below)

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

## ToDo Hardware & Software
- [x] Cleaned up repo
- [ ] Hardware & DIY tasks [Refer to this hardware task](https://github.com/davidzwa/advanced-iot-lisat/issues/49)
- [ ] Software tasks [Refer to this software task](https://github.com/davidzwa/advanced-iot-lisat/issues/50)

## Software sources
- [MSP432 TI-Driver Reference project](https://dev.ti.com/tirex/explore/node?devtools=MSP-EXP432P401R&node=AAJhPvFtK5knOoD6xzMhpQ__z-lQYNj__LATEST)
- [ESP8266 Arduino Documentation](https://arduino-esp8266.readthedocs.io/en/latest/) and [ESP8266 Function Overview](https://www.arduino.cc/reference/en/libraries/esp8266-framework/)
- [ESP8266 Ardiono SDK](https://github.com/esp8266/Arduino)

## Hardware sources
- [TI RSLK MAX Program](https://university.ti.com/programs/RSLK/)
- [TI RSLK MAX Chassis Kit](https://www.pololu.com/product/3670)
- [TI RSLK Chassis Board](https://www.pololu.com/product/3671/specs)
- [ROMI Chassi Kit (base plate, motors, wheels)](https://www.pololu.com/product/3500)

## DIY & 3D Print reference
- https://www.mtb3d.com/

### Papers
- Audio-based localization for swarms of micro air vehicles [(link)](https://ieeexplore.ieee.org/document/6907551)
  - TDOA using four microphones packed tightly together
  - Single 'beacon' unit which transmits chirps for others to localize
  - Extended Kalman filter for fusing sound information with sensory movement information
- Sound based localization and identification in industrial environments [(link)](https://ieeexplore.ieee.org/abstract/document/8217063)
  - TDOA Source direction-only localization using two microphones
  - Beamforming to let microphones listen in specific direction
