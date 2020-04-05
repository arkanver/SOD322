import matplotlib.pyplot as plt
import argparse
import numpy as np

parser = argparse.ArgumentParser()
parser.add_argument('pagerank_results', type=str,
                    help='the common prefix of the files containing the results of the PageRank algorithm')
parser.add_argument('degrees', type=str,
                    help='the name of the file containing the in and out degress of the graph')

args = parser.parse_args()
prefix = args.pagerank_results

pagerank_results = {}
alphas = ["_0_1", "_0_15", "_0_2", "_0_5", "_0_9"]

for alpha in alphas:
    filename = prefix + alpha;
    print('Reading {}...'.format(filename))
    file = open(filename)
    lines = file.readlines()
    pagerank_results[alpha] = np.array([float(s) for s in lines])
    file.close()

print('Reading {}...'.format(args.degrees))
file = open(args.degrees)
lines = file.readlines()
in_degrees = np.zeros(len(lines))
out_degrees = np.zeros(len(lines))

for (i,l) in enumerate(lines):
    splitted_line = l.split()
    in_degrees[i] = int(splitted_line[0])
    out_degrees[i] = int(splitted_line[1])

file.close()

print('Drawing and saving plots...')

plt.figure()
plt.plot(pagerank_results['_0_15'], in_degrees,'+')
plt.xlabel(r'PageRank with $\alpha = 0.15$')
plt.ylabel('In-degree')
plt.title(r'PageRank with $\alpha = 0.15$ against In-degree')
plt.savefig("results/0_15_vs_in_degree.png")

plt.figure()
plt.plot(pagerank_results['_0_15'], out_degrees,'+')
plt.xlabel(r'PageRank with $\alpha = 0.15$')
plt.ylabel('Out-degree')
plt.title(r'PageRank with $\alpha = 0.15$ against Out-degree')
plt.savefig("results/0_15_vs_out_degree.png")

plt.figure()
plt.loglog(pagerank_results['_0_15'], pagerank_results['_0_1'],'o')
plt.xlabel(r'PageRank with $\alpha = 0.15$')
plt.ylabel(r'PageRank with $\alpha = 0.1$')
plt.title(r'PageRank with $\alpha = 0.15$ against PageRank with $\alpha = 0.1$')
plt.savefig("results/0_15_vs_0_1.png")

plt.figure()
plt.loglog(pagerank_results['_0_15'], pagerank_results['_0_2'],'o')
plt.xlabel(r'PageRank with $\alpha = 0.15$')
plt.ylabel(r'PageRank with $\alpha = 0.2$')
plt.title(r'PageRank with $\alpha = 0.15$ against PageRank with $\alpha = 0.2$')
plt.savefig("results/0_15_vs_0_2.png")

plt.figure()
plt.loglog(pagerank_results['_0_15'], pagerank_results['_0_5'],'o')
plt.xlabel(r'PageRank with $\alpha = 0.15$')
plt.ylabel(r'PageRank with $\alpha = 0.5$')
plt.title(r'PageRank with $\alpha = 0.15$ against PageRank with $\alpha = 0.5$')
plt.savefig("results/0_15_vs_0_5.png")

plt.figure()
plt.loglog(pagerank_results['_0_15'], pagerank_results['_0_9'],'o')
plt.xlabel(r'PageRank with $\alpha = 0.15$')
plt.ylabel(r'PageRank with $\alpha = 0.9$')
plt.title(r'PageRank with $\alpha = 0.15$ against PageRank with $\alpha = 0.9$')
plt.savefig("results/0_15_vs_0_9.png")

print("Retrieving top 5 and bottom 5 pages...")
argsort = np.argsort(pagerank_results['_0_15'])

file = open('wiki_graph/pageNames.txt')
lines = file.readlines()
lines = [l.split() for l in lines]
file.close()
pageNames = {int(l[0]):" ".join(l[1:]) for l in lines}

argsort_iter = iter(reversed(argsort))
top5 = []
while len(top5) < 5:
    candidate = 1+argsort_iter.__next__()
    if candidate in pageNames.keys():
        top5.append(candidate)
del(argsort_iter)

argsort_iter = iter(argsort)
bottom5 = []
while len(bottom5) < 5:
    candidate = 1+argsort_iter.__next__()
    if candidate in pageNames.keys():
        bottom5.append(candidate)
del(argsort_iter)

file = open('results/top_and_bottom_pagerank.txt', 'w')
file.write("Top 5 results\n")
for idx in top5:
    file.write(pageNames[idx])
    file.write("\n")

file.write("\n")

file.write("Bottom 5 results\n")
for idx in bottom5:
    file.write(pageNames[idx])
    file.write("\n")





