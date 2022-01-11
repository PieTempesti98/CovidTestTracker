# CovidTestTracker
Progetto di Reti Informatiche sviluppato da Pietro Tempesti per l'A.A.2020 - 2021.

## L'Applicazione
Basata su un approccio peer-to-peer, l'applicazione si occupa di raccogliere i dati giornalieri di tamponi effettuati ed il numero di nuovi casi. Ogni peer può quindi comunicare con gli altri in modo da poter effettuare delle operazioni aggregazione sui dati raccolti.
Per meglio organizzare la comunicazione tra i peer è presente anche un discovery server, che ha il compito di comunicare ai vari peer quali sono i propri vicini.
Tutto il progetto è scritto in linguaggio C per ambienti UNIX, e i dati sono memorizzati su semplici file di testo.

## Organizzazione del codice
All'interno della repository è presente il codice dei due moduli, il peer e il discovery server, oltre alla documentazione e ad un makefile utile per testare l'applicazione in ambiente locale.

## Installazione ed esecuzione
Dopo aver scaricato il codice (su ambiente UNIX) lanciare il makefile. Si apriranno il discovery server e 5 peer, connessi a diverse porte locali.
