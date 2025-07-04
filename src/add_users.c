/*
 * add_users.c
 * Questo è un programma a parte che viene eseguito da riga di comando.
 * Serve per aggiungere N nuovi utenti alla simulazione, come richiesto
 */

#include "../include/common.h"

#define N_NEW_USERS 3 

int main() {
    printf("[add_users] Aggiunta di %d nuovi utenti alla simulazione in corso...\n", N_NEW_USERS);

    for (int i = 0; i < N_NEW_USERS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            char id[10];
            snprintf(id, 10, "add%d", i); // id simbolico, non usato internamente
            execl("./utente", "utente", id, NULL);
            perror("execl nuovo utente");
            exit(1);
        } else if (pid < 0) {
            perror("fork nuovo utente");
        }
    }

    printf("[add_users] Operazione completata.\n");
    return 0;
}
