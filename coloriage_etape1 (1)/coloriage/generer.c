/*
 * generer.c - Génère un graphe aléatoire reproductible et l'enregistre au format DIMACS
 *
 * Usage   : ./generer <n> <densite> <graine> <fichier.col>
 * Exemple : ./generer 30 0.5 1 graphes/bt_30.col
 *
 * Même n, même densité et même graine => même fichier sur toutes les machines.
 */
#include <stdio.h>
#include <stdlib.h>

#include "graphe.h"
#include "mesures.h"

int main(int argc, char **argv)
{
    console_utf8();
    if (argc != 5) {
        fprintf(stderr, "Usage : %s <n> <densite entre 0 et 1> <graine> <fichier.col>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int n = atoi(argv[1]);
    double densite = atof(argv[2]);
    unsigned long long graine = strtoull(argv[3], NULL, 10);

    double t = chrono();
    Graphe *g = graphe_aleatoire(n, densite, graine);
    t = chrono() - t;
    if (g == NULL) return EXIT_FAILURE;

    graphe_afficher(g);
    if (!graphe_sauver(g, argv[4])) {
        graphe_liberer(g);
        return EXIT_FAILURE;
    }
    printf("Graphe enregistré dans %s (généré en %.3f s)\n", argv[4], t);
    graphe_liberer(g);
    return EXIT_SUCCESS;
}
