from typing import List
from dataclasses import dataclass
import numpy as np
from espdata import EspData


@dataclass
class DataSet(object):
    espDataSets: List[EspData]
    positionExperimentX: int = None
    positionExperimentY: int = None
    metadata: string = ""


# loopen a.h.v. sets [...10*(X,Y)]
    # input:
    # -- X
    # -- Y
    # -- info

    # experiment: save
    # loopen X times record  ==> for each in list met EspData's => save to json in format DataSet
