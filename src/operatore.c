#include "../include/common.h"
#include "../include/utils.h"
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>

#define MSGKEY 1234
#define SHMKEY 5678
#define SEMKEY 9102

int main(int argc, char *argv[]) {
    if (argc != 2) exit(1);
    int id = atoi(argv[1]);
    srand(getpid());

    TipoServizio mio_servizio = id % NUM_SERVIZI;
    printf("[Operatore %d] Sono specializzato in %s\n", id, NOMI_SERVIZI[mio_servizio]);

    int msgid = msgget(MSGKEY, 0666);
    int shmid = shmget(SHMKEY, sizeof(int)*3 + sizeof(Sportello)*MAX_SPORTELLI, 0666);
    void *shared_raw = shmat(shmid, NULL, 0);

    int *giorno_corrente = (int *)((char*)shared_raw + sizeof(int));
    int *giornata_in_corso = (int *)((char*)shared_raw + sizeof(int)*2);
    Sportello *sportelli = (Sportello *)((char*)shared_raw + sizeof(int)*3);

    int ultimo_giorno = 0;
    int pause_effettuate = 0;
    int mio_sportello = -1;

    while (1) {
        // aspetta che inizi una giornata
        while (!*giornata_in_corso);

        // se è iniziato un nuovo giorno, resetta sportello
        if (*giorno_corrente != ultimo_giorno) {
            ultimo_giorno = *giorno_corrente;
            mio_sportello = -1;

            for (int i = 0; i < MAX_SPORTELLI; i++) {
                if (sportelli[i].servizio == mio_servizio && sportelli[i].occupato == 0) {
                    sportelli[i].occupato = 1;
                    mio_sportello = i;
                    printf("[Operatore %d] Ho occupato sportello %d per %s\n",
                           id, i, NOMI_SERVIZI[mio_servizio]);
                    break;
                }
            }
        }

        // se non ha sportello, attende la prossima giornata
        if (mio_sportello == -1) continue;

        // riceve un ticket
        MessaggioTicket job;
        msgrcv(msgid, &job, sizeof(job) - sizeof(long), 100 + mio_servizio, 0);

        // se giornata finita proprio ora, completa comunque il servizio
        if (!*giornata_in_corso) {
            printf("[Operatore %d] Fine giornata rilevata, completo comunque il servizio per utente %d.\n", 
                    id, job.pid_utente);
        }

        int attesa = *giorno_corrente - job.tempo_arrivo; // calcolare il tempo di attesa del servizio
        printf("[Operatore %d] Sto servendo %s per utente %d dopo attesa %d giorni allo sportello %d\n",
               id, NOMI_SERVIZI[mio_servizio], job.pid_utente, attesa, mio_sportello);

        for (volatile int spin = 0; spin < 500000; spin++); // simulazione tempo lavoro

        // invia conferma all'utente
        MessaggioTicket conferma = {
            .mtype = job.pid_utente,
            .pid_utente = job.pid_utente,
            .tipo_servizio = mio_servizio,
            .tempo_arrivo = job.tempo_arrivo
        };
        msgsnd(msgid, &conferma, sizeof(conferma) - sizeof(long), 0);

        // invia report al direttore
        MessaggioTicket report = {
            .mtype = 99,
            .tipo_servizio = mio_servizio,
            .tempo_arrivo = attesa
        };
        msgsnd(msgid, &report, sizeof(report) - sizeof(long), 0);

        // possibilità di pausa
        if (pause_effettuate < MAX_PAUSE && random_between(1,100) <= 20) {
            pause_effettuate++;
            sportelli[mio_sportello].occupato = 0;
            printf("[Operatore %d] Pausa numero %d: libero sportello %d\n",
                   id, pause_effettuate, mio_sportello);
            mio_sportello = -1;
        }

        // se la giornata è finita e non ha più ticket, libera sportello e aspetta
        if (!*giornata_in_corso && mio_sportello != -1) {
            printf("[Operatore %d] Fine giornata, libero sportello e aspetto domani.\n", id);
            sportelli[mio_sportello].occupato = 0;
            mio_sportello = -1;
            while (!*giornata_in_corso);
        }
    }

    shmdt(shared_raw); // detach
    return 0;
}