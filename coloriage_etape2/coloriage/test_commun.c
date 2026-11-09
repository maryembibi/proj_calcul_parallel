/*
 * test_commun.c - Vérifie que le code commun fonctionne sur la machine de chacun
 *
 * À lancer depuis le dossier du projet :   make test
 * Résultat attendu : « Tous les tests sont passés » et aucune ligne [ÉCHEC].
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "graphe.h"
#include "mesures.h"

/* Nombre d'arêtes de graphe_aleatoire(200, 0.3, 42) : doit être le même partout. */
#define M_REFERENCE_200_03_42 5948L

static int nb_tests = 0, nb_echecs = 0;

#define VERIFIER(condition, message)                                   \
    do {                                                               \
        nb_tests++;                                                    \
        if (condition) printf("  [OK]     %s\n", message);             \
        else { nb_echecs++; printf("  [ÉCHEC]  %s\n", message); }      \
    } while (0)

static void titre(const char *texte) { printf("\n=== %s ===\n", texte); }

static int compter_lignes(const char *chemin)
{
    FILE *f = fopen(chemin, "r");
    if (f == NULL) return -1;
    int lignes = 0, c;
    while ((c = fgetc(f)) != EOF)
        if (c == '\n') lignes++;
    fclose(f);
    return lignes;
}

/* ---------------------------------------------------------------------- */
static void test_openmp(void)
{
    titre("Environnement");
#ifdef _OPENMP
    printf("  OpenMP activé (version %d), %d threads disponibles\n", _OPENMP, nb_threads_max());
    VERIFIER(1, "OpenMP est activé (option -fopenmp)");
#else
    VERIFIER(0, "OpenMP est activé : recompiler avec -fopenmp");
#endif
}

/* ---------------------------------------------------------------------- */
static void test_exemple_simple(void)
{
    titre("Exemple simple : graphe prisme (graphes/exemple_simple.col)");
    Graphe *g = graphe_charger("graphes/exemple_simple.col");
    VERIFIER(g != NULL, "chargement du fichier DIMACS");
    if (g == NULL) return;

    graphe_afficher(g);
    VERIFIER(g->n == 6 && g->m == 9, "n = 6 sommets et m = 9 arêtes");
    VERIFIER(graphe_degre_max(g) == 3, "degré maximal = 3");
    VERIFIER(adjacent(g, 0, 1) && adjacent(g, 1, 0) && !adjacent(g, 0, 3),
             "matrice symétrique (A-B reliés, A-D non reliés)");

    int propose[6] = {0, 1, 2, 0, 1, 2};      /* A,D -> 0 ; B,E -> 1 ; C,F -> 2 */
    printf("Coloriage proposé :\n");
    coloration_afficher(g, propose);
    VERIFIER(coloration_est_valide(g, propose), "le coloriage proposé est valide");
    VERIFIER(coloration_nb_couleurs(propose, g->n) == 3, "il utilise 3 couleurs");

    int faux[6] = {0, 1, 2, 1, 1, 2};         /* D passe en couleur 1, comme B et E */
    printf("Coloriage volontairement faux (D en couleur 1) :\n");
    coloration_afficher(g, faux);
    VERIFIER(!coloration_est_valide(g, faux) && coloration_nb_conflits(g, faux) == 2,
             "les 2 conflits B-D et D-E sont détectés");

    int *incomplet = coloration_creer(g->n);
    incomplet[0] = 0;
    VERIFIER(!coloration_est_valide(g, incomplet), "une coloration incomplète est refusée");

    int *copie = coloration_copier(propose, g->n);
    copie[0] = 1;
    VERIFIER(propose[0] == 0, "coloration_copier donne une copie indépendante");

    free(incomplet);
    free(copie);
    graphe_liberer(g);
}

/* ---------------------------------------------------------------------- */
static void test_examens(void)
{
    titre("Exemple réel : emploi du temps des examens (graphes/examens.col)");
    Graphe *g = graphe_charger("graphes/examens.col");
    VERIFIER(g != NULL, "chargement du fichier DIMACS avec noms");
    if (g == NULL) return;

    graphe_afficher(g);
    VERIFIER(g->n == 7 && g->m == 10, "n = 7 examens et m = 10 incompatibilités");

    /* Ordre du fichier : Maths, Physique, Informatique, Anglais, Économie, Chimie, Histoire */
    int creneaux[7] = {0, 1, 2, 1, 0, 0, 2};
    printf("Répartition en créneaux (couleur = créneau) :\n");
    coloration_afficher(g, creneaux);
    VERIFIER(coloration_est_valide(g, creneaux), "aucun étudiant n'a deux examens dans le même créneau");
    VERIFIER(coloration_nb_couleurs(creneaux, g->n) == 3, "3 créneaux utilisés");
    VERIFIER(adjacent(g, 0, 1) && adjacent(g, 0, 2) && adjacent(g, 1, 2),
             "triangle Maths-Physique-Informatique : au moins 3 créneaux nécessaires");
    graphe_liberer(g);
}

