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
    int err;
    struct sockaddr_in from;
    struct sockaddr_in to;
    unsigned char typID;
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
    int headersize = sizeof(typID) + sizeof(namelength) + sizeof(filename) + sizeof(filesize);
    unsigned char header[headersize];
    char* filenameptr = 0;
    unsigned short* namelengthptr = 0;
    unsigned int* filesizeptr = 0;

    int c;
    int i;
    int eof = 0;

    /* DATA_T package stuff*/
    unsigned int seqNr;
    unsigned int* seqNrptr = 0;
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

    /* set local address values */
    from.sin_family = AF_INET;
    from.sin_port = htons(port);
    from.sin_addr.s_addr = htonl(INADDR_ANY);

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
    /* Typ-id */
    typID = HEADER_T;
    memcpy(header, &typID, sizeof(typID));
    /* namelength */
    namelengthptr = (unsigned short*)(header + sizeof(typID));
    memcpy(namelengthptr, &namelength, sizeof(namelength));
    /* filename */
    filenameptr = (char*)(header + sizeof(typID) + sizeof(unsigned short));
    memcpy(filenameptr, filename, strlen(filename));
    /* filesize */
    filesizeptr = (unsigned int*)(header + sizeof(typID) + sizeof(namelength) + strlen(filename));
    memcpy(filesizeptr, &filesize, sizeof(filesize));

    //calc SHA-512
    SHA512_CTX ctx;
    SHA512_Init(&ctx);
    //SHA512_Update(&ctx, file, filesize);
    //SHA512_Final();
    //SHA512(const unsigned char* text, size_t buffer size, shabuffer); //returns pointer to hash

    /* Socket */
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    tolen = sizeof(struct sockaddr_in);
    bind(fd, (struct sockaddr*) &from, sizeof(from));

    filebuff = malloc(filesize);
    for (i = 0; i < filesize; i++) 
    {
        filebuff[i] = fgetc(file);
    }
    SHA512(filebuff, filesize, mySha512);
    //for(i = 0; i < 64; i++) printf("%d\n", mySha512[i]);
    //printf("strlen() = %d\n", strlen(mySha512));
    rewind(file);

    /* Data exchange */
    /* recv initial request */
    printf("waiting for request...\n");
    err = recvfrom(fd, &typID, sizeof(typID), 0, (struct sockaddr*)&to, &tolen);
    if (err == -1)
    {
        printf("%s", timeout_error);
        exit(-1);
    }
    if (*buff != REQUEST_T)
    {
        printf(packet_error);
        exit(-1);
    }
    printf("received request!\n");

    /* set socket timeout (10 sec) */
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    /* send header */
    printf("sending header...\n");
    err = sendto(fd, header, headersize, 0, (struct sockaddr*)&to, tolen);
    if (err == -1)
    {
         printf("ERROR: couldnt send Header!\n");
         exit(-1);
    }
    else printf("finished sending header!\n");

    /* send data */
    printf("sending data...\n");
    typID = DATA_T;
    seqNr = 0;
    data = (unsigned char*)(buff + sizeof(typID) + sizeof(seqNr)); /* Pointer to real data */ 
    while(eof == 0)
    {
        /* prepare datagram*/
        memset(buff, 0, BUFFERSIZE); /* clear buffer */
        memcpy(buff, &typID, sizeof(typID));
        memcpy(buff + sizeof(typID), &seqNr, sizeof(seqNr));
        /* read from file */
        for (i = 0; i < 1019; i++)
        {
            c = fgetc(file);
            if (c == EOF) 
            {
                eof = 1;
                break;
            }
            memcpy(data + i, &c, sizeof(c));
        }
        err =sendto(fd, buff, BUFFERSIZE, 0, (struct sockaddr*)&to, tolen);
        if (err == -1)
        {
            printf("couldnt send data\n");
            exit(-1);
        }
        seqNr++;
    }
    printf("all data sent!\n");

    /* closing file */
    if (fclose(file) == EOF) 
    {
        printf("closing file error");
        exit(-1);
    }



    /* under construction xD*/
    /* send SHA512 (SHA512_T, SHA-512-Hashwert(64Bytes)) */
    typID = SHA512_T;
    printf("Sending SHA512...\n");
    memcpy(shabuff, (unsigned char*) &SHA512_T, 1);
    memcpy(shabuff + 1, &mySha512, 64);
    err = sendto(fd, shabuff, sizeof(shabuff), 0, (struct sockaddr*) &to, tolen);
    if (err == -1)
    {
        printf("ERROR: couldnt send SHA!\n");
        exit(-1);
    }
    else
    {
        printf("Send SHA512 size: %d Bytes\n", err);
    }

    /* receive SHA512 compare result */
    printf("receiving compare result...\n");
    err = recvfrom(fd, &cmpResult, 1, 0, (struct sockaddr*) &to, &tolen);
    if (err == -1)
    {
        printf("%s", timeout_error);
        exit(-1);
    }
    else
    {
        printf("received SHA_CMP_T\n");
    }

    if (cmpResult == SHA512_CMP_OK) printf("sha512_ok\n");
    else if (cmpResult == SHA512_CMP_ERROR) 
    {
        printf("sha512_error");
        exit(-1);
    }
    else
    {
        printf(packet_error);
        exit(-1);
    }
    printf("transmission completed\n");

    /* close socket */
    close(fd);
    free(buff);
}
