#include "ds_headers.h"

uint8_t list_add(struct peer* list, struct sockaddr_in peer, int* tot_peers){
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
            if(ntohs(p->addr.sin_port) == ntohs(peer.sin_port))// il peer e' gia' in lista
                return 0;
            new.next = p->next;
            new.pos = p->pos + 1;
            p->dirty = 1;
            p->next = &new;
            //aggiornamento delle posizioni: incremento la posizione in lista dei peer successivi a quello inserito
            if(new.next != NULL){
                p = &new;
                while(p->next != NULL){
                    p = p->next;
                    p->pos++;
                }
            }
        }

    }
    *tot_peers ++;
    shortcut(list, *tot_peers);
    return 1;
}

uint8_t list_remove(struct peer* list, int peer, int* tot_peers){
    uint8_t result = 0;
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
        result = 1;
    }
    //aggiornamento della posizione per i peer successivi in lista a quello rimosso
    while(q->next != NULL){
        q->pos --;
        q = q->next;
    }
    if(result == 0) //nessun peer rimosso
        return result;
    *tot_peers --;
    shortcut(list, *tot_peers); //calcolo nuovi shortcut
    return result;
}

void shortcut(struct peer* list, int tot_peer){
    double db_peers = (double)tot_peer;
    int offset = (int)round(db_peers/2);
    struct peer* p, *q;

    p = list;
    while(p != NULL){
        int new_shortcut = (p->pos + offset)%tot_peer;
        q = p->shortcut;
        if(q->pos != new_shortcut){
            q = list;
            while(q != NULL){
                if(q->pos == new_shortcut){
                    p->shortcut = q;
                    p->dirty = 1;
                    break;
                }
                else
                    q = q->next;
            }
        }
        if(p->dirty == 1){
        }
        p = p->next;
    }
}
