/*
 * mesures.h - Chronométrage, fichier CSV des mesures et traces pour les diagrammes de Gantt
 *
 * Format du CSV des mesures (une ligne par exécution, en-tête écrit automatiquement) :
 *   algo,version,graphe,n,m,threads,essai,temps_s,nb_couleurs
 *   ex. : glouton,par,G3,2000,999412,4,2,0.012345,187
 *
 * Format du CSV de trace (une ligne par tâche exécutée, temps relatifs à trace_demarrer) :
 *   thread,debut_ms,fin_ms,duree_ms,etiquette
 */
#ifndef MESURES_H
#define MESURES_H

/* ---------- Console ---------- */
void console_utf8(void);      /* accents lisibles dans la console Windows (sans effet ailleurs) */

/* ---------- Chronomètre ---------- */
double chrono(void);          /* temps en secondes (omp_get_wtime)            */
int    numero_thread(void);   /* omp_get_thread_num(), 0 hors région parallèle */
int    nb_threads_max(void);  /* omp_get_max_threads()                          */

/* ---------- Mesures ---------- */
/* Ajoute une ligne au CSV (crée le fichier avec son en-tête s'il est vide).
 * version : "seq" ou "par". threads : 1 pour la version séquentielle. */
int mesure_ecrire(const char *fichier_csv, const char *algo, const char *version,
                  const char *graphe, int n, long m, int threads, int essai,
                  double temps_s, int nb_couleurs);

/* ---------- Trace pour les diagrammes de Gantt ----------
 * Usage :
 *   trace_demarrer(10000);                 // avant la région parallèle
 *   ... dans chaque tâche :
 *       double t = chrono();
 *       ... travail ...
 *       trace_ajouter(t, chrono(), "c=2 v=5");
 *   trace_ecrire_csv("trace.csv");
 *   trace_arreter();
 * trace_ajouter peut être appelée par plusieurs threads en même temps. */
void trace_demarrer(int capacite);
void trace_ajouter(double debut, double fin, const char *etiquette);
int  trace_nb_evenements(void);
int  trace_ecrire_csv(const char *fichier_csv);
void trace_arreter(void);

#endif /* MESURES_H */
