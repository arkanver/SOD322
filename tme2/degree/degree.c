#include "edgelist.h"
#include <stdlib.h>
#include <stdio.h>

int main(int nbarg, char** argv)
{
    char *input_file, *output_file;
    edgelist* graph;
    int* in_degree;
    int* out_degree;
    int first_node;

    double alpha;
    double threshold;

    // lecture des arguments
    if (nbarg != 4)
        return -1;

    input_file = argv[1];
    output_file = argv[2];
    first_node = atoi(argv[3]);

    // lecture du graphe
    graph = edgelist_readedgelist(input_file);

    // allocation de la mémoire pour les tableaux de degré
    in_degree = calloc(graph->n, sizeof(int));  //initialisé à 0
    out_degree = calloc(graph->n, sizeof(int));  //initialisé à 0

    // itération sur les arêtes pour calculer le degré
    for(int i = 0 ; i < graph->e ; i++)
    {
        in_degree[graph->edges[i].t-first_node] ++;
        out_degree[graph->edges[i].s-first_node] ++;
    }

    // écriture du résultat dans le fichier de sortie
    FILE* output_file_handle;
    output_file_handle = fopen(output_file, "w");
    for(int i = 0 ; i < graph->n ; i++)
        fprintf(output_file_handle, "%d %d\n", in_degree[i], out_degree[i]);
    fclose(output_file_handle);

    // libération de la mémoire
    free_edgelist(graph);
    free(in_degree);
    free(out_degree);

    return 0;
}