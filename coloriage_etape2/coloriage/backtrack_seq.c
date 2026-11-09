/*
 * backtrack_seq.c - Coloriage exact par backtracking (version séquentielle, §8.2)
 *
 * Principe : on colorie les sommets dans l'ordre 0, 1, ..., n-1. Pour chaque sommet,
 * on essaie les couleurs 0, 1, ..., K-1. Si une couleur est compatible avec les voisins
 * déjà coloriés, on passe au sommet suivant (appel récursif). Si aucune couleur ne
 * convient, c'est une impasse : on revient au sommet précédent pour changer sa couleur
 * (retour arrière). Pour trouver χ(G), on essaie K = 1, 2, 3, ... : le premier K qui
 * fonctionne est le nombre chromatique.
 *
 * Usage :
 *   backtrack_seq <fichier.col> [options]
 *   backtrack_seq -g <n> <densite> <graine> [options]     (graphe aléatoire reproductible)
 * Options :
 *   -v              affiche l'arbre de recherche (15 sommets au plus)
 *   -r <nb>         répète la mesure nb fois (défaut : 1)
 *   -csv <fichier>  ajoute une ligne par répétition dans ce fichier CSV
 *   -nom <nom>      nom du graphe dans le CSV (ex. BT1)
 *
 * Exemples :
 *   backtrack_seq graphes/exemple_simple.col -v
 *   backtrack_seq -g 60 0.5 1 -r 5 -csv mesures.csv -nom BT1
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graphe.h"
#include "mesures.h"

/* Compteurs pour analyser la taille de l'arbre de recherche */
static long long nb_noeuds = 0;    /* affectations réalisées = nœuds de l'arbre visités */
static long long nb_retours = 0;   /* affectations annulées = retours en arrière        */
static int verbeux = 0;            /* 1 : afficher l'arbre de recherche (option -v)     */

static void indenter(int profondeur)
{
    for (int i = 0; i < profondeur; i++) printf("  ");
}

/*
 * Fonction de vérification (§8.2, point 2).
 * La couleur c peut-elle être donnée au sommet v sans conflit ?
 * Il suffit de regarder les voisins DÉJÀ coloriés : comme on colorie dans l'ordre,
 * ce sont les sommets u < v. Coût : O(v), donc O(n) au pire (matrice d'adjacence).
 */
static int couleur_possible(const Graphe *g, const int *couleurs, int v, int c)
{
    for (int u = 0; u < v; u++)
        if (adjacent(g, v, u) && couleurs[u] == c)
            return 0;                       /* un voisin a déjà cette couleur */
    return 1;
}

/*
 * Fonction récursive (§8.2, points 3 et 4).
 * Essaie de colorier les sommets v, v+1, ..., n-1 avec au plus K couleurs.
 * Renvoie 1 si elle y arrive (le tableau couleurs contient alors la solution), 0 sinon.
 * Chaque appel correspond à un niveau de l'arbre de recherche : le niveau v.
 */
static int colorier(const Graphe *g, int *couleurs, int v, int K)
{
    if (v == g->n)
        return 1;                           /* feuille : tous les sommets ont une couleur */

    char nom[TAILLE_NOM];
    for (int c = 0; c < K; c++) {
        if (!couleur_possible(g, couleurs, v, c))
            continue;                       /* branche coupée : conflit avec un voisin */

        couleurs[v] = c;                    /* on descend dans l'arbre */
        nb_noeuds++;
        if (verbeux) {
            indenter(v);
            printf("%s = %d\n", graphe_nom(g, v, nom), c);
        }

        if (colorier(g, couleurs, v + 1, K))
            return 1;                       /* solution trouvée plus bas : on remonte */

        couleurs[v] = NON_COLORE;           /* RETOUR ARRIÈRE : on annule ce choix... */
        nb_retours++;                       /* ...et on essaie la couleur suivante    */
        if (verbeux) {
            indenter(v);
            printf("retour arrière : %s perd la couleur %d\n", graphe_nom(g, v, nom), c);
        }
    }

    if (verbeux) {
        indenter(v);
        printf("%s : plus aucune couleur possible -> impasse\n", graphe_nom(g, v, nom));
    }
    return 0;                               /* impasse : le sommet précédent devra changer */
}

/*
 * §8.2, point 1 : une coloration valide avec au plus K couleurs existe-t-elle ?
 */
static int est_k_coloriable(const Graphe *g, int K, int *couleurs)
{
    for (int v = 0; v < g->n; v++)
        couleurs[v] = NON_COLORE;
    return colorier(g, couleurs, 0, K);
}

/*
 * §8.2, point 5 : nombre chromatique.
 * On teste K = 1, 2, 3, ... Le premier K qui fonctionne est χ(G), car pour K-1
 * l'arbre de recherche a été exploré ENTIÈREMENT sans trouver de solution.
 * La boucle s'arrête au plus tard à K = Δ+1 (le glouton prouve que Δ+1 suffit).
 */
