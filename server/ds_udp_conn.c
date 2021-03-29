#include"ds_headers.h"

struct peer* ds_boot(int sd, struct peer* list, int* tot_peers, pthread_mutex_t* list_mutex) {
    int ret;
    unsigned int rcv_msg;
    int result;
    char buffer[MSG_STD_LEN];
    struct sockaddr_in peer_addr;
    unsigned int addr_size = sizeof(peer_addr);
    struct peer* p;
        ret = recvfrom(sd, buffer, MSG_STD_LEN, 0, (struct sockaddr *) &peer_addr, &addr_size);
        if (ret < 0) {
            printf("C'e' stato un errore/n");
            perror("Errore in ricezione sul socket UDP: ");
        }
        printf("-SISTEMA- Ricevuto il messaggio %s dal peer sulla porta %u \n", buffer, htons(peer_addr.sin_port));
        if (strcmp(buffer, "STOP") == 0) { //Il peer viene chiuso, devo aggiornare lista e neighbors
            pthread_mutex_lock(list_mutex);
            list = list_remove(list, ntohs(peer_addr.sin_port), *tot_peers); //rimuovo il peer dalla lista, se presente
            /*
            * Assumo che richieste di quit di peer non presenti nella lista siano dovute a perdita dell'ack,
            * quindi provvedero' ad iviarlo ogni volta che arriva una richiesta di quit; solo quando si verifica
            * effettivamente la rimozione di un peer provvedo all'aggiornamento dei neighbors*/
            ret = sendto(sd, buffer, MSG_STD_LEN, 0, (struct sockaddr *) &peer_addr, sizeof(peer_addr));
            if (ret < 0)
                perror("Errore in invio sul socket UDP: ");
            p = list;
            result = 0;
            while (p != NULL) {
                result++;
                p = p->next;
            }
            if (result != *tot_peers) {
                *tot_peers = result;
                printf("-SISTEMA- Peer %d rimosso dalla lista dei peer attivi\n", ntohs(peer_addr.sin_port));
                neighbors_update(sd, &list);
            }
            pthread_mutex_unlock(list_mutex);
        }
        //il peer ha richiesto il boot, quindi lo aggiungo alla lista e sistemo i neighbors
        sscanf(buffer, "%u", &rcv_msg);
        if (rcv_msg == ntohs(peer_addr.sin_port)) {
            pthread_mutex_lock(list_mutex);
            list = list_add(list, peer_addr, *tot_peers);
            p = list;
            result = 0;
            while (p != NULL) {
                result++;
                p = p->next;
            }
            if (result != *tot_peers) {
                *tot_peers = result;
                /* tratto il nuovo peer come un peer a cui ho aggiornato i neighbors, quindi invio la sua lista mentre
                * invio tutte le liste aggiornate */
                printf("-SISTEMA- Effettuato il boot del peer %d \n", ntohs(peer_addr.sin_port));
                if (list != NULL)
                    neighbors_update(sd, &list);
            }
            pthread_mutex_unlock(list_mutex);
        }
    return list;

}

