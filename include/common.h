#ifndef COMMON_H
#define COMMON_H



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <errno.h>
#include <signal.h>


// costanti per i servizi
typedef enum {
    SERVIZIO_PACCHI,
    SERVIZIO_LETTERE,
    SERVIZIO_BANCOPOSTA,
    SERVIZIO_BOLLETTINI,
    SERVIZIO_PRODOTTI_FINANZIARI,
    SERVIZIO_OROLOGI,
    NUM_SERVIZI
} TipoServizio;

// nome dei servizi per stampare
static const char *NOMI_SERVIZI[] __attribute__((unused)) = {
    "Pacchi",
    "Lettere",
    "BancoPosta",
    "Bollettini",
    "ProdottiFinanziari",
    "Orologi"
};


// struttura per un ticket (richiesta utente)
typedef struct {
    long mtype;               // per la message queue
    pid_t pid_utente;
    TipoServizio tipo_servizio;
    int tempo_arrivo;
} MessaggioTicket;

// struttura per statistiche 
typedef struct {
    int utenti_serviti;
    int servizi_erogati[NUM_SERVIZI];
    int servizi_non_erogati[NUM_SERVIZI];
} Statistiche;

static const int TEMPI_EROGAZIONE[] = {
    10, // Pacchi
    8,  // Lettere
    6,  // BancoPosta
    8,  // Bollettini
    20, // ProdottiFinanziari
    20  // Orologi
};

// altri parametri configurabili

#define MAX_SPORTELLI 5
#define MAX_REQUESTS 3
#define MAX_PAUSE 3

// messaggi di logging base
#define DEBUG 1
#define log_debug(msg, ...) if (DEBUG) { printf("[DEBUG] " msg "\n", ##__VA_ARGS__); } //attivare/disattivare messaggi di debug

#define EXPLODE_THRESHOLD 40


typedef struct {
    TipoServizio servizio;
    int occupato; 
} Sportello;




#endif