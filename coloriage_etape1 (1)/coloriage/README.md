# Projet « Coloriage de graphes » – code commun (étape 1)

Base commune aux trois binômes. B2 (glouton) et B3 (backtracking) ajoutent leurs
programmes dans ce même dossier et utilisent uniquement ces fonctions pour lire
les graphes, vérifier les colorations et mesurer les temps.

## Contenu

| Fichier | Rôle |
| --- | --- |
| `graphe.h` / `graphe.c` | Matrice d'adjacence, fichiers DIMACS, générateur reproductible, vérification des colorations |
| `mesures.h` / `mesures.c` | Chronomètre, CSV des mesures, trace des tâches pour les diagrammes de Gantt |
| `generer.c` | Outil en ligne de commande : génère un graphe aléatoire et l'enregistre en `.col` |
| `test_commun.c` | 27 tests à lancer sur chaque machine |
| `graphes/exemple_simple.col` | Exemple simple du rapport (§3) : graphe prisme, χ = 3 |
| `graphes/examens.col` | Exemple réel (§4) : 7 examens, 3 créneaux |
| `graphes/chemin_p4.col` | Chemin a-b-c-d : le glouton dépend de l'ordre |

## Compiler et tester

**Avec Code::Blocks (Windows)** : ouvrir `coloriage.cbp`, choisir la cible
`test_commun` dans la liste « Build target », puis F9 (Build and run).
Les options `-O2 -std=c11 -fopenmp` sont déjà réglées dans le projet.
La cible `generer` reçoit ses arguments par Project > Set programs' arguments.

**En ligne de commande (Linux, WSL, Mac)** :

```
make          # compile test_commun et generer
make test     # lance les tests depuis ce dossier
```

Résultat attendu : `27 tests, 0 échec(s)`. Le test « même nombre d'arêtes que sur
les autres machines » vérifie que `graphe_aleatoire(200, 0.3, 42)` donne bien
5948 arêtes chez tout le monde.

- **Linux / WSL** : `sudo apt install build-essential` suffit.
- **Windows** : Code::Blocks dans sa version livrée avec MinGW (`…mingw-setup.exe`), ou WSL.
- **Mac** : le `gcc` d'Apple est clang, sans OpenMP. Installer `brew install gcc` puis
  lancer `make CC=gcc-14` (adapter le numéro de version).

## Conventions

- Sommets numérotés de 0 à n−1, couleurs de 0 à K−1, `NON_COLORE` (−1) = pas encore colorié.
- `adjacent(g, u, v)` teste une arête en O(1).
- Toujours vérifier un résultat avec `coloration_est_valide(g, couleurs)` (hors chronométrage).
- Graphes de mesure générés en mémoire avec `graphe_aleatoire(n, densite, graine)` :
  pas besoin de fichiers volumineux.
- Une ligne par exécution dans le CSV avec `mesure_ecrire(...)`.

## Ajouter un programme (B2, B3)

Créer par exemple `glouton_seq.c` avec `#include "graphe.h"` et `#include "mesures.h"`.

Dans Code::Blocks : Project > Properties > Build targets > Add, nommer la cible
`glouton_seq`, puis cocher pour elle `glouton_seq.c`, `graphe.c` et `mesures.c`.

Avec make, décommenter le modèle dans le `Makefile` :

```
glouton_seq: glouton_seq.o $(COMMUN)
	$(CC) $(CFLAGS) -o $@ $^
```
