#include"peer_code/peer_headers.h"

int main(int argc, char* argv[]){
    int porta = atoi(argv[1]);
    int udp_socket, ret;
    struct sockaddr_in my_addr;
    struct sockaddr_in neighbors[2];
    char input[50];
    uint8_t esc = 1;
    fd_set master;
    struct sockaddr_in* ds_addr = NULL;

    printf("---------- COVID 19 PEER %u STARTED -----------\n", porta);
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(porta);
    inet_pton(AF_INET, "127.0.0.1", &my_addr.sin_addr);

    ret = bind(udp_socket, (struct sockaddr*)&my_addr, sizeof(my_addr));
    if(ret == -1) {
        perror("Bind error: ");
        exit(1);
    }
    while(1){
        printf("Digita un comando:\n");
        printf("1. Start <indirizzo del DS> <porta del DS> --> collega il peer alla rete\n");
        printf("2. Add <tipo> <quantita'> --> Aggiungi occorrenze dell'evento indicato\n");
        printf("3. Get <aggr> <tipo> <periodo> --> Visualizza un valore aggregato\n");
        printf("4. Stop --> Disconnetti il peer e spengilo\n\n");
        FD_ZERO(&master);
        FD_SET(udp_socket, &master);
        FD_SET(STDIN_FILENO, &master);
        ret = select(udp_socket + 1, &master, NULL, NULL, NULL);
        if(ret < 0)
            perror("Errore in attesa: ");
        if(FD_ISSET(udp_socket, &master)) {
            //Il DS ha inviato comunicazioni
            esc = udp_comm(udp_socket, *ds_addr, neighbors);
            if(esc == 0)
                //DS chiuso
                break;
            else
                printf("-SISTEMA- I nuovi neighbors sono %u e %u\n", ntohs(neighbors[0].sin_port), ntohs(neighbors[1].sin_port));
        }
        //Nuovi caratteri nello standard input
        if(FD_ISSET(STDIN_FILENO, &master)) {
            scanf("%s", input);
            if(strcmp(input, "stop") == 0) {
                if(ds_addr == NULL)
                    break;
                stop(udp_socket, *ds_addr);
                break;
            }
            else if(strcmp(input, "start") == 0){
                int ds_port;
                char ds_ip[16];
                //carico indirizzo e porta del DS e avvio le procedure di start
                scanf("%s", ds_ip);
                scanf("%d", &ds_port);
                if(ds_addr == NULL){
                    ds_addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));
                    ds_addr->sin_family = AF_INET;
                    ds_addr->sin_port = htons(ds_port);
                    inet_pton(AF_INET, ds_ip, &ds_addr->sin_addr.s_addr);
                    start(udp_socket, *ds_addr, porta, neighbors);
                    printf("-SISTEMA- I neighbors assegnati sono i peer %u e %u\n", ntohs(neighbors[0].sin_port), ntohs(neighbors[1].sin_port));
                }else
                    //start quando la connessione e' gia' stata stabilita
                    printf("-SISTEMA- Il peer e' gia' connesso al DS\n");
            }else if(strcmp(input, "add") == 0){
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
    exit(0);
}