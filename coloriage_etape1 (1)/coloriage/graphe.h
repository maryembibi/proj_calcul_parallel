/*
 * graphe.h - Code commun du projet « Coloriage de graphes »
 *
 * Représentation : matrice d'adjacence n x n (unsigned char, 1 octet par case).
 * Conventions partagées par tout le groupe :
 *   - les sommets sont numérotés de 0 à n-1 ;
 *   - les couleurs sont des entiers de 0 à K-1 ;
 *   - NON_COLORE (-1) signifie « sommet pas encore colorié ».
 *
 * Fichiers de graphes : format DIMACS (.col), 1-indexé :
 *   c commentaire
 *   p edge <n> <m>
 *   e <u> <v>          (une ligne par arête)
 *   c nom <id> <nom>   (optionnel, après la ligne p : nom du sommet id)
 */
#ifndef GRAPHE_H
#define GRAPHE_H

#include <stddef.h>

#define NON_COLORE (-1)
#define TAILLE_NOM 32

typedef struct {
    int n;                      /* nombre de sommets |V|                       */
    long m;                     /* nombre d'arêtes |E|                         */
    unsigned char *adj;         /* matrice n*n : adj[u*n + v] = 1 si arête u-v */
    char (*noms)[TAILLE_NOM];   /* noms des sommets, NULL si aucun nom         */
} Graphe;

/* Test d'adjacence en O(1) : 1 si u et v sont reliés, 0 sinon. */
static inline int adjacent(const Graphe *g, int u, int v)
{
    return g->adj[(size_t)u * (size_t)g->n + (size_t)v];
}

/* ---------- Construction ---------- */
Graphe *graphe_creer(int n);                          /* graphe vide à n sommets   */
void    graphe_liberer(Graphe *g);
int     graphe_ajouter_arete(Graphe *g, int u, int v); /* 1 si ajoutée, 0 sinon     */
void    graphe_nommer(Graphe *g, int v, const char *nom);

/* ---------- Informations ---------- */
int         graphe_degre(const Graphe *g, int v);     /* O(n)                      */
int         graphe_degre_max(const Graphe *g);        /* Δ, en O(n²)               */
const char *graphe_nom(const Graphe *g, int v, char tampon[TAILLE_NOM]);
void        graphe_afficher(const Graphe *g);         /* résumé (+ matrice si n <= 20) */

/* ---------- Fichiers DIMACS ---------- */
Graphe *graphe_charger(const char *chemin);           /* NULL en cas d'erreur      */
int     graphe_sauver(const Graphe *g, const char *chemin);

/* ---------- Générateur reproductible ----------
 * Chaque paire {u,v} devient une arête avec la probabilité « densite ».
 * Même (n, densite, graine) => même graphe sur toutes les machines,
 * car le générateur pseudo-aléatoire est codé ici (pas rand()). */
Graphe *graphe_aleatoire(int n, double densite, unsigned long long graine);

unsigned long long alea_suivant(unsigned long long *etat); /* splitmix64          */
double             alea_reel(unsigned long long *etat);    /* réel dans [0, 1)    */

/* ---------- Colorations ---------- */
int  *coloration_creer(int n);                        /* n cases à NON_COLORE      */
int  *coloration_copier(const int *couleurs, int n);  /* copie indépendante        */
int   coloration_est_valide(const Graphe *g, const int *couleurs); /* 1 si complète et sans conflit */
long  coloration_nb_conflits(const Graphe *g, const int *couleurs); /* arêtes de même couleur */
int   coloration_nb_couleurs(const int *couleurs, int n);           /* couleurs distinctes   */
void  coloration_afficher(const Graphe *g, const int *couleurs);

#endif /* GRAPHE_H */
