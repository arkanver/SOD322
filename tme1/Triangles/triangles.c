//#include "adjarray.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define NLINKS 100000000 //maximum number of edges for memory allocation, will increase if needed

typedef struct {
    unsigned long s;
    unsigned long t;
} edge;

//edge list structure:
typedef struct {
    unsigned long n;//number of nodes
    unsigned long e;//number of edges
    edge *edges;//list of edges
    unsigned long *cd;//cumulative degree cd[0]=0 length=n+1
    unsigned long *adj;//concatenated lists of neighbors of all nodes
} adjlist;


//compute the maximum of three unsigned long
unsigned long max3(unsigned long a,unsigned long b,unsigned long c){
	a=(a>b) ? a : b;
	return (a>c) ? a : c;
}

//reading the edgelist from file
adjlist* adjarray_readedgelist(char* input){
	unsigned long e1=NLINKS;
	FILE *file=fopen(input,"r");

	adjlist *g=malloc(sizeof(adjlist));
	g->n=0;
	//g->n=1;
	g->e=0;
	g->edges=malloc(e1*sizeof(edge));//allocate some RAM to store edges

	while (fscanf(file,"%lu %lu", &(g->edges[g->e].s), &(g->edges[g->e].t))==2) {
		g->n=max3(g->n,g->edges[g->e].s,g->edges[g->e].t);
		if (++(g->e)==e1) {//increase allocated RAM if needed
			e1+=NLINKS;
			g->edges=realloc(g->edges,e1*sizeof(edge));
		}
	}
	fclose(file);

	g->n++;

	g->edges=realloc(g->edges,g->e*sizeof(edge));

	return g;
}

//building the adjacency matrix
void mkadjlist(adjlist* g){
	unsigned long i,u,v;
	unsigned long *d=calloc(g->n,sizeof(unsigned long));

	for (i=0;i<g->e;i++) {
		d[g->edges[i].s]++;
		d[g->edges[i].t]++;
	}

	g->cd=malloc((g->n+1)*sizeof(unsigned long));
	g->cd[0]=0;
	for (i=1;i<g->n+1;i++) {
		g->cd[i]=g->cd[i-1]+d[i-1];
		d[i-1]=0;
	}

	g->adj=malloc(2*g->e*sizeof(unsigned long));

	for (i=0;i<g->e;i++) {
		u=g->edges[i].s;
		v=g->edges[i].t;
		g->adj[ g->cd[u] + d[u]++ ]=v;
		g->adj[ g->cd[v] + d[v]++ ]=u;
	}

	free(d);
	//free(g->edges);
}


//freeing memory
void free_adjlist(adjlist *g){
	free(g->edges);
	free(g->cd);
	free(g->adj);
	free(g);
}


//Compute number of triangles in given graph
int main(int argc, char** argv){

  if (argc == 0){
    printf("Please give the path to a graph as input.\n");
    return -1;
  }
  char* input = argv[1];

  //Convert input graph in adj array
	printf("Reading graph... ");
	clock_t start = clock();
  adjlist *a;
  a = adjarray_readedgelist(input);
  mkadjlist(a);
	clock_t inter = clock();
	printf("Done !\n");

  unsigned long triCount = 0;

	print("Computing triangles... ");
	//For every existing edge, compute intersections
	for (int k=0;k<a->e;k++){
		unsigned long s = a->edges[k].s;
		unsigned long t = a->edges[k].t;
		for (int i=a->cd[s];i<a->cd[s+1];i++){
			unsigned long neighbor_s = a->adj[i];
			for (int j=a->cd[t];j<a->cd[t+1];j++){
				if (neighbor_s == a->adj[j]){
					triCount++;
				}
			}
		}
	}
	printf("Done !\n");
	clock_t end = clock();

	double conv_time = (double)(inter-start)/CLOCKS_PER_SEC;
	double tri_time = (double)(end-inter)/CLOCKS_PER_SEC;
	double total_time = (double)(end-start)/CLOCKS_PER_SEC;

	free_adjlist(a);
	free(input);

  printf("Number of triangles : %lu\n", triCount/3);
	printf("Total CPU time : %f\n", total_time);
	printf("CPU time for graph handling : %f\n", conv_time);
	printf("CPU time for triangle computation : %f\n", tri_time);
}
