#include "ds_headers.h"

void list_add(struct peer* list, struct sockaddr_in peer){
    struct peer new, *p, *n;

    new.addr = peer;
    if(list == NULL){ //aggiungo in testa
        list = &new;
        new.next = new.previous = NULL;
        // comunicazione col peer
        return;
    }
    p = list;
    n = list->next;
    while(n != NULL){
        if(ntohs(n->addr.sin_port) > ntohs(peer.sin_port)){ //aggiungo in mezzo alla lista
            p->next = &new;
            new.next = n;
            new.previous = p;
            n->previous = &new;
            //comunicazione col peer
            return;
        }
        p = p->next;
        n = n->next;
    }
    //inserisco in coda il nuovo peer e correggo i neighbors
    n = p;
    n->next = &new;
    n = n->next;
    n->previous = p;
    if(p != list) {
        list->previous = n;
    }
    //comunicazione col peer
}

void list_remove(struct peer* list, int peer){
    struct peer *p, *q;
    if(ntohs(list->addr.sin_port) == peer){
        //rimuovo il peer in testa e correggo il neighbor della nuova testa
        p = list;
        list = list->next;
        list->previous = p->previous;
        //comunicazione coi peer a cui aggiornare i neighbor
        return;
    }
    p = list;
    q = list->next;
    while(q != NULL){
        if(ntohs(q->addr.sin_port) == peer){
            //rimuovo il peer e correggo i neighbors
            p->next = q->next;
            if(q->next != NULL){
                q = q->next;
                q->previous = p;
            }
            //comunicazione coi peer a cui aggiornare i neighbors
            return;
        }
        p = q;
        q = q->next;
    }
}

