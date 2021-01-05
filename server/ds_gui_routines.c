#include "ds_headers.h"

void showpeers(){
    printf("metodo showpeers non ancora implementato\n");
}

void showhelp(){
    printf("metodo help non ancora implementato\n");
};
void showneighbor(int peer){
    printf("metodo showneighbor per il peer %d non ancora implementato\n", peer);
};

int gui(){
    char input[20];

    printf("Digita un comando:\n\n"
           "1. help --> Mostra i dettagli dei comandi\n"
           "2. showpeers --> Mostra un elenco dei peer connessi\n"
           "3. showneighbor <peer> --> Mostra i neighbor del peer specificato\n"
           "4. esc --> Chiudi il DS\n\n"
    );
    scanf("%s", input);
    if(strcmp(input, "esc") == 0)
        return 0;
    if(strcmp(input, "showpeers") == 0)
        showpeers();
    else if(strcmp(input, "help") == 0)
        showhelp();
    else if(strcmp(input, "showneighbor") == 0){
        int peer;
        scanf("%d",&peer);
        showneighbor(peer);
    }

    return 1;
}
