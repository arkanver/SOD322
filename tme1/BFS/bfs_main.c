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
long unsigned bfs2(adjlist* a, unsigned long s, int* found){

	//Initializing useful tables
	printf("Initializing... ");
	long *queue = malloc(TABLE_SIZE*sizeof(unsigned long));
	bool *treated = malloc(a->n*sizeof(bool));
	for (unsigned long i=0;i<a->n;i++){
		treated[i] = false;
	}
	for (unsigned long i=0;i<TABLE_SIZE;i++){
		queue[i] = -1;
	}

	printf("Done !\n");

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

	printf("Number of nodes in current component : %lu\n", count);
	printf("Maximum index of a node : %lu\n", a->n);
	free_adjlist(a);
	free(queue);
	free(treated);

  return count;
}

//Prints vertices in order found through BFS.
int bfs(char* input, unsigned long s){

  //Convert input graph in adj array
	printf("Creating adjacency list... ");
  adjlist *a;
  a = adjarray_readedgelist(input);
  mkadjlist(a);
	printf("Done !\n");

	//Initializing useful tables
	printf("Initializing... ");
	long *queue = malloc(TABLE_SIZE*sizeof(unsigned long));
	bool *treated = malloc(a->n*sizeof(bool));
	for (unsigned long i=0;i<a->n;i++){
		treated[i] = false;
	}
	for (unsigned long i=0;i<TABLE_SIZE;i++){
		queue[i] = -1;
	}

	printf("Done !\n");

  long long next_idx = s;
	unsigned long idx_queue = 0;
	unsigned long idx_treated = 0;
	unsigned long count = 0;

	clock_t start = clock();

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

	clock_t end = clock();

	double runtime = (double)(end-start)/CLOCKS_PER_SEC;

  printf("Number of nodes in current component : %lu\n", count);
  printf("Maximum index of a node : %lu\n", a->n);
	printf("BFS CPU time : %f\n", runtime);
	free_adjlist(a);
	free(queue);
	free(treated);

  return 0;
}

//Detects connected components and prints the number of nodes in each.
void connectedcomps(char* input){
  //Convert input graph in adj array
  adjlist *a;
  a = adjarray_readedgelist(input);

  mkadjlist(a);

  int* found = calloc(a->n, sizeof(int));
  unsigned long totalNb = 0;
  unsigned long compNb = 0;
  unsigned long maxNb = 0;

  for (int i=0;i<a->n;i++){
    if (found[i] == 0){
      compNb = bfs2(a,i,found);
      printf("Connected component with %lu nodes.\n", compNb);
      totalNb += compNb;
      if (compNb > maxNb){
        maxNb = compNb;
      }
    }
  }
	free(found);
	free_adjlist(a);
  printf("Total number of nodes : %lu\n", totalNb);
  printf("Number of nodes in largest connected component : %lu\n", maxNb);
}


//Modified version of bfs for longest path problem - returns end point of longest path found
int bfsdiam(adjlist* a, unsigned long s){

	//Initializing useful tables
	printf("Initializing... ");
	long *queue = malloc(TABLE_SIZE*sizeof(unsigned long));
	bool *treated = malloc(a->n*sizeof(bool));
	unsigned long *stage = calloc(a->n,sizeof(unsigned long));
	for (unsigned long i=0;i<a->n;i++){
		treated[i] = false;
	}
	for (unsigned long i=0;i<TABLE_SIZE;i++){
		queue[i] = -1;
	}

	printf("Done !\n");

  long long next_idx = s;
  stage[next_idx] = 1;

	unsigned long idx_queue = 0;
	unsigned long idx_treated = 0;

  unsigned long count = 0;
  unsigned long path_length = 0;
  unsigned long path_end = 0;

	while (next_idx != -1){
		//printf("%d\n\n",next_idx);
		treated[next_idx] = true;
		count++;
		for (unsigned long vois = a->cd[next_idx];vois<a->cd[next_idx+1];vois++){
			if (stage[a->adj[vois]] == 0){
        stage[a->adj[vois]] = stage[next_idx] + 1;
        if (stage[next_idx] > path_length){
          path_length = stage[next_idx];
          path_end = a->adj[vois];
				}
			}
			queue[idx_queue] = a->adj[vois];
			idx_queue++;
		}
		while (treated[queue[idx_treated]] && idx_treated <= idx_queue+1){
			idx_treated += 1;
		}
		next_idx = queue[idx_treated];
	}

  printf("Current longest path : %lu\n", path_length);
  printf("Longest path goes from %lu to %lu\n", s, path_end);
	free(queue);
	free(treated);

  return path_end;
}


//Finds a lower bound to the diameter of a graph given in adjarray form
void diamLB(adjlist *a, int start, int nb_iter){
  //Maybe start in largest WCC ?
  unsigned long bfs_start = start;
  unsigned long next_start;
  for (int i=0;i<nb_iter;i++){
		printf("Iteration %d : ", i+1);
    next_start = bfsdiam(a,bfs_start);
    bfs_start = next_start;
  }
}

//Does a BFS from a root node included inline (else 0)
int main(int argc, char** argv){

  char* input = argv[1];
  char* start_point = argv[2];
  int s = atoi(start_point);

	clock_t start = clock();

	bfs(input,s);

	clock_t end = clock();

	double runtime = (double)(end-start)/CLOCKS_PER_SEC;

	printf("Total CPU time : %f\n", runtime);
  return 0;
}
