/*
 * graphe.c - Implémentation du code commun (voir graphe.h)
 */
#include "graphe.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Allocation qui arrête le programme proprement si la mémoire manque. */
static void *allouer_zero(size_t nb, size_t taille)
{
    void *p = calloc(nb, taille);
    if (p == NULL) {
        fprintf(stderr, "Erreur : mémoire insuffisante (%.0f octets demandés)\n",
                (double)nb * (double)taille);
        exit(EXIT_FAILURE);
    }
    return p;
}

/* ====================================================================== */
/*  Construction                                                          */
/* ====================================================================== */

Graphe *graphe_creer(int n)
{
    if (n <= 0) {
        fprintf(stderr, "Erreur : nombre de sommets invalide (%d)\n", n);
        return NULL;
    }
    Graphe *g = allouer_zero(1, sizeof *g);
    g->n = n;
    g->m = 0;
    g->adj = allouer_zero((size_t)n * (size_t)n, sizeof(unsigned char));
    g->noms = NULL;
    return g;
}

void graphe_liberer(Graphe *g)
{
    if (g == NULL) return;
    free(g->adj);
    free(g->noms);
    free(g);
}

int graphe_ajouter_arete(Graphe *g, int u, int v)
{
    if (u < 0 || u >= g->n || v < 0 || v >= g->n) {
        fprintf(stderr, "Erreur : arête %d-%d hors limites (n = %d)\n", u, v, g->n);
        return 0;
    }
    if (u == v) return 0;                 /* pas de boucle dans un graphe simple */
    if (adjacent(g, u, v)) return 0;      /* arête déjà présente                 */
    g->adj[(size_t)u * g->n + v] = 1;     /* graphe non orienté : matrice        */
    g->adj[(size_t)v * g->n + u] = 1;     /* symétrique                          */
    g->m++;
    return 1;
}

void graphe_nommer(Graphe *g, int v, const char *nom)
{
    if (v < 0 || v >= g->n) return;
    if (g->noms == NULL)
        g->noms = allouer_zero((size_t)g->n, sizeof *g->noms);
    strncpy(g->noms[v], nom, TAILLE_NOM - 1);
    g->noms[v][TAILLE_NOM - 1] = '\0';
}

/* ====================================================================== */
/*  Informations                                                          */
/* ====================================================================== */

int graphe_degre(const Graphe *g, int v)
{
    int d = 0;
    for (int u = 0; u < g->n; u++)
        d += adjacent(g, v, u);
    return d;
}

int graphe_degre_max(const Graphe *g)
{
    int dmax = 0;
    for (int v = 0; v < g->n; v++) {
        int d = graphe_degre(g, v);
        if (d > dmax) dmax = d;
    }
    return dmax;
}

const char *graphe_nom(const Graphe *g, int v, char tampon[TAILLE_NOM])
{
    if (g->noms != NULL && g->noms[v][0] != '\0')
        return g->noms[v];
    snprintf(tampon, TAILLE_NOM, "%d", v);
    return tampon;
}

/* Étiquette d'une ligne ou colonne de la matrice : le nom court ou le numéro. */
static const char *entete(const Graphe *g, int v, int noms_courts, char tampon[TAILLE_NOM])
{
    if (noms_courts) return g->noms[v];
    snprintf(tampon, TAILLE_NOM, "%d", v);
    return tampon;
}

