/*
 * mesures.c - Implémentation de mesures.h
 */
#include "mesures.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef _OPENMP
#include <omp.h>
#endif
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/* ====================================================================== */
/*  Console                                                               */
/* ====================================================================== */

void console_utf8(void)
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);   /* les fichiers sources sont en UTF-8 */
#endif
}

/* ====================================================================== */
/*  Chronomètre                                                           */
/* ====================================================================== */

double chrono(void)
{
#ifdef _OPENMP
    return omp_get_wtime();
#else
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
#endif
}

int numero_thread(void)
{
#ifdef _OPENMP
    return omp_get_thread_num();
#else
    return 0;
#endif
}

int nb_threads_max(void)
{
#ifdef _OPENMP
    return omp_get_max_threads();
#else
    return 1;
#endif
}

/* ====================================================================== */
/*  Mesures                                                               */
/* ====================================================================== */

int mesure_ecrire(const char *fichier_csv, const char *algo, const char *version,
                  const char *graphe, int n, long m, int threads, int essai,
                  double temps_s, int nb_couleurs)
{
    FILE *f = fopen(fichier_csv, "a");
    if (f == NULL) {
        perror(fichier_csv);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    if (ftell(f) == 0)
        fprintf(f, "algo,version,graphe,n,m,threads,essai,temps_s,nb_couleurs\n");
    fprintf(f, "%s,%s,%s,%d,%ld,%d,%d,%.6f,%d\n",
            algo, version, graphe, n, m, threads, essai, temps_s, nb_couleurs);
    fclose(f);
    return 1;
}

/* ====================================================================== */
/*  Trace pour les diagrammes de Gantt                                    */
/* ====================================================================== */

typedef struct {
    int thread;
    double debut, fin;          /* secondes depuis trace_demarrer */
    char etiquette[32];
} EvenementTrace;

static EvenementTrace *evenements = NULL;
static int capacite_trace = 0;
static int nb_evenements = 0;   /* peut dépasser la capacité : événements perdus */
static double origine = 0.0;

void trace_demarrer(int capacite)
{
    free(evenements);
    evenements = calloc((size_t)capacite, sizeof *evenements);
    capacite_trace = (evenements != NULL) ? capacite : 0;
    nb_evenements = 0;
    origine = chrono();
}

void trace_ajouter(double debut, double fin, const char *etiquette)
{
    int i;
    /* Réserve une case du tableau : l'incrément atomique évite que deux
       threads écrivent dans la même case (condition de concurrence). */
    #pragma omp atomic capture
    i = nb_evenements++;

    if (i >= capacite_trace) return;          /* trace pleine : événement ignoré */

    evenements[i].thread = numero_thread();
    evenements[i].debut = debut - origine;
    evenements[i].fin = fin - origine;
    strncpy(evenements[i].etiquette, etiquette ? etiquette : "", sizeof evenements[i].etiquette - 1);
    for (char *c = evenements[i].etiquette; *c; c++)
        if (*c == ',') *c = ' ';              /* pas de virgule dans un champ CSV */
}

int trace_nb_evenements(void)
{
    return nb_evenements < capacite_trace ? nb_evenements : capacite_trace;
}

int trace_ecrire_csv(const char *fichier_csv)
{
    FILE *f = fopen(fichier_csv, "w");
    if (f == NULL) {
        perror(fichier_csv);
        return 0;
    }
    if (nb_evenements > capacite_trace)
        fprintf(stderr, "Attention : %d événements perdus (augmenter la capacité de la trace)\n",
                nb_evenements - capacite_trace);

    fprintf(f, "thread,debut_ms,fin_ms,duree_ms,etiquette\n");
    int nb = trace_nb_evenements();
    for (int i = 0; i < nb; i++)
        fprintf(f, "%d,%.4f,%.4f,%.4f,%s\n", evenements[i].thread,
                evenements[i].debut * 1e3, evenements[i].fin * 1e3,
                (evenements[i].fin - evenements[i].debut) * 1e3, evenements[i].etiquette);
    fclose(f);
    return 1;
}

void trace_arreter(void)
{
    free(evenements);
    evenements = NULL;
    capacite_trace = 0;
    nb_evenements = 0;
}
