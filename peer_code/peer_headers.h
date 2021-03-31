#include "../headers.h"

struct entry{
    struct tm data;
    char type;
    int value;
};

struct aggr{
    struct tm d1, d2;
    char type;
    char aggr_type;
    char* value;
    struct aggr* next;
};

//peer_udp_conn.c
int start(int udp_socket, struct sockaddr_in ds_addr, int porta, struct sockaddr_in neighbors[2]);
void stop(int sd, struct sockaddr_in ds_addr);
int udp_comm(int sd, struct sockaddr_in ds_addr, struct sockaddr_in neighbors[2]);

//peer_gui_routines.c
void add(char type, int quantity, struct entry entries[2]);
void get(char aggr, char type, char* period, struct aggr** aggregates, struct sockaddr_in neighbors[2], int porta);

//peer_utilities.c
void entries_initializer(struct entry entries[2]);
void append_entries(struct entry entries[2], int porta);
int date_offset(struct tm data1, struct tm data2);
int look_for_entries(char type, struct tm data1, struct tm data2, int peer);
char* extract_entries(struct tm data1, struct tm data2, char type, int peer);

//peer_tcp_conn.c
void tcp_conn(int data_socket, struct sockaddr_in tcp_addr, struct entry entries[2], struct aggr** aggregates, int porta, struct sockaddr_in neighbors[2]);
void send_entries(int peer, struct sockaddr_in next_addr, struct entry entries[2]);
