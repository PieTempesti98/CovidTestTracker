#include "server/ds_headers.h"

int main(int argc, char* argv[]){
    int porta = atoi(argv[1]);
    struct peer* list = NULL;

    printf("******************* DS COVID STARTED *******************\n");

    //Lancio dell'interfaccia di interazione
    while(gui(list)){}
    exit(1);
}
