#ifndef UTILS_H
#define UTILS_H

#include "common.h"

int random_between(int min, int max);
void sleep_simulato(int minuti);

#endif

void salva_statistiche_csv(Statistiche *stat, const char *filename, int giorno);


void stampa_statistiche_tabella_tutti_giorni(Statistiche stats[], int giorni); 