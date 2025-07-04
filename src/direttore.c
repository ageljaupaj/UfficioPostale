#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/wait.h>

#include "../include/common.h"
#include "../include/utils.h"

#define MSGKEY 1234
#define SHMKEY 5678
#define SEMKEY 9102

int NOF_USERS = 50;
int NOF_OPERATORS = 6;
int SIM_DAYS = 5;

union semun { int val; };

pid_t *utenti;
pid_t *operatori;
pid_t erogatore;

void avvia_utenti() {
    for (int i = 0; i < NOF_USERS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            char id[10];
            snprintf(id, sizeof(id), "%d", i);
            execl("./utente", "utente", id, NULL);
            perror("execl utente");
            exit(1);
        }
        utenti[i] = pid;
    }
}

void avvia_operatori() {
    for (int i = 0; i < NOF_OPERATORS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            char id[10];
            snprintf(id, sizeof(id), "%d", i);
            execl("./operatore", "operatore", id, NULL);
            perror("execl operatore");
            exit(1);
        }
        operatori[i] = pid;
    }
}

void avvia_erogatore_ticket() {
    pid_t pid = fork();
    if (pid == 0) {
        execl("./ticket", "ticket", NULL);
        perror("execl ticket");
        exit(1);
    }
    erogatore = pid;
}

void leggi_parametri_ambiente() { // leggere variabili di ambiente
    char *env;
    if ((env = getenv("NOF_USERS"))) NOF_USERS = atoi(env);
    if ((env = getenv("NOF_OPERATORS"))) NOF_OPERATORS = atoi(env);
    if ((env = getenv("SIM_DAYS"))) SIM_DAYS = atoi(env);
}

