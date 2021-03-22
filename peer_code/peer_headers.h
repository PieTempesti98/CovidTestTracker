#include "../headers.h"

void gui(int udp_socket, int porta);

int start(int udp_socket, struct sockaddr_in ds_addr, int porta);

void add(char type, int quantity);

void get(char* aggr, char type, int quantity);

void udp_comm(int sd, struct sockaddr_in ds_addr, struct sockaddr_in* neighbors);
