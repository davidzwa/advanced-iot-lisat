from playsound import playsound
from typing import List
import wave
import serial
import time
import traceback
import string
import os
import random
import numpy as np
import matplotlib.pyplot as plt
import json
import datetime
from model.espdata import EspData
from model.dataset import DataSet
import dataclasses
import simpleaudio as sa
import os
import re
import json
import glob
import statistics


folder_name = 'output'


def find_numbers(input_string: str):
    rr = re.findall(
        "[-+]?[.]?[\d]+(?:,\d\d\d)*[\.]?\d*(?:[eE][-+]?\d+)?", input_string)
    if rr and len(rr) > 1:
        return rr[-1]


def load_jsonfile(filename):
    with open(filename) as file:
        return json.load(file)


if __name__ == '__main__':
    print(os.getcwd())
    scanned_files = [f for f in glob.glob(
        "./data/" + folder_name + "/tdifftests/dataset-2020*") if os.path.isfile(f)]
    print('Number of scanned files:', len(scanned_files))

    dataset_directions = np.array([])
    tdiff_arrays = []
    for filename in scanned_files:
        # Parse file and extract the static direction of the experiments in the set
        filename_split = os.path.splitext(filename)[0]
        jsonBlob = load_jsonfile(filename)
        dataset = DataSet(**jsonBlob)
        direction = dataset.direction
        dataset_directions = np.append(dataset_directions, direction)

        # Analyse time-differences of 2 mics
        timediff_array = np.array([])
        for entry in dataset.espDataSets:
            jsonModel = EspData(**entry)
            tdiff = jsonModel.mic1LTimeUs - jsonModel.mic2MTimeUs
            timediff_array = np.append(
                timediff_array, tdiff)
        tdiff_arrays.append(timediff_array)

        average_tdiff = np.average(timediff_array)
        print('file read',
              'direction', direction,
              'rms', average_tdiff,
              'items', len(jsonBlob))

    index_sort = dataset_directions.argsort()
    sorted_tdiff_arrays = []
    for index in index_sort:
        sorted_tdiff_arrays.append(tdiff_arrays[index])

    fig1, ax1 = plt.subplots()
    ax1.set_title('Angle vs Time-difference between two microphones')
    plt.ylabel('Time-difference (us)')
    ax1.boxplot(sorted_tdiff_arrays, positions=dataset_directions[index_sort])
    plt.show()
