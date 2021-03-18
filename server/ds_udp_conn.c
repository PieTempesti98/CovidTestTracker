#include"ds_headers.h"

void ds_boot(int sd, struct peer* list, int* tot_peers){
    int ret, porta, rcv_msg;
    uint8_t result;
    char buffer[1024];
    struct sockaddr_in peer_addr;
    while(1){
        ret = recvfrom(sd, buffer, sizeof(int), 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
        if(ret < 0){
            perror("Errore in ricezione sul socket UDP: ");
            if(errno == EBADF){ //Il socket UDP e' stato chiuso
                break;
            }
        }
        else{
            if(strcmp(buffer, "QUIT") == 0){ //Il peer viene chiuso, devo aggiornare lista e neighbors
                rcv_msg == ntohs(peer_addr.sin_port);
                result = list_remove(list, rcv_msg, tot_peers); //rimuovo il peer dalla lista, se presente
                /*
                 * Assumo che richieste di quit di peer non presenti nella lista siano dovute a perdita dell'ack,
                 * quindi provvedero' ad iviarlo ogni volta che arriva una richiesta di quit; solo quando si verifica
                 * effettivamente la rimozione di un peer provvedo all'aggiornamento dei neighbors*/
                strcpy(buffer, "ACK");
                ret = sendto(sd, buffer, sizeof(buffer), 0, (struct sockaddr*)&peer_addr, sizeof(peer_addr));
                if(ret < 0)
                    perror("Errore in invio sul socket UDP: ");
                if(result == 1){
                    //aggiornamento dei neighbors
                }
            }
            //il peer ha richiesto il boot, quindi lo aggiungo alla lista e sistemo i neighbors
            if(rcv_msg == ntohs(peer_addr.sin_port)){
                result = list_add(list, peer_addr, tot_peers);
                if(result == 1)
                /* tratto il nuovo peer come un peer a cui ho aggiornato i neighbors, quindi invio la sua lista mentre
                 * invio tutte le liste aggiornate */
                neighbors_update(sd, list);
            }
        }
    }
}


void neighbors_update(int sd, const struct peer* list){
    struct peer *p = list;
    while(p != NULL){
        if(p->dirty == 1){
            //estraggo il numero di porta dei due neighbors
            //scrivo il messaggio nel formato [porta porta]
            //invio il messaggio al peer
            //aspetto l'ack
            p->dirty = 0;
        }
         p = p->next;
    }
}