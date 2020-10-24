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


print(os.getcwd())
scanned_files = [f for f in glob.glob(
    "./" + folder_name + "/distancetests/dataset-2020*") if os.path.isfile(f)]
print('Number of scanned files:', len(scanned_files))

distances = np.array([])
rms_arrays = []
# rms_arrays = rms_arrays.transpose()
for filename in scanned_files:
    filename_split = os.path.splitext(filename)[0]
    distance_indicator = int(find_numbers(filename_split))

    jsonBlob = load_jsonfile(filename)

    rms_array = np.array([])
    for entry in jsonBlob:
        jsonModel = EspData(**entry)
        rms_array = np.append(rms_array, jsonModel.rms)
    result = np.average(rms_array)
    rms_arrays.append(rms_array)
    distances = np.append(distances, distance_indicator)
    print('file read', 'dist', distance_indicator, 'rms', result, 'items', len(jsonBlob))

index_sort = distances.argsort()
sorted_rms_arrays = []
for index in index_sort:
    sorted_rms_arrays.append(rms_arrays[index])

fig1, ax1 = plt.subplots()
ax1.set_title('RMS vs Distance [X*0.25m] (10+ measurements per distance)')
ax1.boxplot(sorted_rms_arrays)
plt.show()
