#include "ds_headers.h"

struct peer* list_add(struct peer* list, struct sockaddr_in peer, int tot_peers){
    struct peer *new, *p, *n;
    new = (struct peer*)malloc(sizeof(struct peer));
    new->addr = peer;
    new->shortcut = NULL;
    new->dirty = 1;
    if(list == NULL){ //Lista vuota: primo peer
        new->next = NULL;
        new->pos = 0;
        list = new;
    }
    else {
        p = list;
        if(ntohs(p->addr.sin_port) == ntohs(peer.sin_port)){
            // il peer e' gia' in lista
            return list;
        }
        if(p->addr.sin_port > peer.sin_port){
            new->next = p;
            new->pos = 0;//neighbor con numero di porta più piccolo, inserisco in testa
            list = new;
        }
        else{
            while (p->next != NULL && ntohs(p->addr.sin_port) > ntohs(peer.sin_port)) {
                if(ntohs(p->addr.sin_port) == ntohs(peer.sin_port)){
                    // il peer e' gia' in lista
                    return list;
                }

                p = p->next;
                n = n->next;
            }
            new->next = p->next;
            new->pos = p->pos + 1;
            p->dirty = 1;
            p->next = new;
        }
        //aggiornamento delle posizioni: incremento la posizione in lista dei peer successivi a quello inserito
        if(new->next != NULL){
            p = new;
            while(p->next != NULL){
                p = p->next;
                p->pos++;
            }
        }
    }
    tot_peers ++;
    return shortcut(list, tot_peers);
}

struct peer* list_remove(struct peer* list, int peer, int tot_peers){
    uint8_t result = 0;
    struct peer *p = list, *q = p->next;
    if(ntohs(list->addr.sin_port) == peer){//rimuovo il peer in testa
        list = list->next;
        free(p);
        q = list;
    }
    else {
        while (p->next != NULL && ntohs(q->addr.sin_port) == peer) { //scorro la lista per cercare il peer da rimuovere
            p = q;
            q = q->next;
        }
        //rimuovo il peer e correggo i neighbors
        p->next = q->next;
        free(q);
        p->dirty = 1;
        result = 1;
        q = p->next;
    }
    //aggiornamento della posizione per i peer successivi in lista a quello rimosso
    while(q->next != NULL){
        q->pos --;
        q = q->next;
    }
    if(result == 0) //nessun peer rimosso
        return list;
    tot_peers --;
    return shortcut(list, tot_peers); //calcolo nuovi shortcut
}

struct peer* shortcut(struct peer* list, int tot_peer){
    int offset;
    struct peer* p, *q;

    printf("Calcolo gli shortcut\n");
    if(tot_peer > 2) {
        if (tot_peer % 2 == 0)
            offset = tot_peer / 2;
        else
            offset = tot_peer / 2 + 1;
        printf("L'offset e' %d\n", offset);
        p = list;
        while (p != NULL) {
            int new_shortcut = (p->pos + offset) % tot_peer;
            q = p->shortcut;
            if (q == NULL || q->pos != new_shortcut) {
                q = list;
                while (q != NULL) {
                    if (q->pos == new_shortcut) {
                        p->shortcut = q;
                        p->dirty = 1;
                        break;
                    } else
                        q = q->next;
                }
            }
            if (p->dirty == 1) {
            }
            p = p->next;
        }
    }
    printf("Il bit dirty del primo elemento in lista e' %d\n", list->dirty);
    return list;
}
