#include "../headers.h"

//peer_udp_conn.c
void start(int udp_socket, struct sockaddr_in ds_addr, int porta, struct sockaddr_in neighbors[2]);
void stop(int sd, struct sockaddr_in ds_addr);
int udp_comm(int sd, struct sockaddr_in ds_addr, struct sockaddr_in neighbors[2]);

//peer_gui_routines.c
void add(char type, int quantity);
void get(char* aggr, char type, int quantity);


