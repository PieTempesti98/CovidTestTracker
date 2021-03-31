#include "peer_headers.h"

int udp_comm(int sd, struct sockaddr_in ds_addr, struct sockaddr_in neighbors[2]){
    int ret;
    unsigned int n1_port, n2_port;
    unsigned long n1_ip, n2_ip;
    char buffer[34]; //dimensione del messaggio di aggiornamento eighbors
    unsigned int addr_size = sizeof(ds_addr);

    ret = recvfrom(sd, buffer, sizeof(buffer), 0, (struct sockaddr *) &ds_addr, &addr_size);
    if (ret < 0)
        perror("Errore di comunicazione col DS: ");
    printf("-SISTEMA- messaggio \"%s\" ricevuto dal DS %u\n",buffer, ntohs(ds_addr.sin_port));
    if (strcmp(buffer, "QUIT") == 0){
        //Messaggio di chiusura del DS: invio l'ack e
        // chiudo la connessione
        ret = sendto(sd, buffer, sizeof(buffer), 0, (struct sockaddr*)&ds_addr, addr_size);
        if (ret < 0) {
            perror("Errore di comunicazione col DS: ");
            return 1;
        }
        return 0;
    }
    if (strcmp(buffer, "ENDD") == 0){
        //Messaggio di chiusura delle entries del giorno: invio l'ack e aggiorno il register
        ret = sendto(sd, buffer, sizeof(buffer), 0, (struct sockaddr*)&ds_addr, addr_size);
        if (ret < 0) {
            perror("Errore di comunicazione col DS: ");
            return 1;
        }
        return 2;
    }
    else {
        //Messaggio di aggiornamento neighbors: invio l'ack di ricezione e li aggiorno selo se sono diversi
        sscanf(buffer, "%lu:%u %lu:%u", &n1_ip, &n1_port, &n2_ip, &n2_port);
        if (n1_port == neighbors[0].sin_port && n2_port == neighbors[1].sin_port) {

        } else {
            neighbors[0].sin_family = AF_INET;
            neighbors[0].sin_port = n1_port;
            neighbors[0].sin_addr.s_addr = n1_ip;

            neighbors[1].sin_family = AF_INET;
            neighbors[1].sin_port = n2_port;
            neighbors[1].sin_addr.s_addr = n2_ip;
        }
        ret = sendto(sd, buffer, sizeof(buffer), 0, (struct sockaddr *) &ds_addr, addr_size);
        if (ret < 0) {
            perror("Errore di comunicazione col DS: ");
            return 1;
        }
        return 1;
    }

}

int start(int udp_socket, struct sockaddr_in ds_addr, int porta, struct sockaddr_in neighbors[2]){
    int ret;
    unsigned int n1_port, n2_port;
    unsigned long n1_ip, n2_ip;
    char* buffer;
    fd_set master;
    struct timeval timeout;
    unsigned int size_addr = sizeof(ds_addr);
    do{
        //setto il timeout e inserisco il socket UDP nella fd_list
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        FD_ZERO(&master);
        FD_SET(udp_socket, &master);
        //Carico il numero di porta nel buffer e lo invio al DS
        buffer = malloc(MSG_STD_LEN);
        sprintf(buffer,"%d", porta);
        ret = sendto(udp_socket, buffer, MSG_STD_LEN, 0, (struct sockaddr*) &ds_addr, size_addr);
        if(ret < 0) {
            perror("Errore di comunicazione col DS: ");
            return 0;
        }
        printf("-SISTEMA- inviato il messaggio di boot %s al DS sulla porta %u\n", buffer, ntohs(ds_addr.sin_port));
        free(buffer);
        // Mi metto in attesa della risposta del DS, se non arriva entro il timeout rimando il
        // messaggio di boot
        ret = select(udp_socket + 1, &master, NULL, NULL, &timeout);
        if(ret == 1) {
            buffer = (char*)malloc(34);
            memset(buffer, 0, 34);
            memset(&ds_addr, 0, size_addr);
            ret = recvfrom(udp_socket, buffer, 34, 0, (struct sockaddr *) &ds_addr, &size_addr);
            if(ret < 0) {
                perror("Errore di comunicazione col DS: ");
                return 0;
            }
            sscanf(buffer, "%lu:%u %lu:%u", &n1_ip, &n1_port, &n2_ip, &n2_port);
            printf("-SISTEMA- Ricevuto il messagio %lu:%u %lu:%u dal DS sulla porta %u\n", n1_ip, n1_port, n2_ip, n2_port, ntohs(ds_addr.sin_port));
            //neigbors ricevuti, posso uscire dal ciclo
            break;
        }
    }
    while(1);
    //Scrivo nell'array gli indirizzi dei due neighbors assegnati (0 = NON ASSEGNATO)
    neighbors[0].sin_family = AF_INET;
    neighbors[0].sin_port = n1_port;
    neighbors[0].sin_addr.s_addr = n1_ip;

    neighbors[1].sin_family = AF_INET;
    neighbors[1].sin_port = n2_port;
    neighbors[1].sin_addr.s_addr = n2_ip;

    //Invio l'ack di ricezione dei neighbors
    ret = sendto(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&ds_addr, size_addr);
    if (ret < 0)
        perror("Errore di comunicazione col DS: ");
    return 1;
}

void stop(int sd, struct sockaddr_in ds_addr){
    int ret;
    unsigned int addr_len = sizeof(ds_addr);
    char buffer[MSG_STD_LEN];
    fd_set read_fds;
    struct timeval timeout;

    strcpy(buffer, "STOP");
    ret = sendto(sd, buffer, MSG_STD_LEN, 0, (struct sockaddr*)&ds_addr, addr_len);
    if(ret < 0) {
        perror("Errore di comunicazione col DS: ");
        return;
    }
    printf("-SISTEMA- Inviato messaggio %s al DS %u\n", buffer, ntohs(ds_addr.sin_port));
    while(1){
        FD_ZERO(&read_fds);
        FD_SET(sd, &read_fds);
        timeout.tv_sec = 2;
        timeout.tv_usec = 0;
        ret = select(sd + 1, &read_fds, NULL, NULL, &timeout);
        if(ret == 1){
            int ds_port = ntohs(ds_addr.sin_port);
            memset(buffer, 0, MSG_STD_LEN);
            memset(&ds_addr, 0, addr_len);
            ret = recvfrom(sd, buffer, MSG_STD_LEN, 0, (struct sockaddr*)&ds_addr, &addr_len);
            if(ret < 0) {
                perror("Errore di comunicazione col DS: ");
                return;
            }
            printf("-SISTEMA- Ricevuto messaggio %s dal DS %u\n", buffer, ntohs(ds_addr.sin_port));
            if(strcmp("STOP", buffer) == 0 && ds_port == ntohs(ds_addr.sin_port))
                break;
        }
    }
    printf("-SISTEMA- Spengimento del peer approvato dal DS\n");
}