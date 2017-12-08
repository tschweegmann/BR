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
    FILE* file = fopen("dir.zip", "a");
    unsigned char buff[1024];
    unsigned char shabuff[64];
    int tolen;
    unsigned char typID;
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
    /* sending request */
    typID = REQUEST_T;
    err = sendto(fd, &typID, sizeof(typID), 0, (struct sockaddr*) &to, tolen);
    if(err == -1)
    {
        printf("ERROR: couldnt send\n");
    }
    else
    {
        printf("Data sent %d Bytes\n", err);
    }

    /* receiving header */
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
    while(1)
    {
        err = recvfrom(fd, buff, sizeof(buff), 0, (struct sockaddr*) &to, &tolen); 
        if (*buff != DATA_T) 
        {
            printf("No more DATA_T packages!\n");
            break;
        }
        if (err == -1) 
        {
            printf("ERROR: couldnt receive\n");
            exit(-1);
        }
        printf("DATA_T: %d \n", *buff);
        for (i = 5; i < 1019; i++) fputc(buff[i], file); 
    }
    fclose(file);

    /* receiving SHA */
    printf("receifing SHA512...\n");
    //err = recvfrom(fd, shabuff, sizeof(shabuff), 0, (struct sockaddr*) &to, &tolen);
    if (*buff == SHA512_T)
    {
        printf("received SHA512 typID\n");
    }

    /* sending SHA512_CMP_T */
    typID = SHA512_CMP_T;

    /* close socket */
    close(fd);
    return 0;
}
