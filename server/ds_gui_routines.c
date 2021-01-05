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
        printf("%d ", p->port);
        p = p->next;
    }
    printf("\n");
}

void showhelp(){
    printf("metodo help non ancora implementato\n");
};
void showneighbor(struct peer* list, int peer){
    struct peer* p;
    if(list == NULL){
        printf("Al momento non ci sono peer connessi.\n");
        return;
    }
    p = list;
    while(p != NULL){
        if(p->port == peer){
            struct peer* prev; struct peer* next;
            prev = p->previous;
            next = p->next;
            if(prev != NULL){
                printf("1. %d\n", prev->port);
                if(next != NULL)
                    printf("2. %d\n", next->port);
            }
            else if(next != NULL)
                printf("1. %d\n", next->port);
            else
                printf("Il peer indicato al momento non ha neighbors");
            return;
        }
        p = p->next;
    }
    printf("Il peer indicato non e' connesso al ds\n");
};

int gui(struct peer* list){
    char input[20];

    printf("Digita un comando:\n\n"
           "1. help --> Mostra i dettagli dei comandi\n"
           "2. showpeers --> Mostra un elenco dei peer connessi\n"
           "3. showneighbor <peer> --> Mostra i neighbor del peer specificato\n"
           "4. esc --> Chiudi il DS\n\n"
    );
    scanf("%s", input);
    if(strcmp(input, "esc") == 0)
        return 0;
    if(strcmp(input, "showpeers") == 0)
        showpeers(list);
    else if(strcmp(input, "help") == 0)
        showhelp();
    else if(strcmp(input, "showneighbor") == 0){
        int peer;
        scanf("%d",&peer);
        showneighbor(list, peer);
    }

    return 1;
}
