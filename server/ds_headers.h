//Header file con le dichiarazioni di tutti i metodi del server

#include "../headers.h" //librerie di sistema

//descrittore dei peer
struct peer{
    struct sockaddr_in addr;
    struct peer* shortcut;
    struct peer* next;
    uint16_t pos;
    int dirty;
};

//struttura dati che contiene i parametri della end_day da passare al thread di supporto
struct param{
    pthread_mutex_t** list_mutex;
    struct peer** list;
    int sd;
};

//ds_gui_routines.c
int gui(struct peer* list, pthread_mutex_t* list_mutex);

//list_utility.c
struct peer* list_add(struct peer* list, struct sockaddr_in peer, int tot_peers);
struct peer* list_remove(struct peer* list, int peer, int tot_peers);
struct peer* shortcut(struct peer* list, int tot_peers);

//ds_udp_conn.c
struct peer* ds_boot(int sd, struct peer* list, int* tot_peers, pthread_mutex_t* list_mutex);
void neighbors_update(int sd, struct peer** list);
void quit(int sd, struct peer* list);
void end_day(struct param* param);