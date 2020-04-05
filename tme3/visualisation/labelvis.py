# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import networkx as nx

#Create graph
G=nx.Graph()

#Open graph and add edges
f = open("../RandomGraphs/graph_08_01.txt","r")
lines = f.readlines()

for line in lines:
    edge = [int(s) for s in line.split() if s.isdigit()]
    G.add_edge(edge[0],edge[1])
f.close()
    
#Add labels and color according to label propagation results
colors_available = ['red','blue','green','orange'] #Only 4 colors for this code
color_map = []
for node in G:
    color_map.append("")
labels_found = []

f = open("../Results/graph_08_01.txt","r")
lines = f.readlines()

for line in lines:
    contents = [int(s) for s in line.split() if s.isdigit()]
    node = contents[0]
    label = contents[1]
    
    if label not in labels_found:
        labels_found.append(label)
    
    for i in range(0,len(labels_found)):
        if (labels_found[i] == label):
            color_map[node] = colors_available[i]
    
#pos = nx.circular_layout(G)
nx.draw_circular(G, node_color=color_map, node_size = 20, width = 0.005)


plt.savefig("LabeledGraphs/graph_08_01.png") # save as png
plt.show() # display
