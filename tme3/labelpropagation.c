#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdbool.h>
//#include "adjarray.h"

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


//Finding the most frequent label in a vertex's neighborhood
unsigned long mostFrequentLabel(adjlist *a, unsigned long* label, unsigned long v, unsigned long *labels_found, unsigned long *nb, unsigned long *most_present){
	//Find number of neighbors

	unsigned long nb_neighbors = a->cd[v+1]-a->cd[v];
	//Remember their labels and the number of times they appear
	//printf("Init labels_found\n");
	//unsigned long *labels_found = malloc(nb_neighbors*sizeof(unsigned long));
	for (unsigned long i=0;i<nb_neighbors;i++){
		labels_found[i] = a->n;
	}
	//printf("Init nb\n");
	//unsigned long *nb = calloc(nb_neighbors,sizeof(unsigned long));
	unsigned long number_of_labels = 0; //Number of labels already found
	unsigned long adj_start = a->cd[v];

	//Fill out the tables
	for (unsigned long i=0;i<nb_neighbors;i++){
		unsigned long j = 0;
		bool found = false;
		unsigned long i_label = label[a->adj[adj_start+i]];
		while (j<number_of_labels && !found){
			if (i_label == labels_found[j]){
				found = true;
				nb[j] += 1;
			}
			j += 1;
		}
		if (!found){
			labels_found[number_of_labels] = i_label;
			nb[number_of_labels] = 1;
			number_of_labels += 1;
		}
	}

	//Find the most present label
	//unsigned long *most_present = malloc(number_of_labels*sizeof(unsigned long));
	//printf("Done !\n");
	unsigned long max = 0;
	unsigned long nbmax = 0;

	for (unsigned long i=0;i<number_of_labels;i++){
		if (nb[i] > max){
			most_present[0] = labels_found[i];
			max = nb[i];
			nbmax = 1;
		}
		//If multiple labels are found the same amount of time, keep them in history
		//We will choose at random
		if (nb[i] == max){
			most_present[nbmax] = labels_found[i];
			nbmax += 1;
		}
	}
	int chosen = rand() % nbmax;
	unsigned long label_chosen = most_present[chosen];

	//Re-initialize nb
	for (unsigned long i=0;i<a->n;i++){
		nb[i] = 0;
	}
	return label_chosen;
}

//Shuffle with Fisher-Yates method. Shuffles only up to index n.
void fisherYates(unsigned long* order, unsigned long n){
	for (unsigned long i=0;i<n;i++){
		unsigned long j = i + rand() % (n-i);
		unsigned long temp = order[i];
		order[i] = order[j];
		order[j] = temp;
	}
}


void labelPropagation(char* input, char* output){
	srand(time(NULL));
	printf("Creating adjlist...\n");
	//Create the adjlist
	adjlist *a = adjarray_readedgelist(input);
	mkadjlist(a);
	printf("Done !\n");

	printf("Initializing labels...\n");
	// Initialize labels and list of vertices
	unsigned long *label = malloc(a->n*sizeof(unsigned long));
	unsigned long *order = malloc(a->n*sizeof(unsigned long));
	unsigned long *labels_found = malloc(a->n*sizeof(unsigned long));
	unsigned long *nb = calloc(a->n,sizeof(unsigned long));
	unsigned long *most_present = malloc(a->n*sizeof(unsigned long));
	for (unsigned long i=0;i<a->n;i++){
		label[i] = i;
		order[i] = i;
	}
	printf("Done !\n");

	bool finished = false;

	printf("Algorithm start. \n");
	clock_t start = clock();
	while (!finished){
		finished = true;
		//First step : shuffle vertices
		printf("  Shuffling...");
		fisherYates(order,a->n);
		printf(" Done !\n");

		//Second step : modify labels according to most present label around
		printf("  Re-labeling...");
		for (unsigned long i=0;i<a->n;i++){
			unsigned long v = order[i];
			unsigned long most_present_label = mostFrequentLabel(a,label,v,labels_found,nb,most_present);
			if (label[v] != most_present_label){
				label[v] = most_present_label;
				finished = false;
			}
		}
		printf(" Done !\n");
	}
	free(order);
	clock_t end = clock();
	printf("Algorithm stop.\n");
	double runtime = (double)(end-start)/CLOCKS_PER_SEC;
	printf("CPU time to compute communities : %f s\n", runtime);

	//Print results in output file
	FILE *file = fopen(output, "w");
	if (file == NULL){
		printf("Could not write results.");
	}
	else{
		for (unsigned long i=0;i<a->n;i++){
			fprintf(file,"%lu %lu\n",i,label[i]);
		}
	}
	free(label);
	free_adjlist(a);
}



int main(int argc, char** argv){
	char* input = argv[1];
	char* output = argv[2];
	labelPropagation(input,output);
	return 0;
}
