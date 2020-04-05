//Function that prints the number of vertices and edges found in graph.
#define N_VERT 100000000
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

void neCount(char* input){

  //Evaluate running time
  clock_t start = clock();

  unsigned long n=0;
  unsigned long max=0;
  bool *id = malloc(N_VERT*sizeof(bool));
  unsigned long e=0;
  FILE *file=fopen(input,"r");

  unsigned long s;
  unsigned long t;

  printf("Reading graph...\n");


  //First iteration outside loop
  if (fscanf(file, "%lu %lu", &s, &t) != 2){
    //Then skip first four lines
    char buffer[100];
    fgets(buffer, 100, file);
    fgets(buffer, 100, file);
    fgets(buffer, 100, file);
    fgets(buffer, 100, file);
  }
  else{
    e++;

    if (id[s] == false){
      id[s] = true;
      n++;
    }

    if (id[t] == false){
      id[t] = true;
      n++;
    }
  }

  while (fscanf(file,"%lu %lu", &s, &t)==2){
    e++;

    if (id[s] == false){
      id[s] = true;
      n++;
      if (n==N_VERT){
        id = realloc(id, n*sizeof(bool));
      }
    }

    if (id[t] == false){
      id[t] = true;
      n++;
      if (n==N_VERT){
        id = realloc(id, n*sizeof(bool));
      }
    }
  }
  clock_t end = clock();
  double runtime = (double)(end-start)/CLOCKS_PER_SEC;

  free(id);
  printf("Number of vertices : %lu\n", n);
  printf("Number of edges : %lu\n", e);
  printf("Running time : %f\n", runtime);
}


int main(int argc, char** argv){
  char* input = argv[1];
  neCount(input);
  return 0;
}
