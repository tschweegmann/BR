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
    fd_set fds;
    int err;
    struct sockaddr_in server;
    FILE* file;
    unsigned char buff[BUFFERSIZE];
    unsigned char shabuff[64];
    unsigned char recvsha[64];
    unsigned char* filebuff;
    int tolen;
    struct timeval tv;
    int i = 0;
    char* filename;
    unsigned short namelength = 0;
    unsigned int filesize = 0;
    int receivedBytes = 0;
    tolen = sizeof(server);

    /* Input */
    port = atoi(argv[1]);
    if (port > 65535 || port < 0)
    {
        printf(port_error, argv[1]);
        exit(-1);
    }
    addr = argv[2];
    if (inet_pton(AF_INET, addr, &(server.sin_addr)) == 0)
    {
        printf(address_error, addr, argv[1]);
        exit(-1);
    }
    printf("Connecting to %s\n", addr);

    /* socket() */
    fd = socket(AF_INET, SOCK_STREAM, 0);
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    /* server address */
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(addr);

    //FD_ZERO(&fds);
    //FD_SET(fd, &fds);
    printf("connecting...\n");
    err = connect(fd, (const struct sockaddr*) &server, tolen);
    if (err == 0) printf("connected\n");
    else exit(-1);
    //read() Header
    printf("reading::\n");
    memset(buff, 0, BUFFERSIZE);
//if (select(fd +1, &fds, NULL, NULL, &tv) > 0) {
    err = read(fd, buff, BUFFERSIZE);
    if (err == -1) printf("ERROR recv header\n");
    if (err == 0) printf("EOF\n");
    //for(i = 0; i < BUFFERSIZE; i++) printf("%d\n", buff[i]);
//}
    /* get header information*/
    memcpy(&namelength, buff, 2);
    filename = malloc(namelength);
    memcpy(filename, buff + 2, namelength);
    memcpy(&filesize, buff + 2 + namelength, 4);

    /* print information */
    printf(filename_str, filename);
    printf(filesize_str, filesize);

    /* receiving data */
    file = fopen("dir.zip", "w");
    if (file == NULL)
    {
       perror("couldnt open file: ");
    }

    select(fd, &fds, NULL, NULL, &tv);
    //read Data
    printf("read data...\n");
    while(1)
    {
        err = read(fd, buff, BUFFERSIZE); 
        if (err == -1)
        {
            printf("ERROR: couldnt receive\n");
            printf(timeout_error);
            exit(-1);
        }
//        if (err == 0)
//        {
//            printf("End of Data transmission");
//            break;
//        }
if(err!=0){        /* write data to file */
        for (i = 0; i < BUFFERSIZE; i++) 
        {
            fputc(buff[i], file);
            receivedBytes++;
            if (receivedBytes == filesize) i = 9999;
        }
        memset(buff, 0, BUFFERSIZE);
}
    }
    fclose(file);
printf("end reading\n");
    //read SHA
    read(fd, buff, BUFFERSIZE);
    printf("receiving SHA512...\n");
    memcpy(recvsha, buff + 1, 64);
    for(i = 0; i < 64; i++) printf("%d\n", recvsha[i]); 

    file = fopen("dir.zip", "r");
    filebuff = malloc(filesize);
    for (i = 0; i < filesize; i++) filebuff[i] = fgetc(file);
    SHA512(filebuff, filesize, shabuff);
    //for(i = 0; i < 64; i++) printf("%d\n", shabuff[i]);
    //printf("strlen() = %d\n", strlen(shabuff));

    //write() SHA_CMP

    //close()
    close(fd);
    return 0;
}