/* ---------------------------------------------------------------------- */
static void test_chemin(void)
{
    titre("Chemin a-b-c-d (graphes/chemin_p4.col)");
    Graphe *g = graphe_charger("graphes/chemin_p4.col");
    VERIFIER(g != NULL, "chargement du fichier");
    if (g == NULL) return;

    int ordre_abcd[4] = {0, 1, 0, 1};         /* glouton dans l'ordre a, b, c, d */
    int ordre_adbc[4] = {0, 1, 2, 0};         /* glouton dans l'ordre a, d, b, c */
    VERIFIER(coloration_est_valide(g, ordre_abcd) && coloration_nb_couleurs(ordre_abcd, 4) == 2,
             "ordre a, b, c, d : coloriage valide à 2 couleurs");
    VERIFIER(coloration_est_valide(g, ordre_adbc) && coloration_nb_couleurs(ordre_adbc, 4) == 3,
             "ordre a, d, b, c : coloriage valide mais à 3 couleurs");
    graphe_liberer(g);
}

/* ---------------------------------------------------------------------- */
static void test_generateur(void)
{
    titre("Générateur reproductible");
    const int n = 200;
    Graphe *g1 = graphe_aleatoire(n, 0.3, 42);
    Graphe *g2 = graphe_aleatoire(n, 0.3, 42);
    Graphe *g3 = graphe_aleatoire(n, 0.3, 43);
    size_t taille = (size_t)n * n;

    printf("  graphe_aleatoire(200, 0.3, 42) : m = %ld arêtes\n", g1->m);
    VERIFIER(memcmp(g1->adj, g2->adj, taille) == 0, "même graine => même graphe");
    VERIFIER(memcmp(g1->adj, g3->adj, taille) != 0, "graine différente => graphe différent");

    long attendu = (long)(0.3 * n * (n - 1) / 2);   /* 5970 arêtes en moyenne */
    long ecart = g1->m > attendu ? g1->m - attendu : attendu - g1->m;
    VERIFIER(ecart < attendu / 20, "nombre d'arêtes proche de densité x n(n-1)/2 (à 5 % près)");
    VERIFIER(M_REFERENCE_200_03_42 < 0 || g1->m == M_REFERENCE_200_03_42,
             "même nombre d'arêtes que sur les autres machines du groupe");

    VERIFIER(graphe_sauver(g1, "graphe_test.col"), "sauvegarde au format DIMACS");
    Graphe *g4 = graphe_charger("graphe_test.col");
    VERIFIER(g4 != NULL && g4->m == g1->m && memcmp(g1->adj, g4->adj, taille) == 0,
             "rechargement identique au graphe sauvegardé");
    remove("graphe_test.col");

    graphe_liberer(g1);
    graphe_liberer(g2);
    graphe_liberer(g3);
    graphe_liberer(g4);
}

/* ---------------------------------------------------------------------- */
static void test_mesures_et_trace(void)
{
    titre("Fichier de mesures et trace pour Gantt");

    remove("mesures_test.csv");
    mesure_ecrire("mesures_test.csv", "glouton", "seq", "G1", 1000, 249750, 1, 1, 0.0123, 112);
    mesure_ecrire("mesures_test.csv", "glouton", "par", "G1", 1000, 249750, 4, 1, 0.0045, 112);
    VERIFIER(compter_lignes("mesures_test.csv") == 3, "CSV des mesures : 1 en-tête + 2 lignes");
    remove("mesures_test.csv");

    int equipe = 1;
    trace_demarrer(100);
    #pragma omp parallel num_threads(4)
    {
        double debut = chrono();
        volatile double x = 0.0;
        for (int i = 0; i < 200000; i++) x += i * 0.5;      /* petit travail factice */
        char etiquette[32];
        snprintf(etiquette, sizeof etiquette, "tache du thread %d", numero_thread());
        trace_ajouter(debut, chrono(), etiquette);
#ifdef _OPENMP
        #pragma omp single
        equipe = omp_get_num_threads();
#endif
    }
    printf("  %d threads ont enregistré %d tâches\n", equipe, trace_nb_evenements());
    VERIFIER(trace_nb_evenements() == equipe, "une tâche tracée par thread, sans collision");
    VERIFIER(trace_ecrire_csv("trace_test.csv") && compter_lignes("trace_test.csv") == equipe + 1,
             "CSV de trace : 1 en-tête + 1 ligne par tâche");
    remove("trace_test.csv");
    trace_arreter();
}

/* ---------------------------------------------------------------------- */
int main(void)
{
    console_utf8();
    test_openmp();
    test_exemple_simple();
    test_examens();
    test_chemin();
    test_generateur();
    test_mesures_et_trace();

    printf("\n%d tests, %d échec(s)\n", nb_tests, nb_echecs);
    if (nb_echecs == 0)
        printf("Tous les tests sont passés : le code commun fonctionne sur cette machine.\n");
    return nb_echecs == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
