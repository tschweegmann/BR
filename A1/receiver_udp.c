#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <unistd.h>
#include <sys/time.h>
#include "Aufgabe2.h"
#include <string.h>

#define BUFFERSIZE 1024

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
    unsigned char buff[BUFFERSIZE];
    unsigned char shabuff[64];
    unsigned char recvsha[64];
    unsigned char* filebuff;
    int tolen;
    unsigned char typID;
    struct timeval tv;
    int i = 0;
    unsigned int seqNr = 0;
    unsigned int nextseqNr = 1;
    char* filename;
    unsigned short namelength = 0;
    unsigned int filesize = 0;
    int receivedBytes = 0;
    tolen = sizeof(to);

    /* Input */
    port = atoi(argv[2]);
    if (port > 65535 || port < 0)
    {
        printf(port_error, argv[1]);
        exit(-1);
    }
    addr = argv[1];
    if (inet_pton(AF_INET, addr, &(to.sin_addr)) == 0)
    {
        printf(address_error, addr, argv[1]);
        exit(-1);
    }
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

    /* data exchange*/
    /* sending request */
    typID = REQUEST_T;
    err = sendto(fd, &typID, sizeof(typID), 0, (struct sockaddr*) &to, tolen);
    if(err == -1)
    {
        printf("ERROR: couldnt send\n");
    }
    else
    {
        printf("Request sent (%d Bytes)\n", err);
    }

    /* receiving header */
    printf("waiting for response...\n");
    err = recvfrom(fd, buff, BUFFERSIZE, 0, (struct sockaddr*) &to, &tolen);
    if (err == -1)
    {
        printf(timeout_error);
        exit(-1);
    }
    else
    {
        printf("Received header (%d Bytes)\n", err);
    }
    if (*buff != HEADER_T)
    {
        printf(packet_error);
        exit(-1);
    }

    /* get header information*/
    memcpy(&namelength, buff + 1, 2);
    filename = malloc(namelength);
    memcpy(filename, buff + 3, namelength);
    memcpy(&filesize, buff + 3 + namelength, 4);

    /* print information */
    printf(filename_str, filename);
    printf(filesize_str, filesize);

    /* receiving data */
    file = fopen("dir.zip", "w");
    if (file == NULL)
    {
       perror("couldnt open file: ");
    }
    printf("receiving Datagram...\n");

    while(1)
    {
        err = recvfrom(fd, buff, BUFFERSIZE, 0, (struct sockaddr*) &to, &tolen); 
        /* check correct typ-ID*/
        if (*buff != DATA_T)
        {
            printf("No more DATA_T packages!\n");
            break;
        }
        if (err == -1)
        {
            printf("ERROR: couldnt receive\n");
            printf(timeout_error);
            exit(-1);
        }
        memcpy(&seqNr, (buff + 1), sizeof(unsigned int));
        /*check sequence number*/
        if (seqNr != nextseqNr - 1)
        {
            printf(order_error, seqNr, nextseqNr - 1);
            exit(-1);
        }
        /* write data to file */
        for (i = 5; i < 1024; i++) 
        {
            fputc(buff[i], file);
            receivedBytes++;
            if (receivedBytes == filesize) i = 9999;
        }
        nextseqNr++;
        memset(buff, 0, BUFFERSIZE);
    }
    fclose(file);

    /* receiving SHA */
    printf("receifing SHA512...\n");
    if (*buff == SHA512_T)
    {
        printf("received SHA512 typID\n");
    }
    else printf(packet_error);
    memcpy(recvsha, buff + 1, 64);

    file = fopen("dir.zip", "r");
    filebuff = malloc(filesize);
    for (i = 0; i < filesize; i++) filebuff[i] = fgetc(file);
    SHA512(filebuff, filesize, shabuff);

    //Print calculated SHA-Value of receiver
    unsigned char* shaString; 
    shaString = create_sha512_string(shabuff);
    printf(receiver_sha512, shaString);

    /* sending SHA512_CMP_T */
    if(strcmp(shabuff, recvsha))
    {
        typID = SHA512_CMP_OK;
        printf("sha512_ok\n");
    }
    else
    {
        typID = SHA512_CMP_ERROR;
        printf("sha512_error\n");
    }
    sendto(fd, &typID, sizeof(typID), 0, (struct sockaddr*) &to, tolen);

    printf("transmission completed\n");

    /* close socket */
    fclose(file);
    close(fd);
    free(filename);
    return 0;
}
