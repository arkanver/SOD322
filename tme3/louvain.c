#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <strings.h>//to use "bzero"
#include <time.h>//to estimate the runing time
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

//#include "partition.h"
//#include "struct.h"

#define NLINKS 100000000 //maximum number of edges of the input graph: used for memory allocation, will increase if needed. //NOT USED IN THE CURRENT VERSION
#define NNODES 10000000 //maximum number of nodes in the input graph: used for memory allocation, will increase if needed
#define HMAX 100 //maximum depth of the tree: used for memory allocation, will increase if needed

//All structures used
typedef struct {
  unsigned long s;
  unsigned long t;
} edge;


//edge list structure:
typedef struct {
  unsigned long n;//number of nodes
  unsigned long long e;//number of edges
  unsigned long long emax;//max number of edges
  edge *edges;//list of edges
  unsigned long *map;//map[u]=original label of node u
} edgelist;


//edge list structure:
typedef struct {
  unsigned long n;//number of nodes
  unsigned long long e;//number of edges
  unsigned long long emax;//max number of edges
  edge *edges;//list of edges
  unsigned long long *cd;//cumulative degree cd[0]=0 length=n+1
  unsigned long *adj;//concatenated lists of neighbors of all node
  long double *weights;//concatenated lists of weights of neighbors of all nodes
  long double totalWeight;//total weight of the links
  unsigned long *map;//map[u]=original label of node u
} adjlist;

// louvain partition
typedef struct {
  // size of the partition
  unsigned long size;

  // community to which each node belongs
  unsigned long *node2Community;

  // in and tot values of each node to compute modularity
  long double *in;
  long double *tot;

  // utility arrays to find communities adjacent to a node
  // communities are stored using three variables
  // - neighCommWeights: stores weights to communities
  // - neighCommPos: stores list of neighbor communities
  // - neighCommNb: stores the number of neighbor communities
  long double *neighCommWeights;
  unsigned long *neighCommPos;
  unsigned long neighCommNb;
} louvainPartition;


//Needed redefinition of qsort_r with necessary add-ons.
static _Thread_local struct
{
    int (*compar)(const void *, const void *, void *);
    void *arg;
} state;

static int compar_wrapper(const void *a, const void *b)
{
    return state.compar(a, b, state.arg);
}

/* Follow the upcoming POSIX prototype, coming from GNU/libc.
 * Note that this differs from the BSD prototype. */

void qsort_r(void *base, size_t nmemb, size_t size,
             int (*compar)(const void *, const void *, void *),
             void *arg)
{
    int (*saved_compar)(const void *, const void *, void *) = state.compar;
    void *saved_arg = state.arg;

    state.compar = compar;
    state.arg = arg;

    qsort(base, nmemb, size, compar_wrapper);

    /* Restore state for nested reentrant calls */
    state.compar = saved_compar;
    state.arg = saved_arg;
}




//Contents of partition.c
#define NLINKS2 8
#define MIN_IMPROVEMENT 0.005

int myCompare (const void * a, const void * b, void * array2) {
  long diff = ((unsigned long *)array2)[*(unsigned long *)a] - ((unsigned long *)array2)[*(unsigned *)b];
  int res = (0 < diff) - (diff < 0);
  return  res;
}

unsigned long * mySort(unsigned long *part, unsigned long size) {
  unsigned long i;
  unsigned long *nodes = (unsigned long *)malloc(size * sizeof(unsigned long));
  for (i = 0; i < size; i++) {
    nodes[i]=i;
  }
  qsort_r(nodes, size, sizeof(unsigned long), myCompare, (void *)part);
  //qsort(nodes, size, sizeof(unsigned long), myCompare, (void *)part);
  return nodes;
}



long double degreeWeighted(adjlist *g, unsigned long node) {
  unsigned long long i;
  if (g->weights == NULL) {
    return 1.*(g->cd[node + 1] - g->cd[node]);
  }

  long double res = 0.0L;
  for (i = g->cd[node]; i < g->cd[node + 1]; i++) {
    res += g->weights[i];
  }
  return res;
}

