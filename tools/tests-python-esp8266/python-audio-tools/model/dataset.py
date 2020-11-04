from typing import List
from dataclasses import dataclass
import numpy as np
from model.espdata import EspData

@dataclass
class DataSet(object):
    espDataSets: List[EspData] = None
    positionExperimentX: int = None
    positionExperimentY: int = None
    metadata: str = ""


# loopen a.h.v. sets [...10*(X,Y)]
    # input:
    # -- X
    # -- Ys
    # -- info

    # experiment: save
    # loopen X times record  ==> for each in list met EspData's => save to json in format DataSet
