#include "adjarray.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

void find_vertex_with_minimal_degree(int* result_vertex, int* result_degree, adjlist* graph, int* degree, char* excluded);
void update_degree(int removed_vertex, adjlist* graph, int* degree);
void compute_degree_array(adjlist* graph, int* degree);


int main(int nbarg, char** argv)
{
    char *input_file, *output_file;
    adjlist* graph;
    char* excluded; // tableau de booléens
    int *k_decomp, *coreness, *degree;
    int k_value = 0;

    const int expected_nbarg = 3;

    // lecture des arguments
    if (nbarg != expected_nbarg)
    {
        printf("Incorrect number of arguments. Received %d arguments instead of %d.\n", nbarg, expected_nbarg);
        printf("Arguments received :\n");
        for(int i = 0 ; i < nbarg ; i++)
            printf("........%s\n", argv[i]);
        return -1;
    }

    input_file = argv[1];
    output_file = argv[2];

    // lecture du graphe
    printf("Reading the input file...\n");
    graph = adjarray_readedgelist(input_file);
    printf("Structuring the data...\n");
    mkadjlist(graph);

    // création d'un tableau de booléen recensant les sommets exclus
    excluded = calloc(graph->n, sizeof(char)); // initialisé à 0

    // création d'un tableau contenant les degrés du graphe mis à jour
    degree = calloc(graph->n, sizeof(int));
    compute_degree_array(graph, degree);

    // création du tableau contenant l'ordre de décomposition
    k_decomp = calloc(graph->n, sizeof(int));
    // création du tableau contenant la coreness de chaque sommet
    coreness = calloc(graph->n, sizeof(int));

    // algorithme de décomposition en noyaux
    printf("Decomposing in k-cores...\n");
    clock_t start, end;
    double time_used;
    start = clock();
    for(int i = graph->n ; i > 0 ; i--)
    {
        if (i%10 == 0)
            printf("%d/%ld\n", i, graph->n);
        int min_vertex;
        int min_degree;
        find_vertex_with_minimal_degree(&min_vertex, &min_degree, graph, degree, excluded);
        if(min_degree > k_value)
            k_value = min_degree;
        k_decomp[min_vertex] = i;
        coreness[min_vertex] = k_value;
        excluded[min_vertex] = 1;
        update_degree(min_vertex, graph, degree);
    }
    end = clock();
    time_used = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("Algorithm executed in %f seconds.\n", time_used);

    // écriture du fichier de résultat
    printf("Writing results to output_file...\n");
    FILE* output_file_handle;
    output_file_handle = fopen(output_file, "w");
    fprintf(output_file_handle, "k_value %d\n", k_value);
    for(int i = 0 ; i < graph->n ; i++)
        fprintf(output_file_handle, "%d %d %d\n", i+1, k_decomp[i], coreness[i]);

    fclose(output_file_handle);

    free(coreness);
    free(k_decomp);
    free(degree);
    free(excluded);
    free_adjlist(graph);
    printf("Done.\n");

    return 0;
}

void find_vertex_with_minimal_degree(int* result_vertex, int* result_degree, adjlist* graph, int* degree, char* excluded)
{
    int min_degree = graph->n;
    int min_vertex = 0;
    for(int i = 0 ; i < graph->n ; i++)
    {
        if(excluded[i])
            continue;

        if(degree[i] < min_degree)
        {
            min_degree = degree[i];
            min_vertex = i;
        }
    }

    *result_vertex = min_vertex;
    *result_degree = min_degree;
    return ;
}

void update_degree(int removed_vertex, adjlist* graph, int* degree)
{
    for (int neighbor = graph-> cd[removed_vertex] ; neighbor < graph->cd[removed_vertex+1] ; neighbor++)
        degree[graph->adj[neighbor]]--;
}

void compute_degree_array(adjlist* graph, int* degree)
{
    for (int i = 0 ; i < graph->n ; i++)
        degree[i] = graph->cd[i+1]-graph->cd[i];
}