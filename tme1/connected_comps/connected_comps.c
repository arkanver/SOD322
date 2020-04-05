//#include "adjarray.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define NLINKS 100000000 //maximum number of edges for memory allocation, will increase if needed
#define TABLE_SIZE 100000000

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

//Version that takes an adjlist as argument
long unsigned bfs2(adjlist* a, unsigned long s, bool* treated){

	//Initializing useful tables
	//printf("Initializing... ");
	long *queue = malloc(TABLE_SIZE*sizeof(unsigned long));
	for (unsigned long i=0;i<TABLE_SIZE;i++){
		queue[i] = -1;
	}
	//printf("Done !\n");

	long long next_idx = s;
	unsigned long idx_queue = 0;
	unsigned long idx_treated = 0;
	unsigned long count = 0;

	while (next_idx != -1){
		//printf("%d\n\n",next_idx);
		treated[next_idx] = true;
		count++;
		for (unsigned long vois = a->cd[next_idx];vois<a->cd[next_idx+1];vois++){
			queue[idx_queue] = a->adj[vois];
			idx_queue++;
		}
		while (treated[queue[idx_treated]] && idx_treated <= idx_queue+1){
			idx_treated += 1;
		}
		next_idx = queue[idx_treated];
	}

	//printf("Number of nodes in current component : %lu\n", count);
	//printf("Maximum index of a node : %lu\n", a->n);
	free(queue);

  return count;
}


//Does a BFS from a root node included inline (else 0)
int main(int argc, char** argv){

	//Retrieve command-line data
  char* input = argv[1];

	//Convert input graph in adj array
	printf("Converting data... ");
  adjlist *a;
  a = adjarray_readedgelist(input);
  mkadjlist(a);
	printf("Done !\n");

	printf("%lu\n",a->n);


	//Initialize treated table
	bool *treated = malloc((1+a->n)*sizeof(bool));
	for (long i=0;i<=a->n;i++){
		treated[i] = false;
	}

	clock_t start = clock();

  unsigned long compNb = 0;
  unsigned long maxNb = 0;

	printf("Starting computation...\n");
  for (int i=0;i<a->n;i++){
    if (treated[i] == false){
      compNb = bfs2(a,i,treated);
      printf("Connected component with %lu nodes.\n", compNb);
      if (compNb > maxNb){
        maxNb = compNb;
      }
    }
  }
	clock_t end = clock();

	free(treated);
	free_adjlist(a);
  printf("Number of nodes in largest connected component : %lu\n", maxNb);

	double runtime = (double)(end-start)/CLOCKS_PER_SEC;

	printf("Total CPU time : %f\n", runtime);
  return 0;
}
