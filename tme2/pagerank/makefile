pagerank: pagerank.c edgelist.o adjmatrix.o adjarray.o heapfunctions.o heapfunctions.c heapfunctions.h
	gcc -O9 -o pagerank pagerank.c heapfunctions.c edgelist.o adjmatrix.o adjarray.o -lm

edgelist.o: edgelist.c edgelist.h
	gcc -O9 -c edgelist.c

adjmatrix.o: adjmatrix.c adjmatrix.h
	gcc -O9 -c adjmatrix.c

adjarray.o: adjarray.c adjarray.h
	gcc -O9 -c adjarray.c