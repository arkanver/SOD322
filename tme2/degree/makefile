degree: degree.c edgelist.o adjmatrix.o adjarray.o
	gcc -O9 -o degree degree.c edgelist.o adjmatrix.o adjarray.o

edgelist.o: edgelist.c edgelist.h
	gcc -O9 -c edgelist.c

adjmatrix.o: adjmatrix.c adjmatrix.h
	gcc -O9 -c adjmatrix.c

adjarray.o: adjarray.c adjarray.h
	gcc -O9 -c adjarray.c