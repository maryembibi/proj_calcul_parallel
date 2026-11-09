"""
Dessine les figures des exemples du rapport à partir des fichiers .col.

Usage (depuis le dossier du projet) :
    python3 figures/dessiner_exemples.py
Produit : figures/exemple_simple.png, figures/examens.png, figures/chemin_p4.png
Dépendances : pip install matplotlib networkx
"""
import os

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import networkx as nx
from matplotlib.patches import Patch

DOSSIER = os.path.dirname(os.path.abspath(__file__))
GRAPHES = os.path.join(DOSSIER, "..", "graphes")

# Palette lisible aussi par les daltoniens (une couleur par classe)
PALETTE = ["#4C72B0", "#DD8452", "#55A868", "#C44E52"]


def charger(nom_fichier):
    """Lit un fichier DIMACS et renvoie un graphe networkx (noms de sommets inclus)."""
    g = nx.Graph()
    noms = {}
    with open(os.path.join(GRAPHES, nom_fichier), encoding="utf-8") as f:
        for ligne in f:
            mots = ligne.split()
            if not mots:
                continue
            if mots[0] == "p":
                g.add_nodes_from(range(int(mots[2])))
            elif mots[0] == "e":
                g.add_edge(int(mots[1]) - 1, int(mots[2]) - 1)
            elif mots[0] == "c" and len(mots) >= 4 and mots[1] == "nom":
                noms[int(mots[2]) - 1] = mots[3]
    return g, noms


def dessiner(ax, g, noms, couleurs, positions, titre, taille_noeud=1400, police=13):
    nx.draw_networkx_edges(g, positions, ax=ax, width=2, edge_color="#555555")
    nx.draw_networkx_nodes(g, positions, ax=ax, node_size=taille_noeud,
                           node_color=[PALETTE[couleurs[v]] for v in g.nodes],
                           edgecolors="#222222", linewidths=1.2)
    nx.draw_networkx_labels(g, positions, ax=ax, labels={v: noms.get(v, str(v)) for v in g.nodes},
                            font_size=police, font_color="white", font_weight="bold")
    ax.set_title(titre, fontsize=13)
    ax.axis("off")
    ax.margins(0.15)


def legende(ax, etiquettes):
    ax.legend(handles=[Patch(color=PALETTE[i], label=e) for i, e in enumerate(etiquettes)],
              loc="lower center", bbox_to_anchor=(0.5, -0.12), ncol=len(etiquettes), frameon=False,
              fontsize=11)


def figure_exemple_simple():
    g, noms = charger("exemple_simple.col")
    couleurs = [0, 1, 2, 0, 1, 2]                      # A,D -> 0 ; B,E -> 1 ; C,F -> 2
    positions = {0: (0, 2.0), 1: (-1.8, -1.0), 2: (1.8, -1.0),      # triangle extérieur A, B, C
                 5: (0, 0.85), 3: (-0.75, -0.35), 4: (0.75, -0.35)}  # triangle intérieur F, D, E
    fig, ax = plt.subplots(figsize=(6, 5.4))
    dessiner(ax, g, noms, couleurs, positions,
             "Graphe prisme : n = 6, m = 9, coloriage à 3 couleurs (χ = 3)")
    legende(ax, ["Couleur 0", "Couleur 1", "Couleur 2"])
    fig.tight_layout()
    fig.savefig(os.path.join(DOSSIER, "exemple_simple.png"), dpi=160, bbox_inches="tight")
    plt.close(fig)


def figure_examens():
    g, noms = charger("examens.col")
    couleurs = [0, 1, 2, 1, 0, 0, 2]                   # créneau de chaque examen
    # Disposition sans croisement d'arêtes : triangle Maths-Physique-Informatique en haut,
    # triangle Anglais-Économie-Histoire en bas, Chimie à droite.
    positions = {0: (-2.0, 1.5), 1: (0.0, 2.2), 2: (-1.0, 0.3), 3: (-2.2, -1.5),
                 4: (-0.6, -0.6), 5: (1.8, 0.6), 6: (1.6, -1.5)}
    fig, ax = plt.subplots(figsize=(7.5, 5.8))
    nx.draw_networkx_edges(g, positions, ax=ax, width=2, edge_color="#555555")
    for v in g.nodes:                                  # noms longs : étiquettes en boîte arrondie
        x, y = positions[v]
        ax.text(x, y, noms[v], ha="center", va="center", fontsize=12, color="white",
                fontweight="bold", zorder=3,
                bbox=dict(boxstyle="round,pad=0.55", fc=PALETTE[couleurs[v]], ec="#222222", lw=1.2))
    ax.set_title("Examens : une arête = au moins un étudiant commun", fontsize=13)
    ax.set_xlim(-3.0, 2.6)
    ax.set_ylim(-2.1, 2.7)
    ax.axis("off")
    legende(ax, ["Créneau 1", "Créneau 2", "Créneau 3"])
    fig.tight_layout()
    fig.savefig(os.path.join(DOSSIER, "examens.png"), dpi=160, bbox_inches="tight")
    plt.close(fig)


def figure_chemin():
    g, noms = charger("chemin_p4.col")
    positions = {v: (v, 0) for v in g.nodes}
    fig, axes = plt.subplots(2, 1, figsize=(6.5, 3.6))
    dessiner(axes[0], g, noms, [0, 1, 0, 1], positions,
             "Glouton, ordre a, b, c, d : 2 couleurs (optimal)", taille_noeud=900)
    dessiner(axes[1], g, noms, [0, 1, 2, 0], positions,
             "Glouton, ordre a, d, b, c : 3 couleurs", taille_noeud=900)
    fig.tight_layout()
    fig.savefig(os.path.join(DOSSIER, "chemin_p4.png"), dpi=160, bbox_inches="tight")
    plt.close(fig)


if __name__ == "__main__":
    figure_exemple_simple()
    figure_examens()
    figure_chemin()
    print("Figures écrites dans", DOSSIER)
