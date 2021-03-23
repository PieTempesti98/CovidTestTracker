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
    printf("metodo help non ancora implementato\n");
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
            if(shortcut != NULL){
                printf("1. %d\n", ntohs(shortcut->addr.sin_port));
                if(next != NULL)
                    printf("2. %d\n", ntohs(next->addr.sin_port));
            }
            else if(next != NULL)
                printf("1. %d\n", ntohs(next->addr.sin_port));
            else
                printf("Il peer indicato al momento non ha neighbors\n");
            presente = 1;
        }
        p = p->next;
    }
    if(presente == 0)
        printf("Il peer indicato non e' connesso al ds\n");
}

int gui(struct peer* list){
    char input[13];
    int* peer = NULL;
    scanf("%s %d", input, peer);
    if(strcmp(input, "esc") == 0)
        return 0;
    if(strcmp(input, "showpeers") == 0)
        showpeers(list);
    else if(strcmp(input, "help") == 0)
        showhelp();
    else if(strcmp(input, "showneighbor") == 0){
        if(peer == NULL)
            peer = 0;
        showneighbor(list, *peer);
    }

    return 1;
}
