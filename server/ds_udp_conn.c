#include"ds_headers.h"

void ds_boot(int sd, peer* list, int* tot_peers){
    int ret, porta, rcv_msg;
    char buffer[1024];
    struct sockaddr_in peer_addr;
    while(1){
        ret = recvfrom(sd, buffer, sizeof(int), 0, (struct sockaddr*)&peer_addr, &sizeof(peer_addr));
        if(ret < 0){
            perror("Errore in ricezione sul socket UDP: ");
            if(errno == EBADF){ //Il socket UDP e' stato chiuso
                break;
            }
        }
        else{
            if(strcmp(buffer, "QUIT") == 0){ //Il peer viene chiuso, devo aggiornare lista e neighbors

            }
            if(strcmp(buffer, "ACK") == 0){ //Il peer che invia ha ricevuto l'aggiornamento dei neighbors
            }
            rcv_msg = sscanf("%d", buffer);
            if(rcv_msg == ntohs(peer_addr.sin_port)){ //il peer ha richiesto il boot, quindi lo aggiungo alla lista e sistemo i neighbors
                list_add(list, peer_addr, tot_peers);

            }
        }
    }


}