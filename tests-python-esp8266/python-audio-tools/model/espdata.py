from typing import List
from dataclasses import dataclass
import numpy as np

@dataclass
class EspData(object):
    samplingRate: int = None
    soundData: np.ndarray = None
    mic1LTimeUs: int = None
    mic2MTimeUs: int = None
    mic3RTimeUs: int = None
