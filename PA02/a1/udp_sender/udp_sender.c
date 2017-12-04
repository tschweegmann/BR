#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Aufgabe2.h"

int main(int argc, char** argv)
{
    char* adress;
    int port;
    int fd;
    struct sockaddr_in from;
    struct sockaddr_in to;
    void* request = malloc(8);
    adress = argv[1];
    port = atoi(argv[2]);
    int tolen;
    unsigned short namelength
    string filename;
    
    //socket()
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    tolen = sizeof(to);
    // REQUEST_T empfangen
    recvfrom(fd, request, sizeof(request), 0, (struct sockaddr*)&to, &tolen);
    if (request == REQUEST_T)
    {
        cout << "received Request!" << endl;
    }

    // HEADER_T senden (HEADER_T, unsigned short namelength, string filename, unsigned integer filesize)
    sendto(fd, );
    // DATA_T senden (DATA_T, unsigned integer SeqNmbr(0-n), Daten)

    // SHA512_T senden (SHA512_T, SHA-512-HAshwert(64Bytes))

    // SHA512_CMP_T empfangen (SHA512_CMP_T, Vergleichsergebniss(1Bytes))

    //close socket
}
