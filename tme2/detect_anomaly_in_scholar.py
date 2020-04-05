import numpy as np
import matplotlib.pyplot as plt
import matplotlib.colors as colors
from matplotlib.patches import Ellipse
import pandas as pd

filename = 'results/scholar-k-decomp.txt'
file = open(filename)
lines = file.readlines()
file.close()
lines = lines[:-3]
indices = [int(l.split()[1]) for l in lines]
coreness = [int(l.split()[2]) for l in lines]

coreness_temp = np.zeros(len(coreness), dtype=int)
for index, value in zip(indices, coreness):
    coreness_temp[index-1] = value
coreness = coreness_temp


filename = 'results/scholarDegrees.txt'
file = open(filename)
lines = file.readlines()
file.close()
degrees = np.array([sum([int(x) for x in l.split()]) for l in lines], dtype=int)

count = {}

for degree_, coreness_ in zip(degrees, coreness):
    if (degree_, coreness_) in count:
        count[(degree_, coreness_)] += 1
    else:
        count[(degree_, coreness_)] = 1

degree_arr = np.zeros(len(count))
coreness_arr = np.zeros(len(count))
count_arr = np.zeros(len(count))

for idx, key in enumerate(count):
    degree_arr[idx] = key[0]
    coreness_arr[idx] = key[1]
    count_arr[idx] = count[key]

fig = plt.figure()

sc = plt.scatter(degree_arr, coreness_arr, c=count_arr, cmap='jet', norm=colors.LogNorm())
plt.colorbar(sc)
plt.title('''Diagramme degré-coreness pour le graphe "Google Scholar"''')
plt.xlabel("Degré")
plt.ylabel("Coreness")

ellipses = [Ellipse([23,14], 20, 1), Ellipse([65.5,11], 15, 0.7)]
for e in ellipses:
    plt.gca().add_artist(e)
    e.set_clip_box(plt.gca().bbox)
    e.set_alpha(1.0)
    e.set_edgecolor('r')
    e.set_facecolor('none')
    e.set_linewidth(2)

fig.savefig('results/scholar-coreness-plot.png')
fig.show()