long double selfloopWeighted(adjlist *g, unsigned long node) {
  unsigned long long i;

  for (i = g->cd[node]; i < g->cd[node + 1]; i++) {
    if (g->adj[i] == node) {
      return (g->weights == NULL)?1.0:g->weights[i];
    }
  }

  return 0.0;
}

void removeNode(louvainPartition *p, adjlist *g, unsigned long node, unsigned long comm, long double dnodecomm) {
  p->in[comm]  -= 2.0L * dnodecomm + selfloopWeighted(g, node);
  p->tot[comm] -= degreeWeighted(g, node);
}

void insertNode(louvainPartition *p, adjlist *g, unsigned long node, unsigned long comm, long double dnodecomm) {
  p->in[comm]  += 2.0L * dnodecomm + selfloopWeighted(g, node);
  p->tot[comm] += degreeWeighted(g, node);

  p->node2Community[node] = comm;
}

long double gain(louvainPartition *p, adjlist *g, unsigned long comm, long double dnc, long double degc) {
  long double totc = p->tot[comm];
  long double m2   = g->totalWeight;

  return (dnc - totc*degc/m2);
}

//freeing memory
void free_adjlist2(adjlist *g){
  free(g->cd);
  free(g->adj);
  free(g->weights);
  free(g);
}

void freeLouvainPartition(louvainPartition *p) {
  free(p->in);
  free(p->tot);
  free(p->neighCommWeights);
  free(p->neighCommPos);
  free(p->node2Community);
  free(p);
}


louvainPartition *createLouvainPartition(adjlist *g) {
  unsigned long i;

  louvainPartition *p = malloc(sizeof(louvainPartition));

  p->size = g->n;

  p->node2Community = malloc(p->size * sizeof(unsigned long));
  p->in = malloc(p->size * sizeof(long double));
  p->tot = malloc(p->size * sizeof(long double));

  p->neighCommWeights = malloc(p->size * sizeof(long double));
  p->neighCommPos = malloc(p->size * sizeof(unsigned long));
  p->neighCommNb = 0;

  for (i = 0; i < p->size; i++) {
    p->node2Community[i] = i;
    p->in[i]  = selfloopWeighted(g, i);
    p->tot[i] = degreeWeighted(g, i);
    p->neighCommWeights[i] = -1;
    p->neighCommPos[i] = 0;
  }

  return p;
}

long double modularity(louvainPartition *p, adjlist *g) {
  long double q  = 0.0L;
  long double m2 = g->totalWeight;
  unsigned long i;

  for (i = 0; i < p->size; i++) {
    if (p->tot[i] > 0.0L)
      q += p->in[i] - (p->tot[i] * p->tot[i]) / m2;
  }

  return q / m2;
}

void neighCommunitiesInit(louvainPartition *p) {
  unsigned long i;
  for (i = 0; i < p->neighCommNb; i++) {
    p->neighCommWeights[p->neighCommPos[i]] = -1;
  }
  p->neighCommNb = 0;
}

/*
Computes the set of neighbor communities of a given node (excluding self-loops)
*/
void neighCommunities(louvainPartition *p, adjlist *g, unsigned long node) {
  unsigned long long i;
  unsigned long neigh, neighComm;
  long double neighW;
  p->neighCommPos[0] = p->node2Community[node];
  p->neighCommWeights[p->neighCommPos[0]] = 0.;
  p->neighCommNb = 1;

  // for all neighbors of node, add weight to the corresponding community
  for (i = g->cd[node]; i < g->cd[node + 1]; i++) {
    neigh  = g->adj[i];
    neighComm = p->node2Community[neigh];
    neighW = (g->weights == NULL)?1.0:g->weights[i];

    // if not a self-loop
    if (neigh != node) {
      // if community is new (weight == -1)
      if (p->neighCommWeights[neighComm] == -1) {
	p->neighCommPos[p->neighCommNb] = neighComm;
	p->neighCommWeights[neighComm] = 0.;
	p->neighCommNb++;
      }
      p->neighCommWeights[neighComm] += neighW;
    }
  }
}