void graphe_afficher(const Graphe *g)
{
    double paires = (double)g->n * (g->n - 1) / 2.0;
    printf("Graphe : n = %d sommets, m = %ld arêtes, degré max = %d, densité = %.3f\n",
           g->n, g->m, graphe_degre_max(g), paires > 0 ? g->m / paires : 0.0);

    if (g->n > 20) return;   /* au-delà, la matrice ne tient pas à l'écran */

    /* En-têtes courts : les noms s'ils font au plus 3 octets, sinon les numéros. */
    int noms_courts = (g->noms != NULL);
    for (int v = 0; noms_courts && v < g->n; v++)
        if (strlen(g->noms[v]) == 0 || strlen(g->noms[v]) > 3) noms_courts = 0;

    char tampon[TAILLE_NOM];
    if (!noms_courts && g->noms != NULL) {
        printf("Légende :");
        for (int v = 0; v < g->n; v++) printf(" %d=%s", v, graphe_nom(g, v, tampon));
        printf("\n");
    }

    printf("Matrice d'adjacence :\n    ");
    for (int v = 0; v < g->n; v++)
        printf("%4s", entete(g, v, noms_courts, tampon));
    printf("   degré\n");
    for (int u = 0; u < g->n; u++) {
        printf("%4s", entete(g, u, noms_courts, tampon));
        for (int v = 0; v < g->n; v++) printf("%4d", adjacent(g, u, v));
        printf("   %d\n", graphe_degre(g, u));
    }
}

/* ====================================================================== */
/*  Fichiers DIMACS                                                       */
/* ====================================================================== */

Graphe *graphe_charger(const char *chemin)
{
    FILE *f = fopen(chemin, "r");
    if (f == NULL) {
        perror(chemin);
        return NULL;
    }

    Graphe *g = NULL;
    long m_annonce = -1;
    int num_ligne = 0;
    char ligne[512];

    while (fgets(ligne, sizeof ligne, f) != NULL) {
        num_ligne++;
        char *p = ligne;
        while (*p == ' ' || *p == '\t') p++;

        if (*p == 'p') {                                   /* p edge n m */
            char format[16];
            int n;
            long m;
            if (g != NULL || sscanf(p, "p %15s %d %ld", format, &n, &m) != 3 || n <= 0) {
                fprintf(stderr, "%s:%d : ligne « p » invalide ou en double\n", chemin, num_ligne);
                goto erreur;
            }
            g = graphe_creer(n);
            m_annonce = m;
        } else if (*p == 'e') {                            /* e u v (1-indexé) */
            int u, v;
            if (g == NULL) {
                fprintf(stderr, "%s:%d : arête avant la ligne « p »\n", chemin, num_ligne);
                goto erreur;
            }
            if (sscanf(p, "e %d %d", &u, &v) != 2 || u < 1 || u > g->n || v < 1 || v > g->n) {
                fprintf(stderr, "%s:%d : arête invalide\n", chemin, num_ligne);
                goto erreur;
            }
            graphe_ajouter_arete(g, u - 1, v - 1);
        } else if (*p == 'c') {                            /* commentaire ou nom */
            int id;
            char nom[TAILLE_NOM];
            if (g != NULL && sscanf(p, "c nom %d %31s", &id, nom) == 2 && id >= 1 && id <= g->n)
                graphe_nommer(g, id - 1, nom);
        }
        /* lignes vides ou inconnues : ignorées */
    }
    fclose(f);

    if (g == NULL) {
        fprintf(stderr, "%s : ligne « p edge n m » manquante\n", chemin);
        return NULL;
    }
    if (m_annonce != g->m)
        fprintf(stderr, "Attention (%s) : %ld arêtes annoncées, %ld arêtes distinctes lues\n",
                chemin, m_annonce, g->m);
    return g;

erreur:
    fclose(f);
    graphe_liberer(g);
    return NULL;
}

int graphe_sauver(const Graphe *g, const char *chemin)
{
    FILE *f = fopen(chemin, "w");
    if (f == NULL) {
        perror(chemin);
        return 0;
    }
    fprintf(f, "c Graphe du projet coloriage (format DIMACS, sommets numérotés à partir de 1)\n");
    fprintf(f, "p edge %d %ld\n", g->n, g->m);
    if (g->noms != NULL)
        for (int v = 0; v < g->n; v++)
            if (g->noms[v][0] != '\0') fprintf(f, "c nom %d %s\n", v + 1, g->noms[v]);
    for (int u = 0; u < g->n; u++)
        for (int v = u + 1; v < g->n; v++)
            if (adjacent(g, u, v)) fprintf(f, "e %d %d\n", u + 1, v + 1);
    fclose(f);
    return 1;
}

