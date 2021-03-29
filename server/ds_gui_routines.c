#include "ds_headers.h"

void showpeers(struct peer* list){
    struct peer* p;

    if(list == NULL){
        printf("Al momento non ci sono peer connessi\n");
        return;
    }
    p = list;
    printf("Elenco dei peer connessi: \n");
    while(p != NULL){
        printf("%d ", ntohs(p->addr.sin_port));
        p = p->next;
    }
    printf("\n");
}

void showhelp(){
    printf("SHOWPEERS: permette di vedere una lista aggiornata dei peer connessi al DS\n"
           "SHOWNEIGHBOR [peer]: permette di vedere chi sono i neighbors del peer specificato; se non viene passato"
           " alcun parametro si visualizzeranno i neighbor di tutti i peer.\n"
           "ESC: Avvisa i peer e spenge il DS: i peer salveranno le entries raccolte durante la giornata e si "
           "spengeranno a loro volta\n");
}

void showneighbor(struct peer* list, int peer){
    struct peer* p;
    int presente = 0;

    if(list == NULL){
        printf("Al momento non ci sono peer connessi.\n");
        return;
    }
    p = list;
    while(p != NULL){
        if(ntohs(p->addr.sin_port) == peer || peer == 0){
            struct peer* shortcut; struct peer* next;
            shortcut = p->shortcut;
            next = p->next;
            printf("Neighbors del peer %u:\n", ntohs(p->addr.sin_port));
            if(shortcut != NULL){
                printf("1. %d\n", ntohs(shortcut->addr.sin_port));
                if(next != NULL)
                    printf("2. %d\n", ntohs(next->addr.sin_port));
                else if(next == NULL && p->pos != 0)//Peer in coda: il neighbor next e' il peer in testa
                    printf("2. %d\n", ntohs(list->addr.sin_port));
            }
            else if(next != NULL)
                printf("1. %d\n", ntohs(next->addr.sin_port));
            else if(next == NULL && p->pos != 0)//Peer in coda: il neighbor next e' il peer in testa
                printf("1. %d\n", ntohs(list->addr.sin_port));
            else
                printf("Il peer indicato al momento non ha neighbors\n");
            presente = 1;
        }
        p = p->next;
    }
    if(presente == 0)
        printf("Il peer indicato non e' connesso al ds\n");
}

int gui(struct peer* list, pthread_mutex_t* list_mutex){
    char* input = (char*)malloc(20);
    int peer;
    input = fgets(input, 20, stdin);
    if(strncmp(input, "esc",3) == 0)
        return 0;
    pthread_mutex_lock(list_mutex);
    if(strncmp(input, "showpeers", 9) == 0)
        showpeers(list);
    else if(strncmp(input, "help",4) == 0)
        showhelp();
    else if(strlen(input) > 13) {
        char* p = strtok(input, " ");
        peer = atoi(strtok(NULL, "\n"));
        if(strncmp(p, "showneighbor",12) == 0)
            showneighbor(list, peer);
    }
    else if(strncmp(input, "showneighbor",12) == 0)
        showneighbor(list, 0);
    pthread_mutex_unlock(list_mutex);
    return 1;
}