void neighbors_update(int sd, struct peer** list){
    struct peer *p, *n1, *n2;
    p = *list;
    //printf("Il peer %d ha il bit dirty a %d\n", ntohs(list->addr.sin_port), list->dirty);
    while(p != NULL){
        if(p->dirty == 1){
            int ret = 0; //lo inizializzo a 0 per effettuare il primo invio di dati
            struct sockaddr_in peer_addr;
            fd_set read_fds;
            //Variabili per memorizzare ip e porte in formato network
            unsigned long ip1, ip2;
            unsigned int port1, port2;
            /* long unsigned su 32 bit: massimo 10 caratteri per la rappresentazione in stringa
             * unsigned int su 16 bit: massimo 5 caratteri per la rappresentazione in stringa
             * formato del messaggio: [ip(network):porta(network) ip(network):porta(network)]
             * buffer di grandezza massima: 10 + 1 + 5 + 1 + 10 + 1 + 5 + 1 = 34 bytes
             */
            unsigned int addr_size = sizeof(peer_addr);
            unsigned int buf_size = 34;
            char buffer[buf_size];

            //timeout di ricezione dell'ack: 1 secondo
            struct timeval timeout;

            //estraggo il sockaddr_in dei due neighbors
            if(p->next == NULL && p->pos != 0) //Ultimo in lista: il NEXT e' l'elemento in testa
                n1 = *list;
            else
                n1 = p->next;
            n2 = p->shortcut;
            //scrivo il messaggio nel formato
            if(n1 == NULL){//Se non ho neighbor passo IP e porta pari a 0 come convenzione
                ip1 = 0;
                port1 = 0;
            }
            else{
                ip1 = n1->addr.sin_addr.s_addr;
                port1 = n1->addr.sin_port;
            }
            if(n2 == NULL || (n2 != NULL && (n1 != NULL && n1->addr.sin_port == n2->addr.sin_port))){//Se non ho neighbor passo IP e porta pari a 0 come convenzione
                ip2 = 0;
                port2 = 0;
            }
            else{
                ip2 = n2->addr.sin_addr.s_addr;
                port2 = n2->addr.sin_port;
            }
            memset(buffer, 0, sizeof(buffer));
            memset(&peer_addr, 0, sizeof(peer_addr));
            peer_addr = p->addr;
            sprintf(buffer, "%lu:%u %lu:%u", ip1, port1, ip2, port2);
            //invio il messaggio al peer e mi metto in attesa dell'ack
            do {
                if(ret == 0) {
                    //aggiungo nell lista di lettura il socket
                    timeout.tv_sec = 3;
                    timeout.tv_usec = 0;
                    FD_ZERO(&read_fds);
                    FD_SET(sd, &read_fds);
                    printf("-SISTEMA- Invio il messaggio %s al peer sulla porta %d\n", buffer, ntohs(p->addr.sin_port));
                    ret = sendto(sd, buffer, buf_size, 0, (struct sockaddr *) &p->addr, addr_size);
                    if (ret < 0)
                        perror("Errore di invio dei neighbors: ");
                } else if (ret == 1){
                    char rec_buffer[buf_size];
                    memset(rec_buffer, 0, sizeof(rec_buffer));
                    memset(&peer_addr, 0, sizeof(peer_addr));
                    ret = recvfrom(sd, rec_buffer, buf_size, 0, (struct sockaddr *) &peer_addr, &addr_size);
                    if (ret < 0)
                        perror("Errore di ricezione ack dai neighbors: ");
                    else {
                        //l'ack ricevuto e' un echo del messaggio inviato dal DS
                        if (strcmp(buffer, rec_buffer) == 0 && peer_addr.sin_port == p->addr.sin_port)
                            break;
                        // ack ricevuto, esco dal ciclo e passo al peer successivo
                    }
                }
                //aspetto l'ack
            }while((ret = select(sd + 1, &read_fds, NULL, NULL, &timeout)) < 2);
            p->dirty = 0;
            printf("-SISTEMA- Aggiornati i neighbors del peer %d \n", ntohs(p->addr.sin_port));
        }
         p = p->next;
    }
}
//Invio il segnale di quit a tutti i peer e svuoto la lista
void quit(int sd, struct peer* list){
    fd_set read_fds;
    char* msg = "QUIT";
    int ret, ack;
   struct timeval timeout;
   //timeout per l'ack di quit

    while(list != NULL){
        struct peer* p = list;
        list = list->next; //svuoto la lista
        ack = 0;
        do{ //invio il segnale di quit al peer p e mi metto in attesa della risposta
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            FD_ZERO(&read_fds);
            FD_SET(sd, &read_fds);
            printf("-SISTEMA- Invio comando di spengimento al peer %u\n", ntohs(p->addr.sin_port));
            ret = sendto(sd, msg, strlen(msg) + 1,0, (struct sockaddr*)&p->addr, sizeof(p->addr));
            if(ret < 0)
                perror("Errore invio comando di spengimento: ");
            if(select(sd + 1, &read_fds, NULL, NULL, &timeout) == 1) {
                //Pronto a ricevere l'ack, prepro le variabili di ricezione
                struct sockaddr_in recv_addr;
                unsigned int addr_len = sizeof(recv_addr);
                char echo[strlen(msg) + 1];
                memset(&recv_addr, 0, sizeof(recv_addr));
                memset(echo, 0, strlen(msg) + 1);
                ret = recvfrom(sd, echo, strlen(msg) + 1, 0, (struct sockaddr*)&recv_addr, &addr_len);
                if(ret < 0)
                    perror("Errore ricezione ack di spengimento: ");
                //controllo di aver ricevuto l'ack dal mittente corretto, se si chiudo il ciclo
                if(strcmp(echo, msg) == 0 && ntohs(recv_addr.sin_port) == ntohs(p->addr.sin_port)){
                    printf("-SISTEMA- Ricevuto ack di spengimento dal peer %u\n", ntohs(p->addr.sin_port));
                    ack = 1;
                    free(p);
                }

            }
        }while(ack == 0);
    }
}

