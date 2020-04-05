#include "adjarray.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


void update_degree(int removed_vertex, adjlist* graph, int *heap, int *heap_to_list, int *list_to_heap, char* removed_vertices);

void compute_degree_array(adjlist* graph, int* degree);

void create_heap(adjlist* graph, int *heap, int *heap_to_list, int *list_to_heap);

void insert_in_heap(adjlist* graph, int value, int list_index, int *heap, int *heap_to_list, int* list_to_heap);

void remove_from_heap(adjlist* graph, int heap_index, int *heap, int *heap_to_list, int *list_to_heap);

void sift_down(int heap_index, int *heap, int *heap_to_list, int *list_to_heap);

void sift_up(int heap_index, int *heap, int *heap_to_list, int* list_to_heap);

int check_heap(adjlist* graph, int* heap, int* heap_to_list, int* list_to_heap, int verbose);


int main(int nbarg, char** argv)
{
    char *input_file, *output_file;
    adjlist* graph;
    int *heap, *heap_to_list, *list_to_heap; // tableaux supportant la structure de tas
    char *removed_vertices; // tableau de booléen
    int k_value = 0;

    clock_t start, start_algo, end;
    double time_total, time_algo;

    start = clock();

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
    printf("Creation of an adjacency list...\n");
    mkadjlist(graph);

    // création d'un tableau servant de support à une structure de tas sur le degré
    // la valeur heap[0] coreespond à l'indice du dernier noeud du tas
    // le tas commence à 1
    heap = calloc((graph->n)+1, sizeof(int));

    // création des tableaux permettant de passer de la représentation des degrés sous forme de liste à la représentation des degrés sous forme de tas
    // le tableau heap_to_list doit comporter une case de plus car les indices du tas commencent à 1
    heap_to_list = calloc((graph->n)+1, sizeof(int));
    list_to_heap = calloc(graph->n, sizeof(int));

    // initialisation des trois tableaux précédents
    printf("Creation of the heap structure...\n");
    create_heap(graph, heap, heap_to_list, list_to_heap);
    if(check_heap(graph, heap, heap_to_list, list_to_heap, 1)==-1)
        exit(-1);

    // création d'un tableau de booléens indiquant les arêtes exclues
    removed_vertices = calloc(graph->n, sizeof(char)); // automatiquement initialisé à 0
    
    // création du tableau contenant l'ordre de décomposition
    FILE* output_file_handle;
    output_file_handle = fopen(output_file, "w");

    // algorithme de décomposition en noyaux
    printf("Decomposing in k-cores...\n");

    // profiling
    start_algo = clock();

    for(int i = graph->n ; i > 0 ; i--)
    {
        if (i%(graph->n/20) == 0)
            printf("%d/%ld\n", i, graph->n);

        int min_vertex = heap_to_list[1]; // on récupère l'indice de la racine de l'arbre
        int min_degree = heap[1]; // on récupère la racine de l'arbre
        if(min_degree > k_value)
            k_value = min_degree;

        fprintf(output_file_handle, "%d %d %d\n", i, min_vertex+1, k_value);

        remove_from_heap(graph, 1, heap, heap_to_list, list_to_heap); // on retire la racine de l'arbre
        update_degree(min_vertex, graph, heap, heap_to_list, list_to_heap, removed_vertices);
        removed_vertices[min_vertex] = 1;


    }

    // profiling
    end = clock();
    time_algo = ((double)(end-start_algo))/CLOCKS_PER_SEC;
    time_total = ((double)(end-start))/CLOCKS_PER_SEC;
    printf("Algorithm executed in %f seconds.\n", time_algo);

    // on ajoute la k-valeur et le temps d'exécution à la fin du fichier de sortie
    fprintf(output_file_handle, "k_value %d\n", k_value);
    fprintf(output_file_handle, "algorithm_time %f\n", time_algo);
    fprintf(output_file_handle, "execution_time %f\n", time_total);
    fclose(output_file_handle);

    // libération de la mémoire
    free(heap);
    free(list_to_heap);
    free(heap_to_list);
    free(removed_vertices);
    free_adjlist(graph);

    printf("Done.\n");

    return 0;
}

void update_degree(int removed_vertex, adjlist* graph, int *heap, int *heap_to_list, int *list_to_heap, char* removed_vertices)
{
    // if(check_heap(graph, heap, heap_to_list, list_to_heap,0)==-1)
    // {
    //     exit(-1);
    // }
    // printf("hi!\n");
    for (int neighbor = graph->cd[removed_vertex] ; neighbor < graph->cd[removed_vertex+1] ; neighbor++)
    {
        if(removed_vertices[graph->adj[neighbor]])
            continue;
        int heap_index = list_to_heap[graph->adj[neighbor]];
        heap[heap_index]--;
        sift_up(heap_index, heap, heap_to_list, list_to_heap);
    }
}

void create_heap(adjlist* graph, int *heap, int *heap_to_list, int *list_to_heap)
{
    // Cette fonction doit être optimisée en utilisant l'algorithme de Floyd.
    // Même non optimisée, elle s'exécute en O(n*log(n)), ce qui suffit pour les graphes traitées
    // On ne la prend pas en compte dans la mesure de la durée d'exécution
    heap[0] = 0;
    for (int i = 0 ; i < graph->n ; i++)
    {
        int degree = graph->cd[i+1]-graph->cd[i];
        insert_in_heap(graph, degree, i, heap, heap_to_list, list_to_heap);
    }
}

