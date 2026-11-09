# Projet « Coloriage de graphes » - compilation
# Mêmes options pour toutes les versions (séquentielles et parallèles) :
# c'est indispensable pour comparer les temps honnêtement.

CC      = gcc
CFLAGS  = -O2 -Wall -Wextra -std=c11 -fopenmp
COMMUN  = graphe.o mesures.o

PROGRAMMES = test_commun generer backtrack_seq
# B2 ajoutera : glouton_seq glouton_par
# B3 ajoutera : backtrack_par

all: $(PROGRAMMES)

test_commun: test_commun.o $(COMMUN)
	$(CC) $(CFLAGS) -o $@ $^

generer: generer.o $(COMMUN)
	$(CC) $(CFLAGS) -o $@ $^

backtrack_seq: backtrack_seq.o $(COMMUN)
	$(CC) $(CFLAGS) -o $@ $^

# Modèle pour les programmes de B2 et B3 (exemple) :
# glouton_seq: glouton_seq.o $(COMMUN)
# 	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c graphe.h mesures.h
	$(CC) $(CFLAGS) -c $<

test: test_commun
	./test_commun

clean:
	rm -f *.o $(PROGRAMMES) *.exe

.PHONY: all test clean