/*
Same behavior as neighCommunities except:
- self loop are counted
- data structure if not reinitialised
*/
void neighCommunitiesAll(louvainPartition *p, adjlist *g, unsigned long node) {
  unsigned long long i;
  unsigned long neigh, neighComm;
  long double neighW;

  for (i = g->cd[node]; i < g->cd[node + 1]; i++) {
    neigh  = g->adj[i];
    neighComm = p->node2Community[neigh];
    neighW = (g->weights == NULL)?1.0:g->weights[i];

    // if community is new
    if (p->neighCommWeights[neighComm] == -1) {
      p->neighCommPos[p->neighCommNb] = neighComm;
      p->neighCommWeights[neighComm] = 0.;
      p->neighCommNb++;
    }
    p->neighCommWeights[neighComm] += neighW;
  }
}


unsigned long updatePartition(louvainPartition *p, unsigned long *part, unsigned long size) {
  // Renumber the communities in p
  unsigned long *renumber = calloc(p->size, sizeof(unsigned long));
  unsigned long i, last = 1;
  for (i = 0; i < p->size; i++) {
    if (renumber[p->node2Community[i]] == 0) {
      renumber[p->node2Community[i]] = last++;
    }
  }

  // Update part with the renumbered communities in p
  for (i = 0; i < size; i++) {
    part[i] = renumber[p->node2Community[part[i]]] - 1;
  }

  free(renumber);
  return last-1;
}


// Return the meta graph induced by a partition of a graph
// See Louvain article for more details
adjlist* louvainPartition2Graph(louvainPartition *p, adjlist *g) {
  unsigned long node, i, j;
  // Renumber communities
  unsigned long *renumber = (unsigned long *)malloc(g->n * sizeof(unsigned long));
  for (node = 0; node < g->n; node++)
    renumber[node] = 0;
  unsigned long last = 1;
  for (node = 0; node < g->n; node++) {
    if (renumber[p->node2Community[node]] == 0) {
      renumber[p->node2Community[node]] = last++;
    }
  }
  for (node = 0; node < g->n; node++) {
    p->node2Community[node] = renumber[p->node2Community[node]] - 1 ;
  }

  // sort nodes according to their community
  unsigned long * order = mySort(p->node2Community, g->n);
  //  displayPartU(p->node2Community, g->n);

  // Initialize meta graph
  adjlist *res = (adjlist *)malloc(sizeof(adjlist));
  unsigned long long e1 = NLINKS2;
  res->n = last - 1;
  res->e = 0;
  res->cd = (unsigned long long *)calloc((1 + res->n), sizeof(unsigned long long));
  res->cd[0] = 0;
  res->adj = (unsigned long *)malloc(NLINKS2 * sizeof(unsigned long));
  res->totalWeight = 0.0L;
  res->weights = (long double *)malloc(NLINKS2 * sizeof(long double));

  // for each node (in community order), extract all edges to other communities and build the graph
  neighCommunitiesInit(p);
  unsigned long oldComm = p->node2Community[order[0]];//renumber[p->node2Community[order[0]]];

  unsigned long currentComm;
  for (i = 0; i <= p->size; i++) {
    // current node and current community with dummy values if out of bounds
    node = (i == p->size)?0:order[i];
    currentComm = (i == p->size)?currentComm + 1:p->node2Community[order[i]];

    // new community, write previous one
    if (oldComm != currentComm) {
      res->cd[oldComm + 1] = res->cd[oldComm] + p->neighCommNb;
      //      displayPartU(res->degrees, res->n + 1);

      // for all neighboring communities of current community
      for (j = 0; j < p->neighCommNb; j++) {
	unsigned long neighComm = p->neighCommPos[j];
	long double neighCommWeight = p->neighCommWeights[p->neighCommPos[j]];

	// add edge in res
	res->adj[res->e] = neighComm;
	res->weights[res->e] = neighCommWeight;
	res->totalWeight += neighCommWeight;
	(res->e)++;

	// reallocate edges and weights if necessary
	if (res->e == e1) {
	  e1 *= 2;
	  res->adj = (unsigned long *)realloc(res->adj, e1 * sizeof(unsigned long));
	  res->weights = (long double *)realloc(res->weights, e1 * sizeof(long double));
	  if (res->adj == NULL || res->weights == NULL) {
	    printf("error during memory allocation\n");
	    exit(0);
	  }
	}

      }

      //      display(res);

      if (i == p->size) {
	res->adj = (unsigned long *)realloc(res->adj, res->e * sizeof(unsigned long));
	res->weights = (long double *)realloc(res->weights, res->e * sizeof(long double));

	free(order);
	free(renumber);

	return res;
      }

      oldComm = currentComm;
      neighCommunitiesInit(p);
    }

    // add neighbors of node i
    neighCommunitiesAll(p, g, node);
    //    displayNeighs(p, node);
  }

  printf("bad exit\n");
  return res;
}

