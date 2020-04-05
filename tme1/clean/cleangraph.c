/* File that cleans the data - no self-loops, no "symmetric" edges*/
#include <stdlib.h>
#include <stdio.h>
#include <time.h>//to estimate the running time

void cleangraph(char* input, char* output){
  FILE *file=fopen(input,"r");
  if (file==NULL){
    printf("Unable to open file ! Terminating.\n");
  }
  FILE *file2=fopen(output,"w");

  long unsigned int* a;
  long unsigned int* b;

  //First iteration outside loop
  if (fscanf(file, "%lu %lu", &a, &b) != 2){
    //Then skip first four lines (com graphs)
    char buffer[100];
    fgets(buffer, 100, file);
    fgets(buffer, 100, file);
    fgets(buffer, 100, file);
    fgets(buffer, 100, file);
  }
  else {
    if (a < b){
      fprintf(file2,"%lu %lu\n", a, b);
    }
  }


  while (fscanf(file,"%lu %lu", &a, &b)==2){
    if (a < b){
      fprintf(file2,"%lu %lu\n", a, b);
    }
  }
}

int main(int argc, char** argv){
  char* input = argv[1];
  char* output = argv[2];

  FILE *file;

  if ((file = fopen(output, "r"))){
    if (output != input){ //Else, override of an existing file is permitted
      printf("Output file already exists !\n");
      return -1;
    }
  }

  //Evaluate running time
  clock_t start = clock();
  cleangraph(input,output);
  clock_t end = clock();
  double runtime = (double)(end-start)/CLOCKS_PER_SEC;
  printf("CPU time : %f", runtime);
  return 0;
}
