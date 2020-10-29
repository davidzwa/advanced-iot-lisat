import numpy as np
import matplotlib.pyplot as plt
from math import pi
# Wheel
wheel_diam = 70 # mm
circumference = pi * wheel_diam

# Motor gearbox
gear_ratio = 120
# Tacho disc
disc_line_ratio = 32
# Lines per revolution
lines_per_rev = disc_line_ratio * gear_ratio

print(circumference)
print(lines_per_rev)

distance_per_line_rising_edge = circumference/lines_per_rev
print(distance_per_line_rising_edge)

# Interrupt ticks
aclk = 32765

speeds = np.linspace(3, 150, 150)  # mm/s
isrs_per_second = speeds / distance_per_line_rising_edge 
plt.stem(speeds, aclk/isrs_per_second)

plt.xlabel("Speed [mm/s]")
plt.ylabel("Diff. Ticks per interrupt")
plt.show()
