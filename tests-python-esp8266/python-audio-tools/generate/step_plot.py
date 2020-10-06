## When the input signal length matches the buffer (or delay) length we get a periodical output signal
# let the input be a finite-support sequence x[n] = delta_1 + 2 * delta_2 + 3 * delta_3
import numpy as np
import matplotlib.pyplot as plt
x = [1, 2, 3, 0, 0, 0, 0, 0, 0, 0]  # zero otherwise
y = np.zeros(10)
delay = [0, 0, 0]
for i in range(10):

    y[i] = x[i] + delay[0]
    # This is a clock driven system
    # when the clock ticks, the delay cells shift their current content
    # then the last memory cell is updated
    for j in range(len(delay)-1):
        delay[j] = delay[j+1]
    delay[-1] = y[i]
#     print(delay)
fig = plt.figure()
plt.stem(y, use_line_collection=True)
plt.show()