static int nombre_chromatique(const Graphe *g, int *couleurs, int afficher)
{
    for (int K = 1; K <= g->n; K++) {
        long long noeuds_avant = nb_noeuds, retours_avant = nb_retours;
        if (verbeux)
            printf("\n--- Essai avec K = %d couleur(s) ---\n", K);

        double t = chrono();
        int possible = est_k_coloriable(g, K, couleurs);
        t = chrono() - t;

        if (afficher)
            printf("  K = %-3d : %-10s  noeuds : %-12lld retours : %-12lld temps : %.4f s\n",
                   K, possible ? "POSSIBLE" : "impossible",
                   nb_noeuds - noeuds_avant, nb_retours - retours_avant, t);
        if (possible)
            return K;
    }
    return g->n;                            /* jamais atteint : n couleurs suffisent toujours */
}

static void usage(const char *programme)
{
    fprintf(stderr,
            "Usage :\n"
            "  %s <fichier.col> [options]\n"
            "  %s -g <n> <densite> <graine> [options]\n"
            "Options :\n"
            "  -v              affiche l'arbre de recherche (15 sommets au plus)\n"
            "  -r <nb>         répète la mesure nb fois\n"
            "  -csv <fichier>  ajoute les mesures dans ce fichier CSV\n"
            "  -nom <nom>      nom du graphe dans le CSV (ex. BT1)\n",
            programme, programme);
    exit(EXIT_FAILURE);
}

/* Nom court d'un fichier : "graphes/examens.col" -> "examens" */
static void nom_depuis_fichier(const char *chemin, char *nom, size_t taille)
{
    const char *debut = chemin;
    for (const char *p = chemin; *p; p++)
        if (*p == '/' || *p == '\\') debut = p + 1;
    snprintf(nom, taille, "%s", debut);
    char *point = strrchr(nom, '.');
    if (point != NULL) *point = '\0';
}

int main(int argc, char **argv)
{
    console_utf8();

    /* ---------- Lecture des arguments ---------- */
    Graphe *g = NULL;
    char nom_graphe[64] = "";
    const char *fichier_csv = NULL;
    int repetitions = 1;
    int i;

    if (argc >= 5 && strcmp(argv[1], "-g") == 0) {
        int n = atoi(argv[2]);
        double densite = atof(argv[3]);
        unsigned long long graine = strtoull(argv[4], NULL, 10);
        g = graphe_aleatoire(n, densite, graine);
        snprintf(nom_graphe, sizeof nom_graphe, "alea_%d_%.2f_%llu", n, densite, graine);
        i = 5;
    } else if (argc >= 2 && argv[1][0] != '-') {
        g = graphe_charger(argv[1]);
        nom_depuis_fichier(argv[1], nom_graphe, sizeof nom_graphe);
        i = 2;
    } else {
        usage(argv[0]);
    }
    if (g == NULL) return EXIT_FAILURE;

    for (; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0)                          verbeux = 1;
        else if (strcmp(argv[i], "-r") == 0 && i + 1 < argc)     repetitions = atoi(argv[++i]);
        else if (strcmp(argv[i], "-csv") == 0 && i + 1 < argc)   fichier_csv = argv[++i];
        else if (strcmp(argv[i], "-nom") == 0 && i + 1 < argc)
            snprintf(nom_graphe, sizeof nom_graphe, "%s", argv[++i]);
        else {
            fprintf(stderr, "Option inconnue ou incomplète : %s\n", argv[i]);
            usage(argv[0]);
        }
    }
    if (repetitions < 1) repetitions = 1;
    if (verbeux && g->n > 15) {
        printf("Option -v ignorée : l'arbre serait trop grand à afficher (n = %d > 15)\n", g->n);
        verbeux = 0;
    }

    /* ---------- Recherche du nombre chromatique ---------- */
    printf("=== Backtracking séquentiel : %s ===\n", nom_graphe);
    graphe_afficher(g);

    int *couleurs = coloration_creer(g->n);
    int chi = 0;
    double somme_temps = 0.0;

    for (int r = 1; r <= repetitions; r++) {
        nb_noeuds = 0;
        nb_retours = 0;
        if (r == 1) printf("\nRecherche du nombre chromatique (K = 1, 2, 3, ...) :\n");

        double t = chrono();
        chi = nombre_chromatique(g, couleurs, r == 1);
        t = chrono() - t;
        somme_temps += t;

        if (repetitions > 1) printf("  Répétition %d : %.4f s\n", r, t);
        if (fichier_csv != NULL)
            mesure_ecrire(fichier_csv, "backtrack", "seq", nom_graphe, g->n, g->m, 1, r, t, chi);
        verbeux = 0;                        /* l'arbre n'est affiché qu'une fois */
    }

    /* ---------- Résultats ---------- */
    printf("\nNombre chromatique : χ(G) = %d\n", chi);
    if (chi > 1)
        printf("Preuve : avec %d couleur(s), l'arbre a été exploré entièrement sans solution ;\n"
               "         avec %d couleurs, une coloration valide a été trouvée.\n", chi - 1, chi);
    printf("Coloration optimale :\n");
    coloration_afficher(g, couleurs);
    printf("Temps %s : %.4f s\n", repetitions > 1 ? "moyen" : "total", somme_temps / repetitions);
    if (fichier_csv != NULL) printf("Mesures ajoutées dans %s\n", fichier_csv);

    int valide = coloration_est_valide(g, couleurs);
    free(couleurs);
    graphe_liberer(g);
    return valide ? EXIT_SUCCESS : EXIT_FAILURE;
}
