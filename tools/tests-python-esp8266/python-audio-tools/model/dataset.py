from typing import List
from dataclasses import dataclass
import numpy as np
from model.espdata import EspData

@dataclass
class DataSet(object):
    espDataSets: List[EspData] = None
    direction: int = None
