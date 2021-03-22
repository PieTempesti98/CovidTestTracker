#include "peer_headers.h"

void gui(int udp_socket, int porta){
    uint8_t stop = 1;
    char input[50];
    int udp_pid;
    struct sockaddr_in ds_addr;
    while(stop){
        printf("Digita un comando:\n");
        printf("1. Start <indirizzo del DS> <porta del DS> --> collega il peer alla rete\n");
        printf("2. Add <tipo> <quantita'> --> Aggiungi occorrenze dell'evento indicato\n");
        printf("3. Get <aggr> <tipo> <periodo> --> Visualizza un valore aggregato\n");
        printf("4. Stop --> Disconnetti il peer e spengilo\n");
        scanf("%s", input);
        if(strcmp(input, "stop") == 0)
            stop = 0;
        else if(strcmp(input, "start") == 0){
            int ds_port;
            char ds_ip[100];
            scanf("%s", ds_ip);
            scanf("%d", &ds_port);
            ds_addr.sin_family = AF_INET;
            ds_addr.sin_port = htons(ds_port);
            inet_pton(AF_INET, ds_ip, &ds_addr.sin_addr.s_addr);
            udp_pid = start(udp_socket, ds_addr, porta);
        }
        else if(strcmp(input, "add") == 0){
            char type;
            int quantity;
            scanf("%c %d", &type, &quantity);
            add(type, quantity);
        }
        else if(strcmp(input, "get") == 0){
            char aggr[11];
            char type;
            int quantity;
            scanf(" %s %c %d", aggr, &type, &quantity);
            get(aggr, type, quantity);
        }
    }
}

int start(int udp_socket, struct sockaddr_in ds_addr, int porta){
    int ret;
    int pid;
    unsigned int n1_port, n2_port;
    unsigned long n1_ip, n2_ip;
    char* buffer;
    fd_set master;
    struct timeval timeout;
    unsigned int size_addr = sizeof(ds_addr);
    struct sockaddr_in neighbor[2];
    char ip_pres[100];
    inet_ntop(AF_INET, &ds_addr.sin_addr.s_addr, ip_pres, INET_ADDRSTRLEN);
        do{
        timeout.tv_sec = 3;
        timeout.tv_usec = 0;
        FD_ZERO(&master);
        FD_SET(udp_socket, &master);
        buffer = malloc(MSG_STD_LEN);
        sprintf(buffer,"%d", porta);
        ret = sendto(udp_socket, buffer, MSG_STD_LEN, 0, (struct sockaddr*) &ds_addr, size_addr);
        if(ret < 0) {
            perror("Errore di comunicazione col DS: ");
            continue;
        }
        printf("-SISTEMA- inviato il messaggio di boot %s al DS sulla porta %u\n", buffer, ntohs(ds_addr.sin_port));
        free(buffer);

        ret = select(udp_socket + 1, &master, NULL, NULL, &timeout);
        if(ret == 1) {
            buffer = (char*)malloc(34);
            memset(buffer, 0, 34);
            memset(&ds_addr, 0, size_addr);
            ret = recvfrom(udp_socket, buffer, 34, 0, (struct sockaddr *) &ds_addr, &size_addr);
            if (ret < 0) {
                perror("Errore di comunicazione col DS: ");
                continue;
            }
            sscanf(buffer, "%lu:%u %lu:%u", &n1_ip, &n1_port, &n2_ip, &n2_port);
            printf("-SISTEMA- Ricevuto il messagio %lu:%u %lu:%u dal DS sulla porta %u\n", n1_ip, n1_port, n2_ip, n2_port, ntohs(ds_addr.sin_port));

            break;
        }
    }
    while(1);

    neighbor[0].sin_family = AF_INET;
    neighbor[0].sin_port = n1_port;
    neighbor[0].sin_addr.s_addr = n1_ip;

    neighbor[1].sin_family = AF_INET;
    neighbor[1].sin_port = n2_port;
    neighbor[1].sin_addr.s_addr = n2_ip;

    ret = sendto(udp_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&ds_addr, size_addr);
    if (ret < 0)
        perror("Errore di comunicazione col DS: ");
    printf("-SISTEMA- I neighbors assegnati sono i peer %u e %u\n", ntohs(neighbor[0].sin_port), ntohs(neighbor[1].sin_port));
    pid = fork();

    if(pid == 0){
        udp_comm(udp_socket, ds_addr, neighbor);
        exit(0);
    }
    else{
        return pid;
    }
}

void add(char type, int quantity){

}

void get(char* aggr, char type, int quantity){

}