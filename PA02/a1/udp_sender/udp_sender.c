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
    struct timeval tv; /* to make recvfrom() wait */
    FILE* file;
    unsigned char* buff = malloc(BUFFERSIZE);

    /* Zip creation stuff */
    char* zipstring = "zip -r dir.zip ";
    char command[sizeof(zipstring) + sizeof(path)];
    /* Header stuff*/
    char* filename;
    unsigned short namelength;
    unsigned int filesize;
    /* Header */
    int headersize = sizeof(typID) + sizeof(namelength) + sizeof(filename) + sizeof(filesize);
    unsigned char header[headersize];
    /* ptr to things in header*/
    char* filenameptr = 0;
    unsigned short* namelengthptr = 0;
    unsigned int* filesizeptr = 0;

    /* SHA512 stuff*/
    unsigned char shabuff[65];
    unsigned char mySha512[64];
    unsigned char cmpResult;

    int c;
    int i;
    int eof = 0;

    /* DATA_T package stuff*/
    unsigned int seqNr;
    unsigned int* seqNrptr = 0;
    unsigned char* data = 0;

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
    //printf("File path: %s\n", path);
    strcat(command, zipstring);
    strcat(command, path);
    //printf("zip command: %s\n", command);
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
    /* (DEBUG) prints data to be included in header*/
    //printf("Header:\n");
    printf(filename_str, filename);
    //printf("namelength %d\n", namelength);
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
    /* (DEBUG) print actual header data */
    //printf("namelength in header: %d \n", *namelengthptr);
    //memcpy(filename, filenameptr, strlen(filename));
    //printf("filenameptr in header : %s\n", filename);
    //printf("filesize in header: %d \n", *filesizeptr);

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

    /* Data exchange */
    /* recv initial request */
    printf("waiting for request...\n");
    err = recvfrom(fd, &typID, sizeof(typID), 0, (struct sockaddr*)&to, &tolen);
    if (err == -1)
    {
        printf("%s", timeout_error);
        exit(-1);
    }
    else
    {
        printf("Received request (%d Bytes)\n", err);
    }
    if (*buff != REQUEST_T)
    {
        printf("%s", packet_error);
        exit(-1);
    }

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
        memset(buff, 0, BUFFERSIZE); /* clear buffer */
        memcpy(buff, &typID, sizeof(typID));
        memcpy(buff + sizeof(typID), &seqNr, sizeof(seqNr));
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
    if (fclose(file) == EOF) 
    {
        printf("closing file error");
        exit(-1);
    }
    //printf("all DGRAMS sent\n");

    /* send SHA512 (SHA512_T, SHA-512-Hashwert(64Bytes)) */
    typID = SHA512_T;
    printf("Sending SHA512...\n");
    strcat(shabuff, (unsigned char*) &SHA512_T);
    strcat(shabuff, mySha512);
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
    err = recvfrom(fd, &cmpResult, sizeof(cmpResult), 0, (struct sockaddr*) &to, &tolen);
    if (err = -1)
    {
        printf("%s", timeout_error);
        exit(-1);
    }
    else
    {
        printf("received SHA_CMP_T\n");
    }

    if (cmpResult == SHA512_CMP_OK) printf("SHA_CMP is correct!");
    else if (cmpResult == SHA512_CMP_ERROR) 
    {
        printf("SHA_CMP does not match!");
        exit(-1);
    }
    else 
    {
        printf("%s", packet_error);
        exit(-1);
    }
    printf("transmission completed\n");

    /* close socket */
    close(fd);
    free(buff);
    free(filename);
}
