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
    int i = 0;
    int port;
    const char* addr;
    int fd, err;
    struct sockaddr_in from;
    struct sockaddr_in to;
    FILE* file;
    unsigned char buff[1024];
    unsigned char shabuff[64];
    unsigned char request = 5;
    int tolen;
    struct timeval tv;
    tolen = sizeof(to);
    port = atoi(argv[1]);
    addr = argv[2];
    printf("Connecting to %s\n", addr);

    //socket()
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    tv.tv_sec = 10;
    tv.tv_usec = 0;
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
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    //REQUEST_T schicken
    err = sendto(fd, &request, sizeof(request), 0, (struct sockaddr*) &to, tolen);
    if(err == -1)
    {
        printf("ERROR: couldnt send\n");
    }
    else 
    {
        printf("Data sent %d Bytes\n", err);
    }
    //Auf HEADER_T Antwort warten
    printf("waiting for response...\n");
    err = recvfrom(fd, buff, sizeof(buff), 0, (struct sockaddr*) &to, &tolen);

    if (err == -1)
    {
        printf("Nothing received :(\n");
    }
    else
    {
        printf("Received %d Bytes\n", err);
    }
    printf("Header: %d", (int)buff[0]);
    printf("namelength: %d", (short)buff[1]);
    //Datagramme DATA_T empfangen
    printf("receiving Datagram...\n");
    while(buff[0] != (unsigned char)SHA512_T)
    {
        recvfrom(fd, buff, sizeof(buff), 0, (struct sockaddr*) &to, &tolen);
    }

    //SHA-512 empfangen SHA512_T
    recvfrom(fd, shabuff, sizeof(shabuff), 0, (struct sockaddr*) &to, &tolen);
    // Vergleichsergebnis übertragen SHA512_CMP_T

    // close socket
    close(fd);
    return 0;
}
