# Swarm Got Talent

## Folders

- `ccs10-projects`: actual MSP432 code for this project
- `documentation`: papers and small write-ups
- `backup`: old folders with valuable code
- `Matlab_tdoa_simulation`: TDOA simulation + algorithm validation
- `tests-python-eso8266`: secondary method of recording audio with Python and ESP8266 (quick mic/hardware tests)

## ToDo list (tentatively sorted by priority)
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
