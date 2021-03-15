#include "ds_headers.h"

void list_add(struct peer* list, struct sockaddr_in peer, int* tot_peers){
    struct peer new, *p, *n;

    new.addr = peer;
    new.shortcut = NULL;
    new.dirty = 1;
    if(list == NULL){ //Lista vuota: primo peer
        list = &new;
        new.next = NULL;
        new.pos = 0;

    }
    else {
        p = list;
        if(p->addr.sin_port > peer.sin_port){ //neighbor con numero di porta più piccolo, inserisco in testa
            list = &new;
            new.next = p;
            new.pos = 0;
        }
        else{
            while (p->next != NULL && ntohs(p->addr.sin_port) > ntohs(peer.sin_port)) {
                p = p->next;
                n = n->next;
            }
            new.next = p->next;
            new.pos = p->pos + 1;
            p->dirty = 1;
            p->next = &new;
            if(new.next != NULL){ //aggiornamento delle posizioni: incremento la posizione in lista dei peer successivi a quello inserito
                p = &new;
                while(p->next != NULL){
                    p = p->next;
                    p->pos++;
                }
            }
        }

    }
    *tot_peers ++;
    //calcolo shortcut
}

void list_remove(struct peer* list, int peer, int* tot_peers){
    struct peer *p = list, *q = p->next;
    if(ntohs(list->addr.sin_port) == peer){        //rimuovo il peer in testa

        list = list->next;
        q = list;
    }
    else {
        while (p->next != NULL && ntohs(q->addr.sin_port) == peer) { //scorro la lista per cercare il peer da rimuovere
            p = q;
            q = q->next;
        }
        //rimuovo il peer e correggo i neighbors
        p->next = q->next;
        p->dirty = 1;
    }
    //aggiornamento della posizione per i peer successivi in lista a quello rimosso
    while(q->next != NULL){
        q->pos --;
        q = q->next;
    }
    *tot_peers --;
    //calcolo nuovi shortcut
}

