#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Aufgabe2.h"

int main(int argc, char** argv)
{
    int port;
    char* path;
    int fd;
    struct sockaddr_in from;
    struct sockaddr_in to;
    FILE* file;
    unsigned char* buff[1024];

    port = atoi(argv[1]);
    path = argv[2];


    //socket()
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    // Sender Adresse
    from.sin_family = AF_INET;
    from.sin_port = htons(port);
    from.sin_addr.s_addr = htonl(INADDR_ANY);
    // Empfänger Adresse
    to.sin_family = AF_INET;
    to.sin_port = htons(port);
    //to.sin_addr.s_addr = 
    
    //Bind()
    bind(fd, (struct sockaddr*) &from, sizeof(from));
    //REQUEST_T schicken

    //Auf HEADER_T Antwort warten

    //Datagramme DATA_T empfangen

    //SHA-512 empfangen SHA512_T

    // Vergleichsergebnis übertragen SHA512_CMP_T

    // close socket
    close(fd);
    return 0;
}
