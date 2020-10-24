import os
import re
import json
import glob
import statistics
import math
from statistics import stdev
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.collections import LineCollection
from Model.ParsedData import JsonData, DataEntry, PositionData

filename_session1 = './Intermediate/session1/data_filled.json'
filename_session2 = './Intermediate/session2/data2_filled.json'
filename_session3 = './Intermediate/session3/data3_filled.json'

filename = filename_session3
with open(filename) as file:
    jsonDict = json.load(file)

jsonData = JsonData(jsonDict)

xVector = np.array([])
yVector = np.array([])
zVector = np.array([])
iVector = np.array([])

rssiVectorLow = np.array([])
distVectorLow = np.array([])
rssiVectorWall1 = np.array([])
distVectorWall1 = np.array([])
rssiVectorWall2 = np.array([])
distVectorWall2 = np.array([])

stdevs = list()
# print(jsonData.data)
for entry in jsonData.data:
    parent = DataEntry(**entry)
    for child in parent.children:
        childSpec = PositionData(**child)
        set = parent.index
        pos = (childSpec.X, childSpec.Y, childSpec.Z)
        subset = parent.original_data[childSpec.begin_index: childSpec.end_index]

        xVector = np.append(xVector, pos[0])
        yVector = np.append(yVector, pos[1])
        zVector = np.append(zVector, pos[2])
        iVector = np.append(iVector, np.average(subset))

        dist = math.sqrt(pos[0] ** 2 + pos[1] ** 2 + pos[2] ** 2)

        if parent.behind_wall is None or parent.behind_wall is 0:
            distVectorLow = np.append(distVectorLow, dist)
            rssiVectorLow = np.append(rssiVectorLow, np.average(subset))
        elif parent.behind_wall is 1:
            distVectorWall1 = np.append(distVectorWall1, dist)
            rssiVectorWall1 = np.append(rssiVectorWall1, np.average(subset))
        else:
            distVectorWall2 = np.append(distVectorWall2, dist)
            rssiVectorWall2 = np.append(rssiVectorWall2, np.average(subset))

        stdevs.append(stdev(subset))
        # if stdev(subset) > 4:
        #     print("\nIndex",
        #           set)
        #     print(
        #         "Std-dev (>2.0),",
        #         stdev(subset))
        #     print(
        #         'Coord',
        #         pos)
        #     print(
        #         'Avg',
        #         sum(subset) / len(subset))
        #     print(
        #         'element count:',
        #         len(subset))
        #     print(
        #         'subset:',
        #         subset)

        # print(parent.average, len(subset), stdev(subset))

print('max stdev', max(stdevs))
print('min stdev', min(stdevs))
print('avg stdev', np.average(stdevs))
def polyFit(x, y, axis, color):
    z = np.polyfit(x, y, 1)
    p = np.poly1d(z)
    axis.plot(x, p(x), color)


# Plot 2D rssi - distance graph - low/mid/high
fig2D = plt.figure()
ax2D = plt.axes()
indexSortLow = distVectorLow.argsort()
indexSortWall1 = distVectorWall1.argsort()
indexSortWall2 = distVectorWall2.argsort()


def polyFitLine(plot_axis, distance_array, data_array, color, label):
    try:
        plot_axis.scatter(distance_array, data_array,
                          c=color, label=label)
        polyFit(distance_array, data_array, plot_axis, color=color + '--')
    except Exception as e:
        print(e)
        pass


ax2D.set_xlabel('Euclidean distance [m]')
ax2D.set_ylabel('RSSI [dBm]')

# Try to fit a line through eucl-distance vs RSSI scatterpoints
polyFitLine(ax2D,
            distVectorLow[indexSortLow],
            rssiVectorLow[indexSortLow], 'r', 'No walls')
polyFitLine(ax2D,
            distVectorWall1[indexSortWall1],
            rssiVectorWall1[indexSortWall1], 'g', '1 wall')
polyFitLine(ax2D,
            distVectorWall2[indexSortWall2],
            rssiVectorWall2[indexSortWall2], 'b', '2 walls')

handles, labels = ax2D.get_legend_handles_labels()
ax2D.legend(handles[::-1], labels[::-1])

def plot_line(axis, x1, y1, x2, y2, color):
    xline = np.array([x1, x2])
    yline = np.array([y1, y2])
    zline = np.zeros([len(xline)])
    ax.plot3D(xline, yline, zline, c=color)


def plot_square(axis, x1, y1, width, height, color, connect=True):
    if connect:
        xline = np.array([x1, x1+width, x1+width, x1, x1])
        yline = np.array([y1, y1, y1 + height, y1 + height, y1])
    else:
        xline = np.array([x1, x1+width, x1+width, x1])
        yline = np.array([y1, y1, y1 + height, y1 + height])
    zline = np.zeros([len(xline)])
    ax.plot3D(xline, yline, zline, c=color)


# Plot 3D scatter point for average of each RSSI subset
fig = plt.figure()
ax = plt.axes(projection='3d')
ax.scatter3D(xVector, yVector, zVector, c=iVector,
             cmap='summer', edgecolors='gray', s=150)
# Plot 'walls' of experiment on the floor
if filename == filename_session2:
    # Plot Access Point location
    ax.scatter3D(0, 0, 0.6, s=100)

    # EWI Coffee Toilet Hall Room
    plot_square(ax, -2.5, -0.2, 4, 6.2, 'black')
    plot_square(ax, -15, 6, 22, 3, 'black')
    plot_square(ax, -4.1, 9, 1.1, 3, 'black')  # Coffee hallway
    plot_square(ax, -4.1, 12, 3.6, 3.1, 'black')
    plot_square(ax, -2.9, 9.1, 2.2, 2.8, 'gray')  # Service elevator
    # Other elevators (cheated width)
    plot_square(ax, -4.2, 9.1, -8.10, 2.5, 'gray')
    plot_square(ax, -12.4, 9, -1.1, 3.6, 'black')  # Toilet hallway
    plot_square(ax, -12.4, 12.6, -1.1, 3, 'black', False)  # Toilet part
    plot_square(ax, -12.4, 12.6, 6.1, 3, 'black', False)  # Toilet hallway
    # Door of small room
    plot_line(ax, -0.75, 6.0, 0.25, 6.0, 'orange')
elif filename == filename_session1:
    # Plot Access Point location
    ax.scatter3D(0, -0.1, 0.5, s=100)

    # Interior room walls
    zline = np.array([0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0,
                      0.0,  0.0,  0.0,  0.0,  0.0,  0.0,  0.0])
    xline = np.array([2.2,  2.2, -2.0, -2.0, -2.0,  2.2,  2.2,
                      2.2,  4.0,  4.0, -2.0, -2.0,  2.0,  2.2])
    yline = np.array([-4.5, -8.0, -8.0, -1.5, -4.5, -4.5, -
                      1.5,  0.5,  0.5,  4.0,  4.0, -1.5, -1.5, -1.5])
    ax.plot3D(xline, yline, zline, 'black')
else:
    # Plot Access Point location
    ax.scatter3D(0, 0, 0.0, s=100)
    plot_square(ax, -1, 3.5, -8, 12, 'black')
    ax.text(-5, 4.5, 1, s="High containers")
    ax.text(5, 3.1, 1, s="Cars, low containers")
    plot_square(ax, 1.5, 3.0, 7, 12, 'gray')

# Setup 3D axes and view from top
ax.set_xlabel('X [m]')
ax.set_ylabel('Y [m]')
ax.set_zlabel('Z [m]')
ax.view_init(azim=0, elev=90)

# Finally show 3D plot
plt.show()
