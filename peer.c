#include"peer_code/peer_headers.h"

int main(int argc, char* argv[]){
    int porta = atoi(argv[1]);
    int udp_socket, ret;
    struct sockaddr_in my_addr;

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
    gui(udp_socket, porta);
}