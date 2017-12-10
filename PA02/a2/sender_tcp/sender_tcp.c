#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "Aufgabe2.h"
#include <string.h>
#include <openssl/sha.h>
#include <sys/time.h>
#include <libgen.h>
#include <stdlib.h>

#define BUFFERSIZE 1024

int main(int argc, char** argv)
{
/* Input */
    char* path;
    int port;

    int fd; /* Socket */
    int fd2; /* client socket */
    fd_set fds;
    int err;
    struct sockaddr_in server;
    struct sockaddr_in client;
    int tolen;
    struct timeval tv;
    FILE* file;
    unsigned char* buff = malloc(BUFFERSIZE);
    unsigned char* filebuff;

    /* Zip creation */
    char* zipstring = "zip -r dir.zip ";
    char command[sizeof(zipstring) + sizeof(path)];

    /* Header */
    char* filename;
    unsigned short namelength;
    unsigned int filesize;
    int headersize = sizeof(namelength) + sizeof(filename) + sizeof(filesize);
    unsigned char header[headersize];
    char* filenameptr = 0;
    unsigned short* namelengthptr = 0;
    unsigned int* filesizeptr = 0;

    int c;
    int i;
    int eof = 0;

    /* DATA_T package stuff*/
    unsigned char* data = 0;

    /* SHA512 stuff*/
    unsigned char shabuff[65];
    unsigned char mySha512[64];
    unsigned char cmpResult;

    /* checking input */
    port = atoi(argv[1]);
    if (port > 65535 || port < 0)
    {
        printf(port_error, argv[1]);
        exit(-1);
    }
    path = argv[2];
    if (access(path, F_OK) == -1)
    {
        printf("ERROR: File does not exist!\n");
        exit(-1);
    }

    /* set client address values */
    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    //to.sin_addr.s_addr = htonl(INADDR_ANY);

    /* set server address*/
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Zip file */
    strcat(command, zipstring);
    strcat(command, path);
    system(command);
    file = fopen("dir.zip", "rw");

    /* Header */
    /* get data needed for header */
    namelength = strlen(basename(path));
    filename = malloc(namelength);
    filename = basename(path);
    fseek(file, 0L, SEEK_END);
    filesize = ftell(file);
    rewind(file);
    /* file information output */
    printf(filename_str, filename);
    printf(filesize_str, filesize);

    /* add to header */
    /* namelength */
    namelengthptr = (unsigned short*)(header);
    memcpy(namelengthptr, &namelength, sizeof(namelength));
    /* filename */
    filenameptr = (char*)(header + sizeof(unsigned short));
    memcpy(filenameptr, filename, strlen(filename));
    /* filesize */
    filesizeptr = (unsigned int*)(header + sizeof(namelength) + strlen(filename));
    memcpy(filesizeptr, &filesize, sizeof(filesize));

    //calc SHA-512
    SHA512_CTX ctx;
    SHA512_Init(&ctx);

    /* Socket */
    fd = socket(AF_INET, SOCK_STREAM, 0);
    tolen = sizeof(struct sockaddr_in);
    bind(fd, (struct sockaddr*) &server, sizeof(server));

    filebuff = malloc(filesize);
    for (i = 0; i < filesize; i++) 
    {
        filebuff[i] = fgetc(file);
    }
    SHA512(filebuff, filesize, mySha512);
    //for(i = 0; i < 64; i++) printf("%d\n", mySha512[i]);
    //printf("strlen() = %d\n", strlen(mySha512));
    rewind(file);

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    /* listen() */
    printf("listen...\n");
    listen(fd, 5);

    /* select() */
    printf("select...\n");
    //while(1)
    //{
  //  if (select(fd + 1, &fds, NULL, NULL, NULL) > 0)
   // {
    //    printf("ERROR in select!\n");
  //      exit(-1);
    
    //else break;
    //}
    /* accept() */
    printf("waiting for connection...\n");
    fd2 = accept(fd, (struct sockaddr*) &client, &tolen);
    if(fd2 < 0) printf("no connection..\n");
    else printf("cennection!\n");
    printf("fd2: %d\n", fd2);
    printf("accepted!\n");

    /* write() Header */
    write(fd2, header, headersize);

    //write() Data
    /* send data */
    printf("sending data...\n"); 
    while(eof == 0)
    {
        /* prepare datagram*/
        memset(buff, 0, BUFFERSIZE); /* clear buffer *
        /* read from file */
        for (i = 0; i < BUFFERSIZE; i++)
        {
            c = fgetc(file);
            printf("%d", c);
            if (c == EOF) 
            {
                eof = 1;
                break;
            }
            memcpy(buff, &c, sizeof(c));
        }
        while(1) {
if (select(fd + 1, NULL, &fds, NULL, &tv) > 0) break;
}
        err = write(fd, buff, BUFFERSIZE);
        printf("\n\n%d\n", err);
        if (err == -1)
        {
            printf("couldnt send data\n");
            exit(-1);
        }
        if (err == 0)
        {
            printf("EOF\n");
        }
    }
    printf("all data sent!\n");

    /* closing file */
    if (fclose(file) == EOF) 
    {
        printf("closing file error");
        exit(-1);
    }

    /* write() SHA512 */
    printf("Sending SHA512...\n");
    memcpy(shabuff, &mySha512, 64);
    err = write(fd, shabuff, sizeof(shabuff));
    if (err == -1)
    {
        printf("ERROR: couldnt send SHA!\n");
        exit(-1);
    }
    else
    {
        printf("Send SHA512 size: %d Bytes\n", err);
    }
//}
//read() SHA_CMP

//read() close

//close()

close(fd);
return 0;
}