int main() {
    leggi_parametri_ambiente();

    utenti = malloc(sizeof(pid_t) * NOF_USERS);
    operatori = malloc(sizeof(pid_t) * NOF_OPERATORS);

    srand(getpid()); // ogni processo utente o operatore ha una sequenza di numeri casuali diversa
    printf("[Direttore] Inizio simulazione ufficio postale!\n");
    printf("[Direttore] Parametri: utenti=%d, operatori=%d, giorni=%d\n",
           NOF_USERS, NOF_OPERATORS, SIM_DAYS);

    int msgid = msgget(MSGKEY, IPC_CREAT | 0666);//coda
    if (msgid == -1) { perror("msgget"); exit(1); }

    int shmid = shmget(SHMKEY, sizeof(int)*3 + sizeof(Sportello)*MAX_SPORTELLI, IPC_CREAT | 0666);//memria cndivisa
    if (shmid == -1) { perror("shmget"); exit(1); }

    int semid = semget(SEMKEY, 1, IPC_CREAT | 0666);//semaforo
    if (semid == -1) { perror("semget"); exit(1); }
    union semun arg = {.val = 1};
    semctl(semid, 0, SETVAL, arg);

    void *shared_raw = shmat(shmid, NULL, 0);
    if (shared_raw == (void *) -1) { perror("shmat"); exit(1); }

    int *utenti_in_attesa = (int *)shared_raw;
    int *giorno_corrente = (int *)((char*)shared_raw + sizeof(int));
    int *giornata_in_corso = (int *)((char*)shared_raw + sizeof(int)*2);
    Sportello *sportelli = (Sportello *)((char*)shared_raw + sizeof(int)*3);

    *utenti_in_attesa = 0;
    *giorno_corrente = 1;
    *giornata_in_corso = 0;

    avvia_erogatore_ticket();
    avvia_operatori();
    avvia_utenti();

    Statistiche *stats_per_giorno = malloc(sizeof(Statistiche) * SIM_DAYS);
    for (int i = 0; i < SIM_DAYS; i++)
        stats_per_giorno[i] = (Statistiche){0};

    int totale_attesa = 0;
    int totale_servizi = 0;
    int cause_terminate = 0;

    for (int giorno = 1; giorno <= SIM_DAYS; giorno++) {
        *giorno_corrente = giorno;

        printf("[Direttore] Sportelli assegnati oggi: ");
        for (int s = 0; s < NUM_SERVIZI && s < MAX_SPORTELLI; s++) {// crea i sportelli
            sportelli[s].servizio = s;
            sportelli[s].occupato = 0;
            printf("%s ", NOMI_SERVIZI[s]);
        }
        for (int i = NUM_SERVIZI; i < MAX_SPORTELLI; i++) {//assegnazione casuale sportelli
            sportelli[i].servizio = rand() % NUM_SERVIZI;
            sportelli[i].occupato = 0;
            printf("%s ", NOMI_SERVIZI[sportelli[i].servizio]);
        }
        printf("\n");

        *giornata_in_corso = 1;
        for (volatile int tick = 0; tick < 5000000; tick++);

        int attesa_giornaliera = 0;
        int servizi_giorno = 0;

        while (1) {
            MessaggioTicket report; // riceve i messaggi che gli operatori inviano come report di servizio completato
            if (msgrcv(msgid, &report, sizeof(report) - sizeof(long), 99, IPC_NOWAIT) == -1) break;
            stats_per_giorno[giorno-1].utenti_serviti++;
            stats_per_giorno[giorno-1].servizi_erogati[report.tipo_servizio]++;
            if (report.tempo_arrivo <= *giorno_corrente)
                attesa_giornaliera += (*giorno_corrente - report.tempo_arrivo);
            servizi_giorno++;
        }

        while (1) {
            MessaggioTicket fail;//conteggio servizi non erogati
            if (msgrcv(msgid, &fail, sizeof(fail) - sizeof(long), 98, IPC_NOWAIT) == -1) break;
            stats_per_giorno[giorno-1].servizi_non_erogati[fail.tipo_servizio]++;
        }

        MessaggioTicket dump;//pulizia coda 
        while (msgrcv(msgid, &dump, sizeof(dump) - sizeof(long), 0, IPC_NOWAIT) != -1);

        *giornata_in_corso = 0;
        for (volatile int tick = 0; tick < 1000000; tick++);//fa un piccolo delay per permettere loro di rileggere il flag prima che cambi giorno

        if (servizi_giorno > 0) {
            printf("[Direttore] Giorno %d: tempo medio di attesa %.2f\n",
                   giorno, (float)attesa_giornaliera / servizi_giorno);
        }

        totale_attesa += attesa_giornaliera;
        totale_servizi += servizi_giorno;

        salva_statistiche_csv(&stats_per_giorno[giorno-1], "stats/statistiche.csv", giorno);

        struct sembuf p = {0, -1, 0};//fatto prima di controllare utenti_in_attesa per evitare una race condition
        semop(semid, &p, 1);

        printf("[Direttore] Fine giornata %d.\n", giorno);
        if (*utenti_in_attesa > EXPLODE_THRESHOLD) {
            printf("[Direttore] TERMINAZIONE EXPLODE: utenti in attesa = %d > threshold = %d\n",
                   *utenti_in_attesa, EXPLODE_THRESHOLD);
            cause_terminate = 1;
            semop(semid, &(struct sembuf){0, +1, 0}, 1);
            break;
        }
        semop(semid, &(struct sembuf){0, +1, 0}, 1);
    }

    int totale_utenti_serviti = 0;
    int totale_servizi_non_erogati = 0;
    for (int i = 0; i < SIM_DAYS; i++) {
        totale_utenti_serviti += stats_per_giorno[i].utenti_serviti;
        for (int s = 0; s < NUM_SERVIZI; s++) {
            totale_servizi_non_erogati += stats_per_giorno[i].servizi_non_erogati[s];
        }
    }

    printf("\n[Direttore] Utenti serviti totali nella simulazione: %d\n", totale_utenti_serviti);
    printf("[Direttore] Utenti serviti in media al giorno: %.2f\n", 
           (float)totale_utenti_serviti / SIM_DAYS);
    printf("\n[Direttore] Servizi non erogati totali nella simulazione: %d\n", totale_servizi_non_erogati);
    printf("[Direttore] Servizi non erogati in media al giorno: %.2f\n",
           (float)totale_servizi_non_erogati / SIM_DAYS);

    printf("\n[Direttore] Statistiche per tipologia di servizio:\n");
    for (int s = 0; s < NUM_SERVIZI; s++) {
        int tot_erogati = 0, tot_non_erogati = 0;
        for (int g = 0; g < SIM_DAYS; g++) {
            tot_erogati += stats_per_giorno[g].servizi_erogati[s];
            tot_non_erogati += stats_per_giorno[g].servizi_non_erogati[s];
        }
        printf("  - %s: erogati totali = %d, non erogati totali = %d\n",
               NOMI_SERVIZI[s], tot_erogati, tot_non_erogati);
    }

    stampa_statistiche_tabella_tutti_giorni(stats_per_giorno, SIM_DAYS);

    if (totale_servizi > 0) {
        printf("\n[Direttore] Tempo medio di attesa complessivo: %.2f\n",
               (float)totale_attesa / totale_servizi);
    }

    if (cause_terminate == 1) {
        printf("[Direttore] Simulazione terminata per EXPLODE.\n");
    } else {
        printf("[Direttore] Simulazione terminata per durata normale.\n");
    }

    for (int i = 0; i < NOF_USERS; i++) kill(utenti[i], SIGTERM);
    for (int i = 0; i < NOF_OPERATORS; i++) kill(operatori[i], SIGTERM);
    kill(erogatore, SIGTERM);

    for (int i = 0; i < NOF_USERS; i++) waitpid(utenti[i], NULL, 0);
    for (int i = 0; i < NOF_OPERATORS; i++) waitpid(operatori[i], NULL, 0);
    waitpid(erogatore, NULL, 0);

    shmdt(shared_raw);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    free(utenti);
    free(operatori);
    free(stats_per_giorno);

    printf("\n[Direttore] Simulazione conclusa. Arrivederci!\n");
    return 0;
}

