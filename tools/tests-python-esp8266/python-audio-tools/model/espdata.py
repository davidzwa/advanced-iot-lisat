from typing import List
from dataclasses import dataclass
import numpy as np

@dataclass
class EspData(object):
    mic1LTimeUs: int = None
    mic2MTimeUs: int = None
    mic3RTimeUs: int = None

    # Valin
    algoTdoaDirX: float = None
    algoTdoaDirY: float = None
    
    # CTP
    algoCTPDirX: float = None
    algoCTPDirY: float = None

    min: int = None
    max: int = None
    rms: float = None
    samplingRate: int = None
    sampleSize: int = None
    soundData: np.ndarray = None
