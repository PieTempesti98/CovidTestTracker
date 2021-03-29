#include "peer_headers.h"

void add(char type, int quantity, struct entry entries[2]){
    if(type == 'T') //incremento i nuovi tamponi
        entries[0].value += quantity;
    else if(type == 'N') //incremento i nuovi casi
        entries[1].value += quantity;
    else {//passato un valore type scorretto
        printf("Tipo non riconosciuto\n");
        return;
    }
    printf("Dati aggiornati. Nuovi casi: %d, tamponi eseguiti: %d.\n", entries[1].value, entries[0].value);
}

void get(char aggr, char type, char* period, struct aggr** aggregates, struct sockaddr_in neighbors[2], int porta){
    char* elab;
    time_t t = time(NULL);
    struct tm data1 = *localtime(&t), data2 = *localtime(&t);
    struct aggr* p;
    int diff_offset, offset1, offset2;
    float f_offset1, f_offset2;

    if((aggr == 'T' || aggr == 'V') && (type == 'T' || type == 'N')) {
        elab = strtok(period, "-");
        if (strcmp(elab, "*") == 0) {//Imposto il lower bound al 1/01/1900
            data1.tm_year = 0;
            data1.tm_mon = 0;
            data1.tm_mday = 1;
        } else {//inserisco la data specificata come data1
            int m, a;
            sscanf(elab, " %d:%d:%d", &data1.tm_mday, &m, &a);
            data1.tm_mon = m - 1;
            data1.tm_year = a - 1900;
        }
        elab = strtok(NULL, " \n");
        if (strcmp(elab, "*") == 0) {
            if (data2.tm_hour < 18)//Se non sono ancora scattate le 18 l'upper bound e' ieri, altrimenti oggi
                data2.tm_mday--;
        } else {
            int m, a;
            sscanf(elab, " %d:%d:%d", &data2.tm_mday, &m, &a);
            data2.tm_mon = m - 1;
            data2.tm_year = a - 1900;
        }
        //setto i valori orari a 0 per confronti piu' precisi
        data1.tm_hour = data1.tm_min = data1.tm_sec = data2.tm_hour = data2.tm_min = data2.tm_sec = 0;
        data1.tm_mday ++; data2.tm_mday ++;
        //calcolo la differenza in giorni tra le due date indicate (dati da prelevare dall'array di dati aggregati
        diff_offset = (int)difftime(mktime(&data2), mktime(&data1))/(60 * 60 * 24);
        if(aggr == 'T')//le variazioni sono una in meno dei giorni di offset
            diff_offset ++;
        //cerco se ho gia' il dato calcolato
        p = *aggregates;
        while(p != NULL){
            f_offset1 = difftime(mktime(&data1), mktime(&p->d1))/(60*60*24);
            f_offset2 = difftime(mktime(&data2), mktime(&p->d2))/(60 * 60 * 24);
            offset1 = (int)difftime(mktime(&data1), mktime(&p->d1))/(60*60*24);
            offset2 = (int)difftime(mktime(&data2), mktime(&p->d2))/(60 * 60 * 24);
            if((float)offset1 - f_offset1 < -0.5)
                offset1 ++;
            if((float)offset1 - f_offset1 > 0.5)
                offset1 --;
            if((float)offset2 - f_offset2 < -0.5)
                offset2 ++;
            if((float)offset2 - f_offset2 > 0.5)
                offset2 --;
            if(p->type == type && p->aggr_type == aggr &&
            offset1 >= 0 && offset2 <= 0){
                break;
            }
            p = p->next;
        }
        if(p != NULL){//p sta puntando alla struttra di aggr che contiene i dati richiesti
            //calcolo la differenza in giorni dalla data memorizzata nella strttura a quella indicata
            int start_offset = offset1;
            int sum = 0;
            int caratteri_scorsi = 0;
            if(aggr == 'V')
                printf("Di seguito la lista delle variazioni giornaliere di valori di tipo %c a partire dalla"
                       " meno recente:\n", type);
            for(int i = 0; i < (start_offset + diff_offset); i++) {
               int val;
               elab = strtok(p->value + caratteri_scorsi, " ");
               caratteri_scorsi += strlen(elab) + 1;
               if(i >= start_offset) {//scarto tutti i valori precedenti a quelli che mi interessano
                   sscanf(elab, "%d", &val);
                   if (aggr == 'T')
                       sum += val; //sommo tutti i valori che mi servono
                   else
                       printf("%d\n", val); //stampo i valori
               }
            }
            if(aggr == 'T')
                printf("Totale dei dati di tipo %c: %d\n", type, sum);
        }else{//Non ho trovato l'aggregato gia' calcolato: me lo devo calcolare
            //Invio un messaggio request data al primo neighbor
            char* buffer;
            int len, sd, ret;
            uint16_t nlen;

            //grandezza massima del messaggio in formato REQD [PEER] [A] [T] [data1-data2]
            buffer = (char*)malloc(sizeof(char)* 37);

            //formatto il messggio da inviare e la lunghezza da comunicare
            sprintf(buffer, "REQD %d %c %c %d:%d:%d-%d:%d:%d", porta, aggr, type, data1.tm_mday, data1.tm_mon + 1, data1.tm_year + 1900, data2.tm_mday, data2.tm_mon + 1, data2.tm_year + 1900);
            len = strlen(buffer) + 1;
            nlen = htons(len);

            //creo e connetto il socket per contattare il primo neighbor
            sd = socket(AF_INET, SOCK_STREAM, 0);

            ret = connect(sd, (struct sockaddr*)&neighbors[0], sizeof(struct sockaddr_in));
            if(ret != 0)
                perror("Errore di connessione: ");

            // invio lunghezza e messaggio al primo neighbor
            ret = send(sd, (void*) &nlen, sizeof(uint16_t), 0);
            if(ret < 0)
                perror("Errore in trasmissione TCP: ");
            ret = send(sd, (void*) buffer, len, 0);
            if(ret < 0)
                perror("Errore in trasmissione TCP: ");
            printf("-SISTEMA- inviato il messaggio %s al peer %d\n", buffer, ntohs(neighbors[0].sin_port));

            //ricevo la risposta (prima la lunghezza e poi il messaggio effettivo
            ret = recv(sd, (void*)&nlen, sizeof(uint16_t), 0);
            if(ret < 0)
                perror("Errore in ricezione TCP: ");
            len = ntohs(nlen);
            free(buffer);
            buffer = (char*)malloc(sizeof(char)* len);
            memset(buffer, 0, len);
            ret = recv(sd, (void*)buffer, len, 0);
            if(ret < 0)
                perror("Errore in ricezione TCP: ");
            printf("-SISTEMA- Ricevuto il messaggio %s dal peer %d\n", buffer, ntohs(neighbors[0].sin_port));
            elab = strtok(buffer, " ");
            if(strncmp(elab, "REPD", 4) != 0){
                //non ho ricevuto una comunicazione RCVD: ritorno con un errore di sistema
                printf("-ERRORE- Il peer non ha inviato la comunicazione richiesta.\n");
                return;
            }
            elab = strtok(NULL," ");
            if(strncmp(elab, "Y", 1) == 0){//Il neighbor ha i dati aggregati richiesti, li salvo e li elaboro
                struct aggr* new_aggr = (struct aggr*)malloc(sizeof(struct aggr));
                int sum = 0, caratteri_scorsi = 0;
                //inizializzo la nuova struttura dati e la mettp in testa alla lista
                new_aggr->type = type;
                new_aggr->aggr_type = aggr;
                new_aggr->d1 = data1;
                new_aggr->d2 = data2;
                new_aggr->value = (char*)malloc(sizeof(char)*(len - 7));
                new_aggr->next = *aggregates;
                *aggregates = new_aggr;

                strcpy(new_aggr->value, buffer + 7);
                if(aggr == 'V')
                    printf("Di seguito la lista delle variazioni giornaliere di valori di tipo %c a partire dalla"
                           " meno recente:\n", type);
                for(int i = 0; i < diff_offset; i++){
                    int value;
                    elab = strtok(new_aggr->value + caratteri_scorsi, " ");
                    caratteri_scorsi += strlen(elab) + 1;
                    sscanf(elab, "%d", &value);
                    if(aggr == 'T')
                        sum += value;
                    else
                        printf("%d\n", value);
                }
                if(aggr == 'T')
                    printf("Totale dei dati di tipo %c: %d\n", type, sum);
                close(sd);
                free(buffer);
                return;
            }
            //Il primo neighbor non aveva i dati richiesti, contatto il secondo
            //grandezza massima del messaggio in formato REQD [PEER] [A] [T] [data1-data2]
            buffer = (char*)malloc(sizeof(char)* 37);

            //formatto il messggio da inviare e la lunghezza da comunicare
            sprintf(buffer, "REQD %d %c %c %d:%d:%d-%d:%d:%d", porta, aggr, type, data1.tm_mday, data1.tm_mon + 1, data1.tm_year + 1900, data2.tm_mday, data2.tm_mon + 1, data2.tm_year + 1900);
            len = strlen(buffer) + 1;
            nlen = htons(len);

            //creo e connetto il socket per contattare il secondo neighbor
            sd = socket(AF_INET, SOCK_STREAM, 0);

            ret = connect(sd, (struct sockaddr*)&neighbors[1], sizeof(struct sockaddr_in));
            if(ret != 0)
                perror("Errore di connessione: ");

            // invio lunghezza e messaggio al primo neighbor
            ret = send(sd, (void*) &nlen, sizeof(uint16_t), 0);
            if(ret < 0)
                perror("Errore in trasmissione TCP: ");
            ret = send(sd, (void*) buffer, len, 0);
            if(ret < 0)
                perror("Errore in trasmissione TCP: ");
            printf("-SISTEMA- inviato il messaggio %s al peer %d\n", buffer, ntohs(neighbors[1].sin_port));

            //ricevo la risposta (prima la lunghezza e poi il messaggio effettivo
            ret = recv(sd, (void*)&nlen, sizeof(uint16_t), 0);
            if(ret < 0)
                perror("Errore in ricezione TCP: ");
            len = ntohs(nlen);
            free(buffer);
            buffer = (char*)malloc(sizeof(char)* len);
            ret = recv(sd, (void*)buffer, len, 0);
            if(ret < 0)
                perror("Errore in ricezione TCP: ");
            elab = strtok(buffer, " ");
            if(strncmp(elab, "REPD", 4) != 0){
                //non ho ricevuto una comunicazione RCVD: ritorno con un errore di sistema
                printf("-ERRORE- Il peer non ha inviato la comunicazione richiesta.\n");
                return;
            }
            elab = strtok(NULL," ");
            if(strncmp(elab, "Y", 1) == 0){//Il neighbor ha i dati aggregati richiesti, li salvo e li elaboro
                struct aggr* new_aggr = (struct aggr*)malloc(sizeof(struct aggr));
                int sum = 0, caratteri_scorsi = 0;
                elab = strtok(NULL, "\0");
                printf("%ld\n", strlen(elab));
                //inizializzo la nuova struttura dati e la mettp in testa alla lista
                new_aggr->type = type;
                new_aggr->aggr_type = aggr;
                new_aggr->d1 = data1;
                new_aggr->d2 = data2;
                new_aggr->value = (char*)malloc(sizeof(char)*(len - 7));
                new_aggr->next = *aggregates;
                *aggregates = new_aggr;

                strcpy(new_aggr->value, buffer + 7);
                if(aggr == 'V')
                    printf("Di seguito la lista delle variazioni giornaliere di valori di tipo %c a partire dalla"
                           " meno recente:\n", type);
                for(int i = 0; i < diff_offset; i++){
                    int value;
                    elab = strtok(new_aggr->value + caratteri_scorsi, " ");
                    caratteri_scorsi += strlen(elab) + 1;
                    sscanf(elab, "%d", &value);
                    if(aggr == 'T')
                        sum += value;
                    else
                        printf("%d\n", value);
                }
                if(aggr == 'T')
                    printf("Totale dei dati di tipo %c: %d\n", type, sum);
                close(sd);
                free(buffer);
                return;
            }
            //Nessuno dei miei neighbors ha i dati richiesti: procedo a fare il flooding
        }
    }else
        printf("I parametri inseriti non sono corretti\n");

}