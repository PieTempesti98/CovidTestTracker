#include "server/ds_headers.h"



int main(int argc, char* argv[]){
    int udp_socket, ret;
    int porta = atoi(argv[1]), tot_peers = 0;
    struct peer* list;
    struct sockaddr_in ds_addr;
    fd_set master;
    int esc = 1;

    list = NULL;
    printf("******************* DS COVID STARTED *******************\n");

    //avvio del socket UDP
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&ds_addr, 0, sizeof(ds_addr));
    ds_addr.sin_family = AF_INET;
    ds_addr.sin_port = htons(porta);
    inet_pton(AF_INET, "127.0.0.1", &ds_addr.sin_addr);

    ret = bind(udp_socket, (struct sockaddr*)&ds_addr, sizeof(ds_addr));
    if(ret == -1) {
        perror("Bind error: ");
        exit(1);
    }
    //Lancio dell'interfaccia di interazione
    while (esc){
        printf("Digita un comando:\n\n"
               "1. help --> Mostra i dettagli dei comandi\n"
               "2. showpeers --> Mostra un elenco dei peer connessi\n"
               "3. showneighbor <peer> --> Mostra i neighbor del peer specificato\n"
               "4. esc --> Chiudi il DS\n\n"
        );
        FD_ZERO(&master);
        FD_SET(udp_socket, &master);
        FD_SET(STDIN_FILENO, &master);
        ret = select(udp_socket + 1, &master, NULL, NULL, NULL);
        if(ret < 0)
            perror("Errore in attesa: ");
        if(FD_ISSET(udp_socket, &master))
            list = ds_boot(udp_socket, list, &tot_peers);
        if(FD_ISSET(STDIN_FILENO, &master))
            esc = gui(list);
    }
    /* Quando esco dal while e' stato invocato il comando ESC: Termino il processo figlio (comunicazioni UDP),
     * invio i segnali di quit e chiudo il socket */
    quit(udp_socket, list);
    close(udp_socket);
    exit(0);
}
