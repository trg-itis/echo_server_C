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
  #include <pthread.h>
#endif

#define TCP_PORT (5501)
#define MAXPENDING (5)   // numero massimo di connessioni contemporanee
#define RCVBUFSIZE (64)  // dimensione del buffer di ricezione

typedef struct          // STRUCT CHE VA A DEFINIRE I PARAMETRI
{       
    int intsock;        // socket del client
    int remoteport;     // oirta renite del cliente (effimera)
} t_threadargs;


void OnError(const char *errorMessage) {
    fprintf(stderr, "Errore: %s\n", errorMessage);
    exit(1);
}

// FUNZIONI
int AcceptTCPConnection(int sockS, int* remoteport);    // Accettazione della connessione
void *threadClient(void* arg);                          // Thread per ogni client connesso


int main(void){

    int sockServer;     // Socket server
    int sockClient;     // Socket client

    int remotePort;
    struct sockaddr_in addrServer;      // Indirizzo server locale
    pthread_t threadID;                  // Thread ID del client 

    t_threadargs *threadArgs;

    // CREAZIONE SOCKET SERVER
    if ((sockServer = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) OnError("socket() fallita");
    // ----------------------------



    // ----- SETTAGGIO DEL SOCKET ------
    memset(&addrServer, 0, sizeof(addrServer));
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = htonl(INADDR_ANY);     // IP LOCALE
    addrServer.sin_port = htons(TCP_PORT);              // PORTA LOCALE
    // ----------------------------------



    // BINDING (ASSEGNAZIONE PORTA)
    if (bind(sockServer, (struct sockaddr *) &addrServer, sizeof(addrServer)) < 0) OnError("bind() fallita");
    // ----------------------------------


    // CONNESSIONE PASSIVA DEL SERVER
    if (listen(sockServer, MAXPENDING) < 0) OnError("listen() fallita");

    printf("\n-> Server in ascolto su porta %d", TCP_PORT);

    while (1){
        sockClient = AcceptTCPConnection(sockServer, &remotePort); // Client connesso

        threadArgs = (t_threadargs *) malloc(sizeof(t_threadargs));
        // PARAMETRI DA PASSARE AL THREAD CLIENT
        threadArgs ->intsock = sockClient; // socket 
        threadArgs ->remoteport = remotePort; // porta effimera 

        if (pthread_create(&threadID, NULL, (void *(*)(void *))threadClient, threadArgs) != 0) {
            OnError("pthread_create() fallita");
        }

        printf("\n\t (sul thread %lu)\n", (unsigned long)threadID);

    }
    return 0;

}


void* threadClient(void *threadArgs){
    int sockC;                  // Socket client
    char recvBuff[RCVBUFSIZE];  // Buffer di ricezione
    int recvBuffSize;           // Dimensione effettiva dati ricevuti
    int remotePort;             // Porta remota (effimera)

    sockC = ((t_threadargs*) threadArgs)->intsock;              // Client socket
    remotePort = ((t_threadargs*) threadArgs)->remoteport;      // Client port effimera
    free(threadArgs);

    recvBuff[0] = 0;

    while (recvBuff[0]!='x')    // Finché non si riceve 'x'
    {
        recvBuffSize = recv(sockC, recvBuff, RCVBUFSIZE, 0); // FUNZIONE BLOCCANTE
        if (recvBuffSize>0){
            recvBuff[recvBuffSize] = 0;
            send(sockC, recvBuff, recvBuffSize, 0); // ECHO SU CLIENT
            printf("\n\nClient (remote port %d); \tRicevuto e risposto: %s ",remotePort, recvBuff);
            fflush(stdout); // FORZARE LA STAMPA IMMEDIATA 
        }
    }

    close(sockC);
    printf("\n\nClient (remote port %d) Chiuso",remotePort);
    return (NULL);
}


int AcceptTCPConnection(int sockS, int* remoteport){
    int sockC;                          // Client socket
    struct sockaddr_in addrClient;      // Client address
    socklen_t addrSize;

    addrSize = sizeof(addrClient);

    if((sockC = accept(sockS, (struct sockaddr *) &addrClient, &addrSize)) < 0) OnError("accept() fallita");

    *remoteport = ntohs(addrClient.sin_port);
    printf("\n\nClient connesso. IP remoto: %s, Porta remota: %d", inet_ntoa(addrClient.sin_addr), *remoteport);
    return sockC;
}
    