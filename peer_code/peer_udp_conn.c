#include "peer_headers.h"

void udp_comm(int sd, struct sockaddr_in ds_addr, struct sockaddr_in* neighbors){
    int ret;
    unsigned int n1_port, n2_port;
    unsigned long n1_ip, n2_ip;
    char buffer[34];
    unsigned int addr_size = sizeof(ds_addr);

    while(1) {
        ret = recvfrom(sd, buffer, sizeof(buffer), 0, (struct sockaddr *) &ds_addr, &addr_size);
        if (ret < 0)
            perror("Errore di comunicazione col DS: ");
        printf("-SISTEMA- messaggio \"%s\" ricevuto dal DS %u\n",buffer, ntohs(ds_addr.sin_port));
        if (strcmp(buffer, "QUIT") == 0)
            break;
        else {
            sscanf(buffer, "%lu:%u %lu:%u", &n1_ip, &n1_port, &n2_ip, &n2_port);
            if (n1_port == neighbors[0].sin_port && n2_port == neighbors[1].sin_port) {
                ret = sendto(sd, buffer, sizeof(buffer), 0, (struct sockaddr *) &ds_addr, addr_size);
                if (ret < 0)
                    perror("Errore di comunicazione col DS: ");
            } else {
                neighbors[0].sin_family = AF_INET;
                neighbors[0].sin_port = n1_port;
                neighbors[0].sin_addr.s_addr = n1_ip;

                neighbors[1].sin_family = AF_INET;
                neighbors[1].sin_port = n2_port;
                neighbors[1].sin_addr.s_addr = n2_ip;
            }
            printf("-SISTEMA- I nuovi neighbors sono %u e %u\n", ntohs(neighbors[0].sin_port), ntohs(neighbors[1].sin_port));

        }

    }
}