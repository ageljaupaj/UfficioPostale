#include "../include/common.h"
#include "../include/utils.h"
#include <sys/shm.h>
#include <signal.h>

#define MSGKEY 1234

int msgid;

void termina(int sig) {
    (void)sig;
    printf("[Ticket] Terminazione ricevuta.\n");
    exit(0);
}

int main() {
    signal(SIGTERM, termina); //signal di terminazione

    msgid = msgget(MSGKEY, IPC_CREAT | 0666);// coda di idendificatori assiocaiato con una chiave
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    printf("[Ticket] Pronto a ricevere richieste dagli utenti.\n");

    while (1) {
        MessaggioTicket req;

        if (msgrcv(msgid, &req, sizeof(req) - sizeof(long), 1, 0) == -1) {
            if (errno == EINTR) continue;// se è stato solo interrotto da un segnale EINTR, allora facciamo continue per riprovare
            perror("msgrcv");
            break;
        }

        //int tempo_medio = TEMPI_EROGAZIONE[req.tipo_servizio];
        //int tempo_random = random_between(tempo_medio / 2, tempo_medio);
        //sleep_simulato(tempo_random);

        printf("[Ticket] Ricevuta richiesta da utente %d per servizio %s\n",
               req.pid_utente, NOMI_SERVIZI[req.tipo_servizio]);

        // manda direttamente al tipo di servizio giusto
        req.mtype = 100 + req.tipo_servizio;
        if (msgsnd(msgid, &req, sizeof(req) - sizeof(long), 0) == -1) {
            perror("msgsnd verso operatore");
        }
    }

    return 0;
}

