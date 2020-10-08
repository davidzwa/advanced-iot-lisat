from typing import List
from dataclasses import dataclass
import numpy as np

@dataclass
class EspData(object):
    algoTdoaDir1: float = None
    algoTdoaDir2: float = None
    algoTdoaDir3: float = None
    samplingRate: int = None
    soundData: np.ndarray = None
    mic1LTimeUs: int = None
    mic2MTimeUs: int = None
    mic3RTimeUs: int = None
