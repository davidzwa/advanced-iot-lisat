import numpy as np
import scipy as sy
import scipy.fftpack as syfp
import pylab as pyl

dt = 0.02071 
t = dt*np.arange(100000)             ## time at the same spacing of your data
u = np.sin(2*np.pi*t*.01)            ## Note freq=0.01 Hz

# Do FFT analysis of array
FFT = sy.fft(u)

# Getting the related frequencies
freqs = syfp.fftfreq(len(u), dt)     ## added dt, so x-axis is in meaningful units

# Create subplot windows and show plot
pyl.subplot(211)
pyl.plot(t, u)
pyl.xlabel('Time')
pyl.ylabel('Amplitude')
pyl.subplot(212)
pyl.plot(freqs, sy.log10(abs(FFT)), '.')  ## it's important to have the abs here
pyl.xlim(-.05, .05)                       ## zoom into see what's going on at the peak
pyl.show()
