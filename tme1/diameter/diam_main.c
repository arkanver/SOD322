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

//FIFO structure
typedef struct {
    unsigned long n;//number of nodes in the pile
    unsigned long *cont;//All nodes currently in the pile
} fifo;

//Create FIFO prepared for n entries
void mkfifo(fifo* pile, unsigned long n){
  pile->n = 0;
  pile->cont = calloc(n, sizeof(unsigned long));
}

void addfifo(fifo* pile, unsigned long a){
  pile->cont[pile->n] = a;
  pile->n += 1;
}

void rmfifo(fifo* pile){
  pile->n -= 1;
  for (unsigned long i=0;i<pile->n;i++){
    pile->cont[i] = pile->cont[i+1];
  }
}

void freefifo(fifo* pile){
	free(pile->cont);
	free(pile);
}


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


//Modified version of bfs for longest path problem - returns end point of longest path found
int bfsdiam(adjlist* a, unsigned long s){

	//Initializing useful tables
	long *queue = malloc(TABLE_SIZE*sizeof(unsigned long));
	bool *treated = malloc(a->n*sizeof(bool));
	unsigned long *stage = calloc(a->n,sizeof(unsigned long));
	for (unsigned long i=0;i<a->n;i++){
		treated[i] = false;
	}
	for (unsigned long i=0;i<TABLE_SIZE;i++){
		queue[i] = -1;
	}

  long long next_idx = s;
  stage[next_idx] = 1;

	unsigned long idx_queue = 0;
	unsigned long idx_treated = 0;

  unsigned long count = 0;
  unsigned long path_length = 0;
  unsigned long path_end;

	unsigned long nb_max = 0;
	unsigned long *max_length = malloc(a->n*sizeof(unsigned long));


	while (next_idx != -1){
		//printf("%d\n\n",next_idx);
		treated[next_idx] = true;
		count++;
		for (unsigned long vois = a->cd[next_idx];vois<a->cd[next_idx+1];vois++){
			if (stage[a->adj[vois]] == 0){
        stage[a->adj[vois]] = stage[next_idx] + 1;
        if (stage[next_idx] > path_length){
					max_length[0] = a->adj[vois];
					nb_max = 1;
          path_length = stage[next_idx];
				}
				if (stage[next_idx] == path_length){
					max_length[nb_max] = a->adj[vois];
					nb_max++;
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

	//Randomly choose path end among different possible ends
	srand(time(NULL));
	int end = rand() % nb_max;
	path_end = max_length[end];

  printf("Current longest path : %lu\n", path_length);
  printf("Longest path goes from %lu to %lu\n", s, path_end);
	free(queue);
	free(treated);
	free(stage);
	free(max_length);

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


int main(int argc, char** argv){

  char* input = argv[1];
  char* start_point = argv[2];
	char* nb_iter = argv[3];
  int s = atoi(start_point);
	int nb = atoi(nb_iter);
  //Convert input graph in adj array
  adjlist *a;
	clock_t start = clock();
	printf("Creating adjacency list...");
  a = adjarray_readedgelist(input);
  mkadjlist(a);
	printf(" Done !\n");

	clock_t inter = clock();

	printf("Computing diameter...\n");
  diamLB(a,s,nb);

	clock_t end = clock();

	double conv_time = (double)(inter-start)/CLOCKS_PER_SEC;
	double diam_time = (double)(end-inter)/CLOCKS_PER_SEC;
	double runtime = (double)(end-start)/CLOCKS_PER_SEC;

	printf("Total CPU time : %f\n", runtime);
	printf("Data converting CPU time : %f\n", conv_time);
	printf("Time computing diameter : %f\n", diam_time);
  return 0;
}