// Compute one pass of Louvain and returns the improvement
long double louvainOneLevel(louvainPartition *p, adjlist *g) {
  unsigned long nbMoves;
  long double startModularity = modularity(p, g);
  long double newModularity = startModularity;
  long double curModularity;
  unsigned long i,j,node;
  unsigned long oldComm,newComm,bestComm;
  long double degreeW, bestCommW, bestGain, newGain;


  // generate a random order for nodes' movements
  unsigned long *randomOrder = (unsigned long *)malloc(p->size * sizeof(unsigned long));
  for (unsigned long i = 0; i < p->size; i++)
    randomOrder[i] = i;
  for (unsigned long i = 0; i < p->size - 1; i++) {
    unsigned long randPos = rand()%(p->size - i) + i;
    unsigned long  tmp = randomOrder[i];
    randomOrder[i] = randomOrder[randPos];
    randomOrder[randPos] = tmp;
  }

  // repeat while
  //   there are some nodes moving
  //   or there is an improvement of quality greater than a given epsilon
  do {
    curModularity = newModularity;
    nbMoves = 0;

    // for each node:
    //   remove the node from its community
    //   compute the gain for its insertion in all neighboring communities
    //   insert it in the best community with the highest gain
    for (i = 0; i < g->n; i++) {
      node = i;//randomOrder[nodeTmp];
      oldComm = p->node2Community[node];
      degreeW = degreeWeighted(g, node);

      // computation of all neighboring communities of current node
      neighCommunitiesInit(p);
      neighCommunities(p, g, node);

      // remove node from its current community
      removeNode(p, g, node, oldComm, p->neighCommWeights[oldComm]);

      // compute the gain for all neighboring communities
      // default choice is the former community
      bestComm = oldComm;
      bestCommW  = 0.0L;
      bestGain = 0.0L;
      for (j = 0; j < p->neighCommNb; j++) {
	newComm = p->neighCommPos[j];
	newGain = gain(p, g, newComm, p->neighCommWeights[newComm], degreeW);

	if (newGain > bestGain) {
	  bestComm = newComm;
	  bestCommW = p->neighCommWeights[newComm];
	  bestGain = newGain;
	}
      }

      // insert node in the nearest community
      insertNode(p, g, node, bestComm, bestCommW);

      if (bestComm != oldComm) {
	nbMoves++;
      }
    }

    newModularity = modularity(p, g);

    //    printf("%Lf\n", newModularity);

  } while (nbMoves>0 &&
    	   newModularity - curModularity > MIN_IMPROVEMENT);

  free(randomOrder);

  return newModularity - startModularity;
}

unsigned long louvain(adjlist *g, unsigned long *lab) {
  unsigned long i,n;
  long double improvement;
  // Initialize partition with trivial communities
  for (i = 0; i < g->n; i++) {
    lab[i] = i;
  }

  louvainPartition *gp = createLouvainPartition(g);
  louvainOneLevel(gp, g);
  n = updatePartition(gp, lab, g->n);
  freeLouvainPartition(gp);

  return n;
}