void end_day(struct param* param) {
    time_t now_t = time(NULL); //time_t per risalire all'ora corrente
    struct tm ref = *localtime(&now_t), now = *localtime(&now_t); //strutture tm per il confronto della data
    char buffer[MSG_STD_LEN];
    struct sockaddr_in peer_addr;
    unsigned int peer_size = sizeof(peer_addr);
    fd_set read_fd;
    struct timeval timeout;
    struct peer *p;
    int ret;

    ref.tm_hour = 18;
    ref.tm_min = 0;
    ref.tm_sec = 0;
    if(now.tm_hour >= 18)//se accendo il DS dopo le 18 setto subito il ref al giorno successivo
        ref.tm_mday ++;
    while(1){
        //finche' now non raggiunge le 18 continuo ad aggiornarlo
        while (difftime(mktime(&ref), mktime(&now)) > 0) {
            now_t = time(NULL);
            now = *localtime(&now_t);
        }
        printf("ora di aggiornare i register: avverto i peer\n");
        pthread_mutex_lock(*param->list_mutex); //metto il lock alla lista: devo inviare i messaggi ai vari peer
        printf("lock del mutex settato\n");
        p = *param->list;
        ret = pthread_mutex_unlock(*param->list_mutex);//sblocco la lista
        if(ret != 0)
            perror("Errore di sblocco del mutex: ");
        printf("Mutex sbloccato, posso continuare con le interazioni");
        while (p != NULL) {
            int ack = 0;
            printf("ora di aggiornare i register: avverto i peer\n");
            do {
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;
                FD_ZERO(&read_fd);
                FD_SET(param->sd, &read_fd);
                strcpy(buffer, "ENDD");
                printf("-SISTEMA- Invio comando ENDD al peer %u\n", ntohs(p->addr.sin_port));
                ret = sendto(param->sd, buffer, MSG_STD_LEN, 0, (struct sockaddr *) &p->addr, sizeof(p->addr));
                if (ret < 0)
                    perror("Errore invio comando di spengimento: ");
                if (select(param->sd + 1, &read_fd, NULL, NULL, &timeout) == 1) {
                    //Pronto a ricevere l'ack, prepro le variabili di ricezione);
                    memset(&peer_addr, 0, peer_size);
                    memset(buffer, 0, MSG_STD_LEN);
                    ret = recvfrom(param->sd, buffer, MSG_STD_LEN, 0, (struct sockaddr *) &peer_addr, &peer_size);
                    if (ret < 0)
                        perror("Errore ricezione ack di spengimento: ");
                    //controllo di aver ricevuto l'ack dal mittente corretto, se si chiudo il ciclo
                    if (strcmp(buffer, "ENDD") == 0 && ntohs(peer_addr.sin_port) == ntohs(p->addr.sin_port)) {
                        printf("-SISTEMA- Ricevuto ack ENDD dal peer %u\n", ntohs(p->addr.sin_port));
                        ack = 1;
                    }
                }
            } while (ack == 0);
            p = p->next;
        }
        printf("Lista completamente scansionata \n");
        ref.tm_mday++;//incremento ref al giorno successivo
    }
}