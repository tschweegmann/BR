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
    /* Input */
    int port;
    const char* addr;

    int fd; /* Socket */
    int err;
    struct sockaddr_in from;
    struct sockaddr_in to;
    FILE* file;
    unsigned char buff[1024];
    unsigned char shabuff[64];
    unsigned char request = 5;
    int tolen;
    struct timeval tv;
    int i = 0;

    tolen = sizeof(to);

    /* Input */
    port = atoi(argv[1]);
    addr = argv[2];


    printf("Connecting to %s\n", addr);

    /* socket() */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    /* sender address */
    from.sin_family = AF_INET;
    from.sin_port = htons(port);
    from.sin_addr.s_addr = htonl(INADDR_ANY);
    /* receiver address */
    to.sin_family = AF_INET;
    to.sin_port = htons(port);
    to.sin_addr.s_addr = inet_addr(addr); 

    bind(fd, (struct sockaddr*) &from, sizeof(from));

    /* Data exchange*/
    /* sending REQUEST_T */
    err = sendto(fd, &request, sizeof(request), 0, (struct sockaddr*) &to, tolen);
    if(err == -1)
    {
        printf("ERROR: couldnt send\n");
    }
    else 
    {
        printf("Data sent %d Bytes\n", err);
    }

    /* receiving HEADER_T */
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

    /* receiving data */
    printf("receiving Datagram...\n");
    while(buff[0] != (unsigned char)SHA512_T)
    {
        recvfrom(fd, buff, sizeof(buff), 0, (struct sockaddr*) &to, &tolen);
    }

    /* receiving SHA */
    recvfrom(fd, shabuff, sizeof(shabuff), 0, (struct sockaddr*) &to, &tolen);

    /* sending SHA512_CMP_T */

    /* close socket */
    close(fd);
    return 0;
}
