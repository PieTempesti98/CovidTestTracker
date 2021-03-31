#include"peer_headers.h"

void tcp_conn(int data_socket, struct sockaddr_in tcp_addr, struct entry entries[2], struct aggr** aggregates, int porta, struct sockaddr_in neighbors[2]){
    char* buffer;
    int ret, buf_size;
    uint16_t msg_size;
    char* elab;
    char* new_msg;
    ret = recv(data_socket, (void*) &msg_size,sizeof(uint16_t), 0 );
    if(ret < 0){
        perror("Errore in trasmissione TCP: ");
        return;
    }
    buf_size = ntohs(msg_size);

    buffer = (char*)malloc(sizeof(char)*buf_size);

    ret = recv(data_socket, (void*) buffer,buf_size, 0 );
    if(ret < 0){
        perror("Errore in trasmissione TCP: ");
        free(buffer);
        return;
    }
    printf("Ricevuto il messaggio %s\n", buffer);
    new_msg = (char*)malloc(sizeof(char)*(strlen(buffer) + 9));
    strcpy(new_msg, buffer);
    elab = strtok(buffer, " "); //guardo il tipo di messaggio che ho ricevuto

    //messaggio di stop del peer che ha il peer attuale come next
    if(strcmp(elab, "STOP") == 0){
        int peer, t, n;
        //formato: STOP [peer] [n^ T] [n^ N]: faccio il parsing dei valori e li salvo nelle mie entries
        elab = strtok(NULL, " ");
        sscanf(elab, "%d", &peer);
        elab = strtok(NULL, " ");
        sscanf(elab, "%d", &t);
        elab = strtok(NULL, " ");
        sscanf(elab, "%d", &n);
        entries[0].value += t;
        entries[1].value += n;
        printf("-SISTEMA- Ricevuto messaggio di stop dal peer %d: aggiunti %d tamponi e %d nuovi casi\n", peer, t, n);
        close(data_socket);

    }
    //REQUEST_DATA da un altro peer
    //formato: REQD [peer] [aggr_type] [type] [data1-data2]
    if(strncmp(elab, "REQD", 4) == 0){
        int peer, offset1, offset2;
        char a, t;
        char* date_str;
        time_t tm = time(NULL);
        struct tm data1 = *localtime(&tm), data2 = *localtime(&tm);
        struct aggr* p;
        int m, an;//supporto per il parsing delle date

        //setto i valori orari a 0 per confronti piu' precisi
        data1.tm_hour = data1.tm_min = data1.tm_sec = data2.tm_hour = data2.tm_min = data2.tm_sec = 0;

        //faccio il parsing delle informazioni del messaggio
        elab = strtok(NULL, " ");
        sscanf(elab, "%d", &peer);
        elab = strtok(NULL, " ");
        sscanf(elab, "%c", &a);
        elab = strtok(NULL, " ");
        sscanf(elab, "%c", &t);
        elab = strtok(NULL, " \n");
        date_str = (char*)malloc(sizeof(char)*(strlen(elab) + 1));
        strcpy(date_str, elab);
        elab = strtok(date_str, "-");

        //parsing delle date
        sscanf(elab, "%d:%d:%d", &data1.tm_mday, &m, &an);
        data1.tm_mon = m - 1;
        data1.tm_year = an - 1900;


        elab = strtok(NULL, " \n");
        sscanf(elab, "%d:%d:%d", &data2.tm_mday, &m, &an);
        data2.tm_mon = m - 1;
        data2.tm_year = an - 1900;

        //guardo se ho l'informazione richiesta
        p = *aggregates;
        while(p != NULL){
            offset1 = date_offset(data1, p->d1);
            offset2 = date_offset(data2, p->d2);
            if(p->type == t && p->aggr_type == a &&
               offset1 >= 0 && offset2 <= 0){
                break;
            }
            p = p->next;
        }
        if(p == NULL){ //Non ho trovato i dati richiesti: lo comunico al peer
            char* msg = "REPD N";//reply data: non ho i dati richiesti
            int len = strlen(msg) + 1;
            uint16_t nlen = htons(len);
            ret = send(data_socket, (void*)&nlen, sizeof(uint16_t), 0);
            if(ret < 0){
                perror("Errore in trasmissione TCP: ");
                free(buffer);
                free(new_msg);
                free(date_str);
                return;
            }
            ret = send(data_socket, (void*)msg, len, 0);
                if(ret < 0){
                    perror("Errore in trasmissione TCP: ");
                    free(buffer);
                    free(new_msg);
                    free(date_str);
                    return;
                }
            printf("-SISTEMA- Inviato il messaggio %s al peer %d\n", msg, peer);
        }else{//ho l'informazione richiesta: invio il vettore di dati richiesto
            //Calcolo l'offset tra il primo dato memorizzato nella struttura dati e quello che voglio calcolare
            int start_offset = offset1;
            //calcolo la differenza in giorni tra le due date indicate (dati da prelevare dall'array di dati aggregati)
            int diff_offset = date_offset(data2, data1);

            //Tiene il conto di quanti caratteri ho scorso nella stringa dei valori della struttura aggr
            int caratteri_scorsi = 0;

            //se ho dei valori aggregati avro' un valore in meno rispetto alla differenza delle date
            if(a == 'T')
                diff_offset ++;

            //setto la dimensione del buffer: 7 caratteri di intestazione + i caratteri per salvare gli interi
            //(in media occuperanno 4 cifre) + il carattere di spazio per ogni intero (e il \0 per la terminazione)
            buf_size = 7 + (diff_offset * (sizeof(int) + 1));
            free(buffer);
            buffer = (char*)malloc(sizeof(char) * buf_size);
            strcpy(buffer, "REPD Y ");
            for(int i = 0; i < (start_offset + diff_offset); i++){
                //scansiono la stringa dei valori
                char int_cat[8];//6 cifre + " " + \0
                elab = strtok(p->value + caratteri_scorsi, " ");
                caratteri_scorsi += strlen(elab) + 1;
                if(i >= start_offset) {
                    //Copio solo i valori che mi interessano (posizioni tra strat_offset e diff_offset
                    memset(int_cat, 0, 8);
                    strcpy(int_cat, elab);
                    strcat(int_cat, " ");
                    strcat(buffer, int_cat);//inserisco il nuovo valore nel buffer di invio
                }
            }
            //invio prima la lunghezza e poi il esto del messaggio
            int len = strlen(buffer) + 1;
            uint16_t nlen = htons(len);
            ret = send(data_socket, (void*)&nlen, sizeof(uint16_t), 0);
            if(ret < 0){
                perror("Errore in trasmissione TCP: ");
                free(buffer);
                free(new_msg);
                free(date_str);
                return;
            }
            ret = send(data_socket, (void*)buffer, len, 0);
            if(ret < 0){
                perror("Errore in trasmissione TCP: ");
                free(buffer);
                free(new_msg);
                free(date_str);
                return;
            }
            printf("-SISTEMA- Inviato il messaggio %s al peer %d\n", buffer, peer);
        }
        free(date_str);
    }
    if(strncmp(elab, "FFEN", 4) == 0) {
        int peer, d1, m1, y1, d2, m2, y2, pid;
        char t;
        //allungo la stringa del buffer di 8 caratteri
        time_t tm = time(NULL);
        struct tm data1 = *localtime(&tm), data2 = *localtime(&tm);

        strcpy(buffer, new_msg);
        //faccio il parsing del messaggio ricevuto
        sscanf(buffer, "FFEN %d %c %d:%d:%d-%d:%d:%d,", &peer, &t, &d1, &m1, &y1, &d2, &m2, &y2);
        m1--;
        m2--;
        y1 -= 1900;
        y2 -= 1900;
        data1.tm_hour = data1.tm_min = data1.tm_sec = data2.tm_hour = data2.tm_min = data2.tm_sec = 0;
        data1.tm_mday = d1;
        data1.tm_mon = m1;
        data1.tm_year = y1;
        data2.tm_mday = d2;
        data2.tm_mon = m2;
        data2.tm_year = y2;
        pid = fork();
        if(pid == 0){//processo figlio: si occupa di gestire la richiesta
            int result = look_for_entries(t, data1, data2, porta);
            char str_to_add[9];
            sprintf(str_to_add, "%d:", porta);
            if(result == 1)
                strcat(str_to_add, "Y ");
            else
                strcat(str_to_add, "N ");
            strcat(new_msg, str_to_add);
            if(
                 (//Prima risposta inserita ed il mio shortcut e' il richiedente: contnuo
                    (strlen(new_msg) + 1 < 43 && peer == ntohs(neighbors[1].sin_port))
                    || peer != ntohs(neighbors[1].sin_port))//shortcut non e' il richiedente: continuo
                  &&  peer != ntohs(neighbors[0].sin_port)){//next non e' il richiedene: continuo il flooding

                //Il flooding non e' completo: invio a next e aspetto il ritorno del messaggio per farlo tornare indietro
                int sd;
                int len = strlen(new_msg) + 1;
                uint16_t nlen = htons(len);

                sd = socket(AF_INET, SOCK_STREAM, 0);
                ret = connect(sd, (struct sockaddr*)&neighbors[0], sizeof(struct sockaddr_in));
                if(ret < 0){
                    perror("Errore in trasmissione TCP: ");
                    free(buffer);
                    free(new_msg);
                    exit(1);
                }

                ret = send(sd, (void*)&nlen, sizeof(uint16_t), 0);
                if(ret < 0){
                    perror("Errore in trasmissione TCP: ");
                    free(buffer);
                    free(new_msg);
                    exit(1);
                }
                ret = send(sd, (void*) new_msg, len, 0);
                if(ret < 0){
                    perror("Errore in trasmissione TCP: ");
                    free(buffer);
                    free(new_msg);
                    exit(1);
                }
                printf("-SISTEMA- inviato il messaggio %s al peer %d\n", new_msg, ntohs(neighbors[0].sin_port));

                ret = recv(sd, (void*)&nlen, sizeof(uint16_t), 0);
                if(ret < 0){
                    perror("Errore in trasmissione TCP: ");
                    free(buffer);
                    free(new_msg);
                    exit(1);
                }
                len = ntohs(nlen);
                free(new_msg);
                new_msg = (char*)malloc(sizeof(char)* len);
                ret = recv(sd, (void*)new_msg, len, 0);
                if(ret < 0){
                    perror("Errore in trasmissione TCP: ");
                    free(buffer);
                    free(new_msg);
                    exit(1);
                }
                printf("-SISTEMA- Ricevuto il messaggio %s dal peer %d\n", new_msg, ntohs(neighbors[0].sin_port));
                close(sd);
            }
            //Rimando indiero il messaggio (mi e' arrivato il messaggio di ritorno o il flooding si e'concluso)
            int len = strlen(new_msg) + 1;
            uint16_t nlen = htons(len);
            ret = send(data_socket, (void*)&nlen, sizeof(uint16_t), 0);
            if(ret < 0){
                perror("Errore in trasmissione TCP: ");
                free(buffer);
                free(new_msg);
                exit(1);
            }
            ret = send(data_socket, (void*) new_msg, len, 0);
            if(ret < 0){
                perror("Errore in trasmissione TCP: ");
                free(buffer);
                free(new_msg);
                exit(1);
            }

            printf("-SISTEMA- inviato il messaggio %s al peer %d\n", new_msg, peer);
            close(data_socket);
            free(buffer);
            free(new_msg);

            exit(0);
        }
    }
    if(strncmp(elab, "REQE", 4) == 0){//REQUEST_ENTRIES
        int peer, pid;
        char t;
        time_t tm = time(NULL);
        struct tm data1 = *localtime(&tm), data2 = *localtime(&tm);
        int day1, mon1, yr1;
        data1.tm_hour = data1.tm_min = data1.tm_sec = data2.tm_hour = data2.tm_min = data2.tm_sec = 0;
        strcpy(buffer, new_msg);
        //faccio il parsing del messaggio ricevuto
        elab = strtok(NULL, " ");
        sscanf(elab, "%d", &peer);
        elab = strtok(NULL, " ");
        t = *elab;
        elab = strtok(NULL, "-");
        sscanf(elab, "%d:%d:%d",&day1, &mon1, &yr1);
        data1.tm_mday = day1;
        data1.tm_mon = mon1 -1;
        data1.tm_year = yr1 - 1900;
        elab = strtok(NULL, "- \n");
        sscanf(elab, "%d:%d:%d",&day1, &mon1, &yr1);
        data2.tm_mday = day1;
        data2.tm_mon = mon1 -1;
        data2.tm_year = yr1 - 1900;

        printf("%d:%d:%d-%d:%d:%d\n", data1.tm_mday, data1.tm_mon + 1, data1.tm_year + 1900, data2.tm_mday, data2.tm_mon + 1, data2.tm_year + 1900);
        pid = fork();
        if(pid == 0){
            int len;
            uint16_t nlen;
            free(buffer);
            free(new_msg);
            new_msg = extract_entries(data1, data2, t, porta);
            len = strlen(new_msg) + 1;
            nlen = htons(len);
            ret = send(data_socket, (void*)&nlen, sizeof(uint16_t), 0);
            if(ret < 0){
                perror("Errore in comunicazione TCP: ");
                free(new_msg);
                exit(1);
            }
            ret = send(data_socket, (void*)new_msg, len, 0);
            if(ret < 0){
                perror("Errore in comunicazione TCP: ");
                free(new_msg);
                exit(1);
            }
            printf("-SISTEMA- inviato il messaggio %s al peer %d\n", new_msg, peer);
            free(new_msg);
            exit(0);
        }
    }
    close(data_socket);
    free(new_msg);
    free(buffer);
    return;
}

void send_entries(int peer, struct sockaddr_in next_addr, struct entry entries[2]){
    char buffer[50];
    int ret, sd, len;
    uint16_t nlen;
    if(next_addr.sin_port == 0){
        printf("al momento non sono connesso ad alcun peer\n");
        return;
    }
    sd = socket(AF_INET, SOCK_STREAM, 0);

    ret = connect(sd, (struct sockaddr*)&next_addr, sizeof(struct sockaddr_in));
    if(ret != 0)
        perror("Errore di connessione: ");

    sprintf(buffer, "STOP %d %d %d", peer, entries[0].value, entries[1].value);

    len = strlen(buffer) + 1;
    nlen = htons(len);
    ret = send(sd, (void*) &nlen, sizeof(uint16_t), 0);
    if(ret < 0)
        perror("Errore in trasmissione TCP: ");
    ret = send(sd, (void*) buffer, len, 0);
    if(ret < 0)
        perror("Errore in trasmissione TCP: ");
    printf("-SISTEMA- inviato il messaggio %s al peer %d\n", buffer, ntohs(next_addr.sin_port));
    close(sd);
    return;
}



