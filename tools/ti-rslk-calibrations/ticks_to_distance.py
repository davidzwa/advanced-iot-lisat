from math import pi
import numpy as np

# Odometer setup
gear_ratio = 120        # Motor gearbox
disc_line_count = 32  # Tacho disc lines
lines_per_rev = disc_line_count * gear_ratio  # Lines per revolution

# Wheel geometry 
wheel_diam = 70         # mm
circumference = pi * wheel_diam

# Intermediate result
distance_per_line_rising_edge = circumference / lines_per_rev
print('circ:', circumference)
print('dist_per_interrupt:', distance_per_line_rising_edge)

# Clock setup
aclk = 32765            # Interrupt ticks
speeds = np.linspace(50, 600, 500)  # mm/s
# isrs_per_sec = 
