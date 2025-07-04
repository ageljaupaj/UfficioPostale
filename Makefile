# Makefile for UfficioPostale project

CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wvla -std=gnu99 -D_GNU_SOURCE
INCLUDES = -Iinclude

SRC_DIR = src
INCLUDE_DIR = include

EXECUTABLES = direttore ticket operatore utente add_users

all: $(EXECUTABLES)

direttore: $(SRC_DIR)/direttore.c $(INCLUDE_DIR)/common.h $(INCLUDE_DIR)/utils.h
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(SRC_DIR)/utils.c

ticket: $(SRC_DIR)/ticket.c $(INCLUDE_DIR)/common.h $(INCLUDE_DIR)/utils.h
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(SRC_DIR)/utils.c

operatore: $(SRC_DIR)/operatore.c $(INCLUDE_DIR)/common.h $(INCLUDE_DIR)/utils.h
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(SRC_DIR)/utils.c

utente: $(SRC_DIR)/utente.c $(INCLUDE_DIR)/common.h $(INCLUDE_DIR)/utils.h
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(SRC_DIR)/utils.c

add_users: $(SRC_DIR)/add_users.c $(INCLUDE_DIR)/common.h
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm -f *.o $(EXECUTABLES)

.PHONY: all clean
