#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int graphGenerator(char* input, unsigned long nbClus, unsigned long nbNodes, float p, float q){
  //First, open file
  FILE *file;
  file = fopen(input,"w");

  if (file==NULL){
    printf("Error : could not open file.\n");
    return -1;
  }

  //Initialize RNG
  srand(time(NULL));

  //Total number of nodes
  unsigned long totalNb = nbNodes*nbClus;

  for (int k=0;k<nbClus;k++){
    //For edges interior to cluster k
    for(int i=k*nbNodes;i<(k+1)*nbNodes;i++){
      for (int j=i+1;j<(k+1)*nbNodes;j++){
        if ((float)rand()/(float)RAND_MAX <= p){
          fprintf(file,"%d %d\n",i,j);
        }
      }
      for (int j=(k+1)*nbNodes;j<totalNb;j++){
        if ((float)rand()/(float)RAND_MAX <= q){
          fprintf(file, "%d %d\n",i,j);
        }
      }
    }
  }
  return 0;
}

//Example of graph generating
int main(int argc, char** argv){
  if (argc != 6){
    printf("Usage : path to graph created, number of clusters, nodes per cluster, probabilities p and q.\n");
    return -1;
  }
  char* input = argv[1];
  unsigned long nbClus = atoi(argv[2]);
  unsigned long nbNodes = atoi(argv[3]);
  float p = strtof(argv[4],NULL);
  float q = strtof(argv[5],NULL);
  int status = graphGenerator(input,nbClus,nbNodes,p,q);
  printf("Graph generated.\n");
  return status;
}