void insert_in_heap(adjlist* graph, int value, int list_index, int *heap, int *heap_to_list, int *list_to_heap)
{
    heap[0]++;
    int heap_index = heap[0];
    heap_to_list[heap_index] = list_index;
    list_to_heap[list_index] = heap_index;
    heap[heap_index] = value;
    sift_up(heap_index, heap, heap_to_list, list_to_heap);
}

void remove_from_heap(adjlist* graph, int heap_index, int *heap, int *heap_to_list, int *list_to_heap)
{
    heap[heap_index] = heap[heap[0]];
    heap_to_list[heap_index] = heap_to_list[heap[0]];
    list_to_heap[heap_to_list[heap_index]] = heap_index;
    heap[0]--;

    sift_down(heap_index, heap, heap_to_list, list_to_heap);
}

void sift_down(int heap_index, int *heap, int *heap_to_list, int *list_to_heap)
{
    for(;;)
    {
        if (2*heap_index+1 <= heap[0] && heap[heap_index] > heap[2*heap_index+1] && heap[2*heap_index] > heap[2*heap_index+1])
        {
            int swapper = heap[heap_index];
            heap[heap_index] = heap[2*heap_index+1];
            heap[2*heap_index+1] = swapper;

            swapper = heap_to_list[heap_index];
            heap_to_list[heap_index] = heap_to_list[2*heap_index+1];
            heap_to_list[2*heap_index+1] = swapper;

            list_to_heap[heap_to_list[heap_index]] = heap_index;
            list_to_heap[heap_to_list[2*heap_index+1]] = 2*heap_index+1;

            heap_index = 2*heap_index+1;
        }
        else if (2*heap_index <= heap[0] && heap[heap_index] > heap[2*heap_index])
        {
            int swapper = heap[heap_index];
            heap[heap_index] = heap[2*heap_index];
            heap[2*heap_index] = swapper;

            swapper = heap_to_list[heap_index];
            heap_to_list[heap_index] = heap_to_list[2*heap_index];
            heap_to_list[2*heap_index] = swapper;

            list_to_heap[heap_to_list[heap_index]] = heap_index;
            list_to_heap[heap_to_list[2*heap_index]] = 2*heap_index;

            heap_index = 2*heap_index;
        }
        else
            return;
    }
}

void sift_up(int heap_index, int *heap, int *heap_to_list, int *list_to_heap)
{
    while(heap_index > 1 && heap[heap_index/2] > heap[heap_index])
    {
        int swapper = heap[heap_index/2];
        // on échange les valeurs dans le tas
        heap[heap_index/2] = heap[heap_index];
        heap[heap_index] = swapper;

        // on échange les valeurs dans la structure heap_to_list
        swapper = heap_to_list[heap_index];
        heap_to_list[heap_index] = heap_to_list[heap_index/2];
        heap_to_list[heap_index/2] = swapper;

        // on échange les valeurs dans la structure list_to_heap
        swapper = list_to_heap[heap_to_list[heap_index]];
        list_to_heap[heap_to_list[heap_index]] = list_to_heap[heap_to_list[heap_index/2]];
        list_to_heap[heap_to_list[heap_index/2]] = swapper;
        heap_index = heap_index/2;
    }
}

int check_heap(adjlist* graph, int* heap, int* heap_to_list, int* list_to_heap, int verbose)
{
    int return_value = 0;
    for(int i = 1 ; i <= heap[0] ; i ++)
        if((2*i <= heap[0] && heap[i]>heap[2*i])||(2*i+1<=heap[0]&&heap[i]>heap[2*i+1]))
        {
            if(verbose>=1)
                printf("heap property broken at node %d\n", i);
                printf("parent : (%d,%d), left child : (%d, %d), right child = (%d,%d)\n", i, heap[i], 2*i, heap[2*i], 2*i+1, heap[2*i+1]);
            return_value = -1;
        }

    for (int i = 1 ; i <= heap[0] ; i++)
        if (list_to_heap[heap_to_list[i]] != i)
        {
            if(verbose>=1)
                printf("mappings are not involutive at node %d/%d, list index %d, involution %d %d, value at node %d\n", i, heap[0], heap_to_list[i], list_to_heap[heap_to_list[i]], list_to_heap[0], heap[i]);
            return_value = -1;
        }

    for (int i = 0 ; i < graph->n ; i++)
    {
        int degree = graph->cd[i+1]-graph->cd[i];
        if(heap[list_to_heap[i]] != degree && verbose>=2)
        {
            printf("list_to_heap[%d] is wrong : %d vs %d\n", i, heap[list_to_heap[i]], i);
            return_value = -1;
        }
    }

    for (int i = 1 ; i <= graph->n ; i++)
    {
        int index = heap_to_list[i];
        int degree = graph->cd[index+1]-graph->cd[index];
        if(heap[i] != degree && verbose>=2)
        {
            printf("heap_to_list[%d] is wrong : %d vs %d\n", i, heap[i], degree);
            return_value = -1;
        }
    }

    return return_value;
}