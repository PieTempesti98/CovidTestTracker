//Header file con le dichiarazioni di tutti i metodi del server

#include "../headers.h" //librerie di sistema

//descrittore dei peer
struct peer{
    int port;
    struct peer* previous;
    struct peer* next;
};

//ds_gui_routines.c
int gui(struct peer* list);

//list_utility.c
void list_add(struct peer* list, int peer);
void list_remove(struct peer* list, int peer);