# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import networkx as nx

#Create graph
G=nx.Graph()

#Open graph and add edges
f = open("../RandomGraphs/graph_1_0001.txt","r")
lines = f.readlines()

for line in lines:
    edge = [int(s) for s in line.split() if s.isdigit()]
    G.add_edge(edge[0],edge[1])
f.close()
    
    
#pos = nx.circular_layout(G)
nx.draw(G, node_size = 20, width = 0.01)

plt.savefig("Graphs/graph_1_0001.png") # save as png
plt.show() # display

