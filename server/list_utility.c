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
            n = p->next;
            while (p->next != NULL && ntohs(n->addr.sin_port) < ntohs(peer.sin_port)) {
                if(ntohs(n->addr.sin_port) == ntohs(peer.sin_port)){
                    // il peer e' gia' in lista
                    return list;
                }

                p = p->next;
                n = p->next;
            }
            p->next = new;
            new->next = n;
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
    int result = 0;
    struct peer *p = list;
    struct peer* q;

    //Se qualche peer ha come shortcut il peer che rimuovo vado a dereferenziare il puntatore
    while(p != NULL){
        q = p->shortcut;
        if(q != NULL) {
            if (ntohs(q->addr.sin_port) == peer){
                p->shortcut = NULL;
            }
        }
        p = p->next;
    }
    p = list;
    if(ntohs(list->addr.sin_port) == peer){
        //rimuovo il peer in testa
        list = list->next;
        free(p);
        if(list != NULL)
            list->pos --;
        p = list;
        result = 1;
    }
    else if(p->next != NULL) {
        q = p->next;
        printf("scorro la lista\n");
        while (q->next != NULL && ntohs(q->addr.sin_port) != peer) {
            //scorro la lista per cercare il peer da rimuovere
            p = p->next;
            q = q->next;
        }
        if(ntohs(q->addr.sin_port) == peer) {
            //rimuovo il peer e correggo i neighbors
            p->next = q->next;
            free(q);
            p->dirty = 1;
            result = 1;
        }
    }
    if(result == 0) //nessun peer rimosso
        return list;
    //aggiornamento della posizione per i peer successivi in lista a quello rimosso e del totale dei peer
    tot_peers --;
    if(tot_peers > 0)
        while(p->next != NULL){
            p = p->next;
            p->pos --;
        }
    //calcolo nuovi shortcut
    return shortcut(list, tot_peers);
}

struct peer* shortcut(struct peer* list, int tot_peer){
    int offset;
    struct peer* p, *q;
    //Calcolo la distanza tra il peer e lo shortcut solo se ho piu' di due peer connessi
    //Con due peer o meno non uso gli shortcut
    if(tot_peer > 2) {
        if (tot_peer % 2 == 0)
            offset = tot_peer / 2;
        else
            offset = tot_peer / 2 + 1;
        p = list;
        while (p != NULL) {
            //Calcolo la posizine del nuovo shortcut: se e' diversa dalla osizione dello shortcut attualmente puntato
            //allora aggiorno il puntatore allo shortcut corretto
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
    p = list;
    //Setto i bit dirty ad 1 nel caso in cui sia stato rimosso lo shortcut e/o sia cambiato il peer in testa alla lista,
    //in modo che sia aggiornato il next del peer in coda
    while(p != NULL){
        if(p->next == NULL || p->shortcut == NULL)
            p->dirty = 1;
        p = p->next;
    }
    return list;
}
