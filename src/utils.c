#include "../include/utils.h"
#include <unistd.h>
#include <stdio.h>
#include "common.h"


int random_between(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// funzione per simulare il passare del tempo in minuti della simulazione
void sleep_simulato(int minuti) {
    const int N_NANO_SECS = 100000000;
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = minuti * N_NANO_SECS;
    nanosleep(&ts, NULL);
}

// salva le statistiche nel file CSV
void salva_statistiche_csv(Statistiche *stat, const char *filename, int giorno) {
    FILE *f;
    int nuovo_file = access(filename, F_OK) != 0;

    f = fopen(filename, "a");
    if (!f) {
        perror("Errore apertura file CSV");
        return;
    }

    if (nuovo_file) {
        fprintf(f, "Giorno,UtentiServiti,ServiziTotali,Pacchi,Lettere,BancoPosta,Bollettini,Finanziari,Orologi\n");
    }

    int tot_servizi = 0;
    for (int i = 0; i < NUM_SERVIZI; i++) {
        tot_servizi += stat->servizi_erogati[i];
    }

    fprintf(f, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n", giorno, stat->utenti_serviti, tot_servizi,
        stat->servizi_erogati[SERVIZIO_PACCHI],
        stat->servizi_erogati[SERVIZIO_LETTERE],
        stat->servizi_erogati[SERVIZIO_BANCOPOSTA],
        stat->servizi_erogati[SERVIZIO_BOLLETTINI],
        stat->servizi_erogati[SERVIZIO_PRODOTTI_FINANZIARI],
        stat->servizi_erogati[SERVIZIO_OROLOGI]);

    fclose(f);
}

void stampa_statistiche_tabella_tutti_giorni(Statistiche stats[], int giorni) {
    printf("\n+------------------------------------------------------------------------------------------------------------------------------+\n");
    printf("| %-6s | %-13s | %-17s | %-8s | %-8s | %-12s | %-12s | %-14s | %-10s |\n",
           "Giorno", "Ut.Serviti", "Servizi Totali", "Pacchi", "Lettere",
           "BancoPosta", "Bollettini", "Finanziari", "Orologi");
    printf("+------------------------------------------------------------------------------------------------------------------------------+\n");

    int total_utenti = 0, total_servizi_totali = 0;
    int total_erogati[NUM_SERVIZI] = {0};

    for (int g = 0; g < giorni; g++) {
        int total_servizi = 0;
        for (int i = 0; i < NUM_SERVIZI; i++) {
            total_servizi += stats[g].servizi_erogati[i];
            total_erogati[i] += stats[g].servizi_erogati[i];
        }
        total_utenti += stats[g].utenti_serviti;
        total_servizi_totali += total_servizi;

        printf("| %-6d | %-13d | %-17d | %-8d | %-8d | %-12d | %-12d | %-14d | %-10d |\n",
               g+1,
               stats[g].utenti_serviti,
               total_servizi,
               stats[g].servizi_erogati[SERVIZIO_PACCHI],
               stats[g].servizi_erogati[SERVIZIO_LETTERE],
               stats[g].servizi_erogati[SERVIZIO_BANCOPOSTA],
               stats[g].servizi_erogati[SERVIZIO_BOLLETTINI],
               stats[g].servizi_erogati[SERVIZIO_PRODOTTI_FINANZIARI],
               stats[g].servizi_erogati[SERVIZIO_OROLOGI]);
    }

    printf("+------------------------------------------------------------------------------------------------------------------------------+\n");
    printf("| %-6s | %-13d | %-17d | %-8d | %-8d | %-12d | %-12d | %-14d | %-10d |\n",
           "TOT",
           total_utenti,
           total_servizi_totali,
           total_erogati[SERVIZIO_PACCHI],
           total_erogati[SERVIZIO_LETTERE],
           total_erogati[SERVIZIO_BANCOPOSTA],
           total_erogati[SERVIZIO_BOLLETTINI],
           total_erogati[SERVIZIO_PRODOTTI_FINANZIARI],
           total_erogati[SERVIZIO_OROLOGI]);
    printf("+------------------------------------------------------------------------------------------------------------------------------+\n");

    // scrive anche la riga finale TOT nel CSV
    FILE *f = fopen("stats/statistiche.csv", "a");
    if (f) {
        fprintf(f, "TOT,%d,%d,%d,%d,%d,%d,%d,%d\n",
            total_utenti,
            total_servizi_totali,
            total_erogati[SERVIZIO_PACCHI],
            total_erogati[SERVIZIO_LETTERE],
            total_erogati[SERVIZIO_BANCOPOSTA],
            total_erogati[SERVIZIO_BOLLETTINI],
            total_erogati[SERVIZIO_PRODOTTI_FINANZIARI],
            total_erogati[SERVIZIO_OROLOGI]);
        fclose(f);
    }
}
