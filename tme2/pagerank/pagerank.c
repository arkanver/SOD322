#include "edgelist.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

int main(int nbarg, char** argv)
{
    char *input_file, *output_file, *history_file;
    edgelist* graph;
    double* eigenvector;
    int* degree;

    double alpha;
    double threshold;

    // lecture des arguments
    if (nbarg != 6)
    {
        printf("Incorrect number of arguments. Received %d arguments instead of 6.\n", nbarg);
        printf("Arguments received :\n");
        for(int i = 0 ; i < nbarg ; i++)
            printf("........%s\n", argv[i]);
        return -1;
    }

    input_file = argv[1];
    output_file = argv[2];
    alpha = atof(argv[3]);
    threshold = atof(argv[4]);
    history_file = argv[5];

    // lecture du graphe
    graph = edgelist_readedgelist(input_file);

    // initialisation de la graine
    srand(time (NULL));

    // initialisation aléatoire du vecteur
    eigenvector = calloc(graph->n, sizeof(double));
    for(int i = 0 ; i < graph->n ; i++)
        eigenvector[i] = rand()/RAND_MAX;

    // calcul des degrés sortants
    degree = calloc(graph->n, sizeof(int));
    for(int i = 0 ; i < graph->e ; i++)
        degree[graph->edges[i].s-1]++ ;

    // allocation d'un vecteur de résultat intermédiaire
    double* resultat;
    resultat = calloc(graph->n, sizeof(double));

    double diff = 1e8;
    int itermax = 0;

    FILE* history_file_handle = fopen(history_file, "w");

    // itérations de l'algorithme de calcul du vecteur propre
    while(itermax++, diff > threshold)
    {
        // initialisation à zéro du vecteur résultat
        for(int i = 0 ; i < graph->n ; i++)
            resultat[i] = 0.0;

        // itération sur les arêtes pour calculer le produit matriciel
        for(int i = 0 ; i < graph->e ; i++)
        {
            int deg_out;
            float coefficient;
            deg_out = degree[graph->edges[i].s-1];

            if (deg_out == 0)
                coefficient = 1/(graph->n);
            else
                coefficient = deg_out;
            resultat[graph->edges[i].t-1] += coefficient*eigenvector[graph->edges[i].s-1];
        }

        // itération sur les sommets pour mettre à jour le résultat et calculer sa norme
        double norme = 0.0;
        for(int i = 0 ; i < graph->n ; i++)
        {
            resultat[i] = (1-alpha)*resultat[i] + alpha/(graph->n);
            norme += resultat[i]*resultat[i];
        }
        norme = sqrt(norme);

        // itération sur les sommets pour normaliser le vecteur de résultat, calculer l'écart à l'itération précédente et enregistrer le résultat
        diff = 0.0;
        for(int i = 0 ; i < graph->n ; i++)
        {
            resultat[i] = resultat[i]/norme;
            diff += (eigenvector[i]-resultat[i])*(eigenvector[i]-resultat[i]);
            eigenvector[i] = resultat[i];
        }
        diff = sqrt(diff);
        fprintf(history_file_handle, "%e\n", diff);
    }
    fclose(history_file_handle);

    // enregistrement du vecteur de pagerank dans le fichier de sortie
    printf("Writing result in %s\n", output_file);
    FILE* output_file_handle = fopen(output_file, "w");
    for(int i = 0 ; i < graph->n ; i++)
        fprintf(output_file_handle, "%e\n", eigenvector[i]);
    fclose(output_file_handle);

    // libération de la mémoire
    free_edgelist(graph);
    free(eigenvector);
    free(degree);
    free(resultat);

    printf("Convergence reached up to %.2e after %d iterations.\n", threshold, itermax);

    return 0;
}