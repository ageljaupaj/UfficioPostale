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

    float P_SERV = (float)random_between(3, 6) / 10.0;
    printf("[Utente %d] Probabilità di andare in posta: %.1f\n", id, P_SERV);

    int msgid = msgget(MSGKEY, 0666);
    int shmid = shmget(SHMKEY, sizeof(int)*3 + sizeof(Sportello)*MAX_SPORTELLI, 0666);
    int *shared_data = shmat(shmid, NULL, 0);
    int *attesa = &shared_data[0];
    int *giorno_corrente = &shared_data[1];
    int *giornata_in_corso = &shared_data[2];
    int semid = semget(SEMKEY, 1, 0666);

    int ultimo_giorno = 0;

    while (1) {
        while (!*giornata_in_corso);  // aspetta linizio di una giornata

        // svuota eventuali conferme vecchie
        MessaggioTicket conferma;
        while (msgrcv(msgid, &conferma, sizeof(conferma) - sizeof(long), getpid(), IPC_NOWAIT) != -1);

        // se è iniziato un nuovo giorno
        if (*giorno_corrente != ultimo_giorno) {
            ultimo_giorno = *giorno_corrente;

            float r = (float)random_between(0, 10) / 10.0;
            if (r <= P_SERV) {
                int n_servizi = random_between(1, MAX_REQUESTS);
                printf("[Utente %d] Giorno %d: vado in posta e richiedo %d servizi uno dopo l'altro.\n",
                       id, ultimo_giorno, n_servizi);

                struct sembuf p = {0, -1, 0};
                semop(semid, &p, 1);
                (*attesa)++;
                struct sembuf v = {0, +1, 0};
                semop(semid, &v, 1);

                for (int req = 0; req < n_servizi; req++) {
                    if (!*giornata_in_corso) {
                        printf("[Utente %d] Giornata finita prima di iniziare richiesta %d.\n", id, req+1);
                        break;
                    }

                    TipoServizio serv = rand() % NUM_SERVIZI;
                    printf("[Utente %d] Giorno %d: voglio %s (%d/%d).\n",
                           id, ultimo_giorno, NOMI_SERVIZI[serv], req+1, n_servizi);

                    MessaggioTicket richiesta = {
                        .mtype = 100 + serv,
                        .pid_utente = getpid(),
                        .tipo_servizio = serv,
                        .tempo_arrivo = *giorno_corrente
                    };
                    msgsnd(msgid, &richiesta, sizeof(richiesta) - sizeof(long), 0);

                    // attesa bloccante per conferma
                    msgrcv(msgid, &conferma, sizeof(conferma) - sizeof(long), getpid(), 0);

                    if (!*giornata_in_corso) {
                        printf("[Utente %d] Giornata finita mentre aspettavo %s, rinuncio.\n", id, NOMI_SERVIZI[serv]);
                        MessaggioTicket fail_msg = {
                            .mtype = 98,
                            .pid_utente = getpid(),
                            .tipo_servizio = serv
                        };
                        msgsnd(msgid, &fail_msg, sizeof(fail_msg) - sizeof(long), 0);

                        struct sembuf p2 = {0, -1, 0};
                        semop(semid, &p2, 1);
                        (*attesa)--;
                        struct sembuf v2 = {0, +1, 0};
                        semop(semid, &v2, 1);
                        break;
                    }

                    printf("[Utente %d] Servizio %s completato!\n", id, NOMI_SERVIZI[serv]);
                }

                // decrementa comunque attesa se ha finito i servizi
                struct sembuf p2 = {0, -1, 0};
                semop(semid, &p2, 1);
                (*attesa)--;
                struct sembuf v2 = {0, +1, 0};
                semop(semid, &v2, 1);

            } else {
                printf("[Utente %d] Giorno %d: resto a casa.\n", id, ultimo_giorno);
            }
        }
    }

    printf("[Utente %d] Simulazione finita, mi scollego.\n", id);
    shmdt(shared_data);
    return 0;
}




