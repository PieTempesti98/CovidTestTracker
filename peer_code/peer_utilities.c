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
    char data_da_inserire[11]; //lunghezza massima del formato data yyyy:mm:dd\0
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

    sprintf(data_da_inserire, "%4d:%2d:%2d", entries[0].data.tm_year + 1900, entries[0].data.tm_mon + 1, entries[0].data.tm_mday);
    fseek(fd, 0, SEEK_SET);
    while(!feof(fd)){
        int year, month, day;
        char t;
        fscanf(fd,"%d:%d:%d %c %d\n",&year,&month,&day,&t,&t_val);
        //se ho gia' dei dati per il giorno che sto per caricare me li salvo e li sovrascrivo sommandoci i nuovi dati
        if(year ==entries[0].data.tm_year + 1900 && month == entries[0].data.tm_mon + 1 && entries[0].data.tm_mday == day) {
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
            printf("Nel register e' gia' presente una entry per oggi: la vado ad aggiornare\n");
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
