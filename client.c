#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
  // Controlla se si sta compilando su win
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  #pragma warning(disable: 4996) // Evita warning vari
#else
  // Controlla se si sta compilando su linux / macos
  #include <unistd.h>         // close()
  #include <arpa/inet.h>      // inet_pton(), htons(), ecc.
  #include <sys/socket.h>     // socket(), bind(), connect(), ecc.
  #include <netinet/in.h>     // struct sockaddr_in
#endif

#define RCVBUFSIZE (64)  /* Dimensione del buffer di ricezione */

void OnError(const char *errorMessage) {
    fprintf(stderr, "Errore: %s\n", errorMessage);
    exit(1);
}

// Funzione che recupera il numero di porta effimera
u_short GetPort(int sock){
    struct sockaddr_in addrs;
    socklen_t addrsSize = sizeof(addrs); 
    // La funzione getsockname si aspetta un puntatore a una variabile di tipo socklen_t e non int
    getsockname(sock,(struct sockaddr *) &addrs, &addrsSize);
    
    return ntohs(addrs.sin_port);
}

int main(int argc, char *argv[]){
    int sock;                       // SOCKET CLIENT
    struct sockaddr_in addrClient;  // ADDRESS DEL SERVER ECHO
    unsigned short remoteport;      // PORTA DEL SERVER ECHO
    char szIPServer[20];            // SERVER IP ADDRESS (DOTTED ex. "127.0.0.1")
    char szTx[RCVBUFSIZE], recvBuff[RCVBUFSIZE]; // STRINGA DI TRASMISSIONE
    int txSize, bytesRcvd, totalBytesRcvd;

    char ch;

    // ----- IMPOSTO DEI VALORI DI DEFAULT ----- 
    strcpy(szIPServer, "127.0.0.1");
    strcpy(szTx, "TRUGLIA");
    remoteport = 5500;
    // ----------------------------------



    // ----- PARAMETRI DA RIGA DI COMANDO -----
    if (argc > 1) strcpy(szTx, argv[1]);        // STRINGA DA TRASMETTERE PASSATA DA LINEA DI COMANDO
    if (argc > 2) strcpy(szIPServer, argv[2]);  // IP DEL SERVER PASSATO DA LINEA DI COMANDO
    if (argc > 3) remoteport = atoi(argv[3]);  // PORTA DEL SERVER PASSATO DA LINEA DI COMANDO
    txSize = strlen(szTx);
    // ----------------------------------



    // ----- CREAZIONE SOCKET CLIENT ----- 
    if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) OnError("socket() fallita");
    // ----------------------------------


    // ----- SETTAGGIO DEL SOCKET ------
    memset(&addrClient, 0, sizeof(addrClient));
    addrClient.sin_family = AF_INET;
    addrClient.sin_addr.s_addr = inet_addr(szIPServer); // IP REMOTO
    addrClient.sin_port = htons(remoteport);            // PORTA REMOTA
    // ----------------------------------



    // ----- TENTIAMO LA CONNESSIONE ----- 
    printf("\n-> TENTATIVO DI CONNESIONE");
    printf("\n-> connessione a %s:%d ...\n", szIPServer, remoteport);

    if (connect(sock, (struct sockaddr *) &addrClient, sizeof(addrClient)) <0) OnError("connect() fallita");

    printf("\n->CONNESSIONE AVVENUTA CON SUCCESSO");
    printf("\nIP Server: %s; \nPorta Server: %d; \nPorta client: %hu; \n(x: Termina)\n",
        inet_ntoa(addrClient.sin_addr), remoteport, GetPort(sock));
    // ----------------------------------
    

    // ----- LOGICA ----- 
    ch = 0;

    char isFirst = 1;

    while (1){
        
        printf("Inserisci stringa (x per terminare): ");
    
        if (fgets(szTx, RCVBUFSIZE, stdin) == NULL) continue;

        // rimuovo il newline finale
        szTx[strcspn(szTx, "\n")] = '\0';

        if (strcmp(szTx, "x") == 0) break;

        int txSize = strlen(szTx);

        // - TENTIAMO LA TRASMISSIONE -
        if (send(sock, szTx, txSize, 0) != txSize) OnError("send() fallita");
        printf("\n\n->Stringa trasmessa: %s", szTx);

        int bytesRcvd = recv(sock, recvBuff, RCVBUFSIZE - 1, 0);
        if (bytesRcvd <= 0) OnError("recv() fallita");

        recvBuff[bytesRcvd] = '\0';
        printf("\n->Stringa ricevuta: %s\n\n", recvBuff);
        
    
    }


    close(sock);
    printf("\nCONNESSIONE TERMINATA <-\n\n");
    exit(0);

    return 0;    
}