unsigned long louvainComplete(adjlist *g, unsigned long *lab) {
  adjlist *init = g, *g2;
  unsigned long n, i;
  unsigned long long j;
  unsigned long originalSize = g->n;
  long double improvement;
  // Initialize partition with trivial communities
  for (i = 0; i < g->n; i++) {
    lab[i] = i;
  }

  // Execution of Louvain method
  while(1) {
    louvainPartition *gp = createLouvainPartition(g);

    improvement = louvainOneLevel(gp, g);

    n = updatePartition(gp, lab, originalSize);

    if (improvement < MIN_IMPROVEMENT) {
      freeLouvainPartition(gp);
      break;
    }

    g2 = louvainPartition2Graph(gp, g);

    // free all graphs except the original one
    if (g->n < originalSize) {
      free_adjlist2(g);
    }
    freeLouvainPartition(gp);
    g = g2;
  }

  if (g->n < originalSize) {
    free_adjlist2(g);
  }

  return n;
}



//compute the maximum of three unsigned long
//inline unsigned long max3(unsigned long a,unsigned long b,unsigned long c){
unsigned long max3(unsigned long a,unsigned long b,unsigned long c){
  a = (a > b) ? a : b;
  return (a > c) ? a : c;
}

//reading the edgelist from file
//NOT USED IN THE CURRENT VERSION
edgelist* readedgelist(char* input){
  unsigned long long e1=NLINKS;
  edgelist *el=malloc(sizeof(edgelist));
  FILE *file;

  el->n=0;
  el->e=0;
  file=fopen(input,"r");
  el->edges=malloc(e1*sizeof(edge));
  while (fscanf(file,"%lu %lu", &(el->edges[el->e].s), &(el->edges[el->e].t))==2) {
    el->n=max3(el->n,el->edges[el->e].s,el->edges[el->e].t);
    if (++(el->e)==e1) {
      e1+=NLINKS;
      el->edges=realloc(el->edges,e1*sizeof(edge));
    }
  }
  fclose(file);
  el->n++;

  el->edges=realloc(el->edges,el->e*sizeof(edge));

  return el;
}

//freeing memory
//NOT USED IN THE CURRENT VERSION
void free_edgelist(edgelist *el){
  free(el->edges);
  free(el);
}

//building the adjacency matrix
//NOT USED IN THE CURRENT VERSION
adjlist* mkadjlist(edgelist* el){
  unsigned long long i;
  unsigned long u,v;
  unsigned long *d=calloc(el->n,sizeof(unsigned long));
  adjlist *g=malloc(sizeof(adjlist));

  g->n=el->n;
  g->e=el->e;

  for (i=0;i<el->e;i++) {
    d[el->edges[i].s]++;
    d[el->edges[i].t]++;
  }

  g->cd=malloc((g->n+1)*sizeof(unsigned long long));
  g->cd[0]=0;
  for (i=1;i<g->n+1;i++) {
    g->cd[i]=g->cd[i-1]+d[i-1];
    d[i-1]=0;
  }

  g->adj=malloc(2*g->e*sizeof(unsigned long));
  for (i=0;i<g->e;i++) {
    u=el->edges[i].s;
    v=el->edges[i].t;
    g->adj[ g->cd[u] + d[u]++ ]=v;
    g->adj[ g->cd[v] + d[v]++ ]=u;
  }

  g->weights = NULL;
  g->totalWeight = 2*g->e;
  g->map=NULL;

  free(d);
  free_edgelist(el);

  return g;
}

//reading the list of edges and building the adjacency array
adjlist* readadjlist(char* input){
  unsigned long n1=NNODES,n2,u,v,i;
  unsigned long *d=calloc(n1,sizeof(unsigned long));
  adjlist *g=malloc(sizeof(adjlist));
  FILE *file;

  g->n=0;
  g->e=0;
  file=fopen(input,"r");//first reading to compute the degrees
  while (fscanf(file,"%lu %lu", &u, &v)==2) {
    g->e++;
    g->n=max3(g->n,u,v);
    if (g->n+1>=n1) {
      n2=g->n+NNODES;
      d=realloc(d,n2*sizeof(unsigned long));
      memset(d+n1, 0, (n2-n1)*sizeof(unsigned long));
      n1=n2;
    }
    d[u]++;
    d[v]++;
  }
  fclose(file);

  g->n++;
  d=realloc(d,g->n*sizeof(unsigned long));

  g->cd=malloc((g->n+1)*sizeof(unsigned long long));
  g->cd[0]=0;
  for (i=1;i<g->n+1;i++) {
    g->cd[i]=g->cd[i-1]+d[i-1];
    d[i-1]=0;
  }

  g->adj=malloc(2*g->e*sizeof(unsigned long));

  file=fopen(input,"r");//secong reading to fill the adjlist
  while (fscanf(file,"%lu %lu", &u, &v)==2) {
    g->adj[ g->cd[u] + d[u]++ ]=v;
    g->adj[ g->cd[v] + d[v]++ ]=u;
  }
  fclose(file);

  g->weights = NULL;
  g->totalWeight = 2*g->e;
  g->map=NULL;

  free(d);

  return g;
}

