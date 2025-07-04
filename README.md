# Ufficio Postale - Progetto di Sistemi Operativi

**Studente**: Agel Jaupaj  
**Matricola**: 1057350
**Corso**: Sistemi Operativi  
**Anno accademico**: 2024/2025

## Descrizione

Il progetto simula il funzionamento di un ufficio postale per più giorni consecutivi.  
Sono coinvolti processi multipli che comunicano tramite message queue e memoria condivisa:

- **Direttore**: processo principale che avvia tutto.
- **Utenti**: decidono casualmente se andare in posta, richiedono un servizio.
- **Operatori**: ognuno specializzato in un tipo di servizio, servono gli utenti.
- **Ticket**: riceve richieste e le inoltra.
- **Statistiche**: salvate su file CSV giorno per giorno.

---

## Tecnologie e requisiti usati

- Comunicazione tra processi con **message queue** (IPC).
- **Memoria condivisa** e **semafori** verranno usati nelle versioni avanzate.
- Tutti i processi sono eseguibili separati, lanciati con `exec`.
- Compilazione tramite `make` con opzioni richieste (`-Werror`, `-Wall`, ecc.).
- Evitata attesa attiva (usiamo `nanosleep`, `msgrcv`, `msgsnd`, ecc.).
- Cleanup delle risorse IPC alla fine.

---

## Dichiarazione

Dichiaro che questo progetto è stato realizzato individualmente.