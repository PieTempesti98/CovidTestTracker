#include"peer_code/peer_headers.h"

int main(int argc, char* argv[]){
    int porta = atoi(argv[1]); //porta identificativa del peer
    int udp_socket,conn_socket, data_socket, ret; //udp_socket: socket utilizzato per le comunicazioni UDP, ret: int usato per i check
    struct sockaddr_in my_addr;//indirizzo del peer
    struct sockaddr_in neighbors[2]; //indirizzo dei neighbors
    char input[50]; //buffer di input dallo stdin
    uint8_t esc;//controllo dello stato del DS (esc = o DS chiuso)
    fd_set master; //fd_set contenente tutti i socket in attesa di messaggi (compreso stdin)
    struct sockaddr_in* ds_addr = NULL; //puntatore all'indirizzo del DS
    struct entry today_entries[2]; //Array con le entry del giorno (0 per T, 1 per N)
    struct aggr* aggr_list; //Puntatore alla lista dei valori aggregati gia' calcolati


    printf("---------- COVID 19 PEER %u STARTED -----------\n", porta);
    //attivazione dei socket e collegamento con la porta del peer
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    conn_socket = socket(AF_INET, SOCK_STREAM, 0);
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(porta);
    inet_pton(AF_INET, "127.0.0.1", &my_addr.sin_addr);

    ret = bind(udp_socket, (struct sockaddr*)&my_addr, sizeof(my_addr));
    if(ret == -1) {
        perror("Bind error: ");
        exit(1);
    }
    ret = bind(conn_socket, (struct sockaddr*)&my_addr, sizeof(my_addr));
    if(ret == -1) {
        perror("Bind error: ");
        exit(1);
    }
    ret = listen(conn_socket, 10);
    if(ret == -1) {
        perror("Listen error: ");
        exit(1);
    }
    //Creazione delle entry del giorno, inizializzate alla data di oggi e con value 0
    entries_initializer(today_entries);

    //all'inizio non ho valori gggregati calcolati: il puntatore punta a NULL
    aggr_list = NULL;

    while(1){
        int max_socket;
        printf("Digita un comando:\n");
        printf("1. Start <indirizzo del DS> <porta del DS> --> collega il peer alla rete\n");
        printf("2. Add <tipo> <quantita'> --> Aggiungi occorrenze dell'evento indicato\n");
        printf("3. Get <aggr> <tipo> <periodo> --> Visualizza un valore aggregato\n");
        printf("4. Stop --> Disconnetti il peer e spengilo\n\n");
        FD_ZERO(&master);
        FD_SET(udp_socket, &master);
        FD_SET(STDIN_FILENO, &master);
        FD_SET(conn_socket, &master);
        if(udp_socket > conn_socket)
            max_socket = udp_socket;
        else
            max_socket = conn_socket;
        ret = select(max_socket + 1, &master, NULL, NULL, NULL);
        if(ret < 0)
            perror("Errore in attesa: ");
        if(FD_ISSET(conn_socket, &master)){
            //ci sono richieste di collegamento da (almeno) un socket TCP
            struct sockaddr_in* tcp_addr;
            unsigned int addr_size = sizeof(struct sockaddr_in);
            tcp_addr = (struct sockaddr_in*)malloc(addr_size);
            data_socket = accept(conn_socket,(struct sockaddr*)tcp_addr, &addr_size);
            tcp_conn(data_socket, *tcp_addr, today_entries, &aggr_list, porta, neighbors);
        }
        if(FD_ISSET(udp_socket, &master)) {
            //Il DS ha inviato comunicazioni
            esc = udp_comm(udp_socket, *ds_addr, neighbors);
            if(esc == 0) {
                //DS chiuso
                append_entries(today_entries, porta);
                break;
            }
            else if(esc == 2)
                append_entries(today_entries,porta);
            else
                printf("-SISTEMA- I nuovi neighbors sono %u e %u\n", ntohs(neighbors[0].sin_port), ntohs(neighbors[1].sin_port));
        }
        //Nuovi caratteri nello standard input
        if(FD_ISSET(STDIN_FILENO, &master)) {
            char* elab; //stringa da confrontare per preparare i parametri
            fgets(input, 50, stdin);
            elab = strtok(input, " \n");
            if(strcmp(elab, "stop") == 0) {
                if(ds_addr == NULL)
                    break;
                send_entries(porta, neighbors[0], today_entries);
                stop(udp_socket, *ds_addr);
                break;
            }
            else if(strcmp(elab, "start") == 0){
                int ds_port;
                char ds_ip[16];
                //carico indirizzo e porta del DS e avvio le procedure di start
                elab = strtok(NULL, " ");
                sscanf(elab,"%s", ds_ip);
                elab = strtok(NULL, " \n");
                sscanf(elab,"%d", &ds_port);
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
                elab = strtok(NULL, " ");
                sscanf(elab,"%c", &type);
                elab = strtok(NULL, " \n");
                sscanf(elab,"%d", &quantity);
                add(type, quantity, today_entries);
            }
            else if(strcmp(input, "get") == 0){
                char* period;
                char aggr;
                char type;
                elab = strtok(NULL, " ");
                sscanf(elab,"%c", &aggr);
                elab = strtok(NULL, " \n");
                sscanf(elab,"%c", &type);
                elab = strtok(NULL, " \n");
                if(elab == NULL){
                    period = (char*)malloc(sizeof(char)*4); // formato: *:*\0
                    strcpy(period, "*:*");
                }else{
                    period = (char*)malloc(strlen(elab) + 1);
                    strcpy(period, elab);
                }
                get(aggr, type, period, &aggr_list, neighbors, porta);
            }
        }
    }
    exit(0);
}