//reading the list of edges and building the adjacency array
//NOT USED IN THE CURRENT VERSION
#define BUFFER_SIZE (16 * 1024)

int
read_two_integers(int fd, unsigned long *u, unsigned long *v) {
  static char buf[BUFFER_SIZE];
  static ssize_t how_many = 0;
  static int pos = 0;
  unsigned long node_number=0;
  int readu = 0;

  while (1) {
    while(pos < how_many) {
      if (buf[pos] == ' ') {
	*u = node_number;
	readu=1;
	node_number = 0;
	pos++;
      } else if (buf[pos] == '\n') {
	*v = node_number;
	node_number = 0;
	readu=0;
	pos++;
	return 2;
      } else {
	node_number = node_number * 10 + buf[pos] - '0';
	pos++;
      }
    }

    how_many = read (fd, buf, BUFFER_SIZE);
    pos = 0;
    if (how_many == 0) {
      if(readu==1) {
	*v = node_number;
	return 2;
      }
      return 0;;
    }
  }
}

//freeing memory
void free_adjlist(adjlist *g){
  free(g->cd);
  free(g->adj);
  free(g->weights);
  free(g->map);
  free(g);
}


//main function
int main(int argc,char** argv){
  adjlist* g;
  unsigned long *part;
  unsigned long i;

  time_t t0 = time(NULL), t1, t2, t3;
  srand(time(NULL));

  printf("Reading edgelist from file %s and building adjacency array\n", argv[1]);
  g = readadjlist(argv[1]);
  printf("Number of nodes: %lu\n", g->n);
  printf("Number of edges: %llu\n", g->e);

  /*
  //using more memory but reading the input text file only once
  edgelist* el;
  printf("Reading edgelist from file %s\n", argv[1]);
  el=readedgelist(argv[1]);
  printf("Number of nodes: %lu\n", el->n);
  printf("Number of edges: %llu\n", el->e);
  t1 = time(NULL);
  printf("- Time = %ldh%ldm%lds\n", (t1-t0)/3600, ((t1-t0)%3600)/60, ((t1-t0)%60));
  printf("Building adjacency array\n");
  g=mkadjlist(el);
  */

  t1 = time(NULL);
  printf("- Time to load the graph = %ldh%ldm%lds\n", (t1-t0)/3600, ((t1-t0)%3600)/60, ((t1-t0)%60));

  printf("Starting louvain\n");
  part = malloc(g->n * sizeof(unsigned long));
  louvainComplete(g, part);
  t2 = time(NULL);
  printf("- Time to compute communities = %ldh%ldm%lds\n", (t2-t1)/3600, ((t2-t1)%3600)/60, ((t2-t1)%60));

  printf("Prints result in file %s\n", argv[2]);
  FILE* out = fopen(argv[2], "w");
  for(i = 0; i < g->n; i++){
    fprintf(out, "%lu %lu\n", i, part[i]);
  }
  fclose(out);
  t3 = time(NULL);
  printf("- Time to export communities = %ldh%ldm%lds\n", (t3-t2)/3600, ((t3-t2)%3600)/60, ((t3-t2)%60));

  printf("- Overall time = %ldh%ldm%lds\n", (t3-t0)/3600, ((t3-t0)%3600)/60, ((t3-t0)%60));

  return 0;
}
