#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <unistd.h>
#include <sys/time.h>
#include "Aufgabe2.h"

int main(int argc, char** argv)
{
    int port;
    const char* addr;
    int fd;
    struct sockaddr_in from;
    struct sockaddr_in to;
    FILE* file;
    unsigned char* buff[1024];
    int tolen;
    struct timeval tv;
    tolen = sizeof(to);
    port = atoi(argv[1]);
    addr = argv[2];


    //socket()
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    // Sender Adresse
    from.sin_family = AF_INET;
    from.sin_port = htons(port);
    from.sin_addr.s_addr = htonl(INADDR_ANY);
    // Empfänger Adresse
    to.sin_family = AF_INET;
    to.sin_port = htons(port);
    to.sin_addr.s_addr = inet_addr(addr); 
    
    //Bind()
    bind(fd, (struct sockaddr*) &from, sizeof(from));
    //REQUEST_T schicken
    sendto(fd, &REQUEST_T, sizeof(REQUEST_T), 0, (struct sockaddr*) &to, tolen);
    //Auf HEADER_T Antwort warten
    recvfrom(fd, buff, sizeof(buff), 0, (struct sockaddr*) &to, &tolen);
    
    //Datagramme DATA_T empfangen

    //SHA-512 empfangen SHA512_T

    // Vergleichsergebnis übertragen SHA512_CMP_T

    // close socket
    close(fd);
    return 0;
}
