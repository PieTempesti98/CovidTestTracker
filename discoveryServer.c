#include "server/ds_headers.h"

int main(int argc, char* argv[]){
    int udp_socket, ret, pid;
    int porta = atoi(argv[1]), tot_peers = 0;
    struct peer* list = NULL;
    struct sockaddr_in ds_addr;

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

    pid = fork();

    if(pid == 0){
        ds_boot(udp_socket, list, &tot_peers);

    }
    else {
        //Lancio dell'interfaccia di interazione
        while (gui(list, &tot_peers)) {}
        /* Quando esco dal while e' stato invocato il comando ESC: Termino il processo figlio (comunicazioni UDP),
         * invio i segnali di quit e chiudo il socket */
        kill(pid, 2);
        quit(udp_socket, list);
        close(udp_socket);

    }
    exit(0);
}
