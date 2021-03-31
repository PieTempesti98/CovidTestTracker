#include"peer_headers.h"

void entries_initializer(struct entry entries[2]){
    //setto la data odierna
    time_t time1 = time(NULL);
    struct tm hour_check = *localtime(&time1);
    //inizializzo i parametri delle due entries
    entries[0].value = entries[1].value = 0;
    entries[0].type = 'T'; entries[1].type = 'N';
    if(hour_check.tm_hour >= 18)//controllo l'orario: se le 18 son gia' passate allora incremento il giorno
        hour_check.tm_mday ++;
    entries[0].data = entries[1].data = hour_check;
}

void append_entries(struct entry entries[2], int porta){
    FILE *fd = NULL;
    char filename[30]; //lunghezza massima del nome del file
    int t_val = 0, n_val = 0;
    long displacement = 0;

    sprintf(filename, "peer_files/register_%d.txt", porta);

    fd = fopen(filename, "r+");
    if(fd == NULL){
        printf("file non ancora creato: lo creo adesso e inserisco le entries odierne\n");
        fd = fopen(filename, "a");
        fprintf(fd,"%d:%d:%d %c %d\n",entries[0].data.tm_year + 1900, entries[0].data.tm_mon + 1, entries[0].data.tm_mday,entries[0].type,entries[0].value);
        fprintf(fd,"%d:%d:%d %c %d\n",entries[1].data.tm_year + 1900, entries[1].data.tm_mon + 1, entries[1].data.tm_mday,entries[1].type,entries[1].value);
        fclose(fd);
        return;
    }
    fseek(fd, 0, SEEK_SET);
    while(!feof(fd)){ //scansiono il file per vedere se trovo entries relative alla giornata da inserire
        int year, month, day;
        char t;
        fscanf(fd,"%d:%d:%d %c %d\n",&year,&month,&day,&t,&t_val);
        //se ho gia' dei dati per il giorno che sto per caricare me li salvo e li sovrascrivo sommandoci i nuovi dati
        if(year ==entries[0].data.tm_year + 1900 && month == entries[0].data.tm_mon + 1 && entries[0].data.tm_mday == day) {
            //mi calcolo quanti caratteri devo sovrascrivere
            char value_disp[7];
            sprintf(value_disp, "%d", t_val);
            displacement -= strlen(value_disp);
            memset(value_disp, 0, 7);
            sprintf(value_disp, "%d", year);
            displacement -= 2 * strlen(value_disp);
            memset(value_disp, 0, 7);
            sprintf(value_disp, "%d", month);
            displacement -= 2 * strlen(value_disp);
            memset(value_disp, 0, 7);
            sprintf(value_disp, "%d", day);
            displacement -= 2 * (strlen(value_disp) + 6);
            fscanf(fd,"%d:%d:%d %c %d\n",&year,&month,&day,&t,&n_val);
            memset(value_disp, 0, 7);
            sprintf(value_disp, "%d", n_val);
            displacement -= strlen(value_disp);
            break;
        }
    }
    fseek(fd, displacement, SEEK_END);
    entries[0].value += t_val;
    entries[1].value += n_val;
    fprintf(fd,"%d:%d:%d %c %d\n",entries[0].data.tm_year + 1900, entries[0].data.tm_mon + 1, entries[0].data.tm_mday,entries[0].type,entries[0].value);
    fprintf(fd,"%d:%d:%d %c %d\n",entries[1].data.tm_year + 1900, entries[1].data.tm_mon + 1, entries[1].data.tm_mday,entries[1].type,entries[1].value);
    printf("-SISTEMA- Le entries di oggi sono state salvate nel register %s\n", filename);
    entries_initializer(entries);
    fclose(fd);
}

int look_for_entries(char type, struct tm data1, struct tm data2, int peer){
    FILE *fd;
    time_t tm = time(NULL);
    char entry[20], filename[30], t;//entry: string di appoggio per il parsing, t: tipo della entry in analisi
    struct tm date = *localtime(&tm);
    int v, d, m, y;//v: campo value del giorno, d/m/y: variabili di appoggio per il parsing della data

    date.tm_hour = date.tm_min = date.tm_sec = 0;
    //apro il register del peer
    sprintf(filename, "peer_files/register_%d.txt", peer);
    fd = fopen(filename, "r");

    if(fd == NULL) // Il file non esiste, quindi non avro' entries da inviare
        return 0;

    while(fgets(entry, 20, fd) != NULL){
        sscanf(entry, "%d:%d:%d %c %d", &y, &m, &d, &t, &v);//faccio il parsing della entry
        date.tm_mday = d;
        date.tm_mon = m -1;
        date.tm_year = y - 1900;
        //c'e' qlmeno una data nell'intervallo analizzato: chiudo lo stream e ritorno 1
        if(difftime(mktime(&data1), mktime(&date)) <= 0 && difftime(mktime(&data2), mktime(&date)) >= 0 && t == type) {
            fclose(fd);
            return 1;
        }
    }
    //non ci sono date nell'intervallo analizzato: chiudo lo stream e ritorno 0
    fclose(fd);
    return 0;
}

int date_offset(struct tm data1, struct tm data2){
    double f_offset = difftime(mktime(&data1), mktime(&data2))/(60*60*24);
    int offset = (int)difftime(mktime(&data1), mktime(&data2))/(60*60*24);

    if((double)offset - f_offset < -0.5)
        offset ++;
    if((double)offset - f_offset > 0.5)
        offset --;
    return offset;
}

char* extract_entries(struct tm data1, struct tm data2, char type, int peer){
    FILE *fd;
    char* str, filename[30], entry[20];
    int offset = date_offset(data2, data1) + 1; //offset tra le due date: numero di iterazioni
    struct tm confronto = data1;
    struct tm date = data1;
    str = (char*)malloc(sizeof(char)*offset*7); //stringa con tutti i valori da inviare
    sprintf(filename, "peer_files/register_%d.txt", peer);
    memset(str, 0, offset * 7);
    fd = fopen(filename, "r");
    if(fd == NULL){//non c'e' il file: ritorno tutti 0
        for(int i = 0; i < offset; i++)
            strcat(str, "0 ");
        printf("%s\n",str);
        return str;
    }
    for(int i = 0; i < offset; i++) {
        int found = 0;
        char str_v[8];
        fseek(fd, 0, SEEK_SET); //riparto dall'inizio del file
        while (fgets(entry, 20, fd) != NULL) {//lo scansiono tutto
            int v, d, m, y;
            char t;
            sscanf(entry, "%d:%d:%d %c %d", &y, &m, &d, &t, &v);//faccio il parsing della entry
           // printf("%s %d:%d:%d\n", entry, confronto.tm_mday, confronto.tm_mon + 1, confronto.tm_year + 1900);
            date.tm_mday = d;
            date.tm_mon = m - 1;
            date.tm_year = y - 1900;
            if (date_offset(date, confronto) == 0) {
                sprintf(str_v, "%d ", v);
                found = 1;
                break;
            }
        }
        if(found == 0)
            sprintf(str_v, "0 ");
        strcat(str, str_v);

        confronto.tm_mday ++;
    }
    return str;
}