/* ====================================================================== */
/*  Générateur reproductible                                              */
/* ====================================================================== */

/* splitmix64 : générateur simple, rapide et identique sur toutes les machines. */
unsigned long long alea_suivant(unsigned long long *etat)
{
    unsigned long long z = (*etat += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

double alea_reel(unsigned long long *etat)
{
    /* 53 bits aléatoires ramenés dans [0, 1) */
    return (double)(alea_suivant(etat) >> 11) * (1.0 / 9007199254740992.0);
}

Graphe *graphe_aleatoire(int n, double densite, unsigned long long graine)
{
    Graphe *g = graphe_creer(n);
    if (g == NULL) return NULL;
    if (densite < 0.0) densite = 0.0;
    if (densite > 1.0) densite = 1.0;

    unsigned long long etat = graine;
    for (int u = 0; u < n; u++)              /* l'ordre de parcours fixe le résultat */
        for (int v = u + 1; v < n; v++)
            if (alea_reel(&etat) < densite)
                graphe_ajouter_arete(g, u, v);
    return g;
}

/* ====================================================================== */
/*  Colorations                                                           */
/* ====================================================================== */

int *coloration_creer(int n)
{
    int *c = malloc((size_t)n * sizeof *c);
    if (c == NULL) {
        fprintf(stderr, "Erreur : mémoire insuffisante pour la coloration\n");
        exit(EXIT_FAILURE);
    }
    for (int v = 0; v < n; v++) c[v] = NON_COLORE;
    return c;
}

int *coloration_copier(const int *couleurs, int n)
{
    int *c = coloration_creer(n);
    memcpy(c, couleurs, (size_t)n * sizeof *c);
    return c;
}

long coloration_nb_conflits(const Graphe *g, const int *couleurs)
{
    long conflits = 0;
    for (int u = 0; u < g->n; u++)
        for (int v = u + 1; v < g->n; v++)
            if (adjacent(g, u, v) && couleurs[u] != NON_COLORE && couleurs[u] == couleurs[v])
                conflits++;
    return conflits;
}

int coloration_est_valide(const Graphe *g, const int *couleurs)
{
    for (int v = 0; v < g->n; v++)
        if (couleurs[v] < 0) return 0;         /* sommet non colorié */
    return coloration_nb_conflits(g, couleurs) == 0;
}

int coloration_nb_couleurs(const int *couleurs, int n)
{
    int cmax = -1;
    for (int v = 0; v < n; v++)
        if (couleurs[v] > cmax) cmax = couleurs[v];
    if (cmax < 0) return 0;

    unsigned char *vue = allouer_zero((size_t)cmax + 1, 1);
    int nb = 0;
    for (int v = 0; v < n; v++)
        if (couleurs[v] >= 0 && !vue[couleurs[v]]) {
            vue[couleurs[v]] = 1;
            nb++;
        }
    free(vue);
    return nb;
}

void coloration_afficher(const Graphe *g, const int *couleurs)
{
    char tampon[TAILLE_NOM];
    int nb = coloration_nb_couleurs(couleurs, g->n);

    if (g->n <= 50) {
        int cmax = -1;
        for (int v = 0; v < g->n; v++)
            if (couleurs[v] > cmax) cmax = couleurs[v];
        for (int c = 0; c <= cmax; c++) {
            int present = 0;
            for (int v = 0; v < g->n; v++)
                if (couleurs[v] == c) {
                    if (!present) printf("  Couleur %d :", c);
                    printf(" %s", graphe_nom(g, v, tampon));
                    present = 1;
                }
            if (present) printf("\n");
        }
        for (int v = 0; v < g->n; v++)
            if (couleurs[v] == NON_COLORE) printf("  Non colorié : %s\n", graphe_nom(g, v, tampon));
    }
    printf("  Nombre de couleurs : %d | conflits : %ld | coloration %s\n", nb,
           coloration_nb_conflits(g, couleurs),
           coloration_est_valide(g, couleurs) ? "VALIDE" : "NON VALIDE");
}
