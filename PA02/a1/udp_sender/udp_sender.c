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

int main(int argc, char** argv)
{
    /* Input */
    char* path;
    int port;

    int fd; /* Socket */
    int err;
    struct sockaddr_in from;
    struct sockaddr_in to;
    unsigned char typID; /* buffer for REQUEST_T HEADER_T etc (probably irrelevant)*/
    int tolen;
    struct timeval tv; /* to make recvfrom() wait */
    FILE* file;
    unsigned char* buff[1024];

    /* Zip creation stuff */
    char* zipstring = "zip -r dir.zip ";
    char* command[sizeof(zipstring) + sizeof(path)];
    /* Header stuff*/
    char* filename;
    unsigned short namelength;
    unsigned int filesize;
    /* Header still takes to much mem: 120Bytes instead of 15 :/*/
    unsigned char* header[sizeof(typID) + sizeof(namelength) + sizeof(filename) + sizeof(filesize)];
    /* ptr to things in header*/
    char* filenameptr = 0;
    unsigned short* namelengthptr = 0;
    unsigned int* filesizeptr = 0;

    /* SHA512 stuff*/
    unsigned char* shabuff[65];
    unsigned char* mySha512[64];
    unsigned char cmpResult;

    int c;
    int i;
    int eof = 0;
    unsigned int seqNr;
    unsigned int* seqNrptr = 0;
    unsigned char* data = 0;

    path = argv[2];
    port = atoi(argv[1]);

    /* set local address values */
    from.sin_family = AF_INET;
    from.sin_port = htons(port);
    from.sin_addr.s_addr = htonl(INADDR_ANY);


    // Zip file
    printf("File path: %s\n", path);
    strcat(command, zipstring);
    strcat(command, path);
    printf("zip command: %s\n", command);
    system(command);
    file = fopen("dir.zip", "rw");

    // init Header
    printf("Header:\n");
    filename = basename(path);
    printf("filename: %s\n", filename);
    namelength = strlen(filename);
    printf("namelength %d\n", namelength);
    fseek(file, 0L, SEEK_END);
    filesize = ftell(file);
    rewind(file);
    printf("Filesize: %d\n", filesize);

    // creaete Header
    typID = HEADER_T;
    memcpy(header, &typID, sizeof(typID));
    namelengthptr = header + sizeof(typID);
    memcpy(namelengthptr, &namelength, sizeof(namelength));
    filenameptr = header + sizeof(typID) + sizeof(unsigned short);
    memcpy(filenameptr, &filename, sizeof(filename));
    filesizeptr = header + sizeof(typID) + sizeof(namelength) + sizeof(filename);
    memcpy(filesizeptr, &filesize, sizeof(filesize));

    printf("HEADER_T in header: %d \n", *header);
    printf("namelength in header: %d \n", *namelengthptr);
    memcpy(&filename, filenameptr, sizeof(filename));
    printf("filenameptr in header : %s\n", filename);
    printf("filesize in header: %d \n", *filesizeptr);

    //calc SHA-512
    SHA512_CTX ctx;
    SHA512_Init(&ctx);
    //SHA512_Update(&ctx, file, filesize);
    //SHA512_Final();
    //SHA512(const unsigned char* text, size_t buffer size, shabuffer); //returns pointer to hash

    // Socket
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    //setTimeout to 10 sec
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    tolen = sizeof(struct sockaddr_in);
    bind(fd, (struct sockaddr*) &from, sizeof(from));

    /* Data exchange */
    /* initial REQUEST_T */
    printf("waiting for request...\n");
    err = recvfrom(fd, &typID, sizeof(typID), 0, (struct sockaddr*)&to, &tolen);
    if (err == -1)
    {
        printf("ERROR: nothing received\n");
        exit(-1);
    }
    else
    {
        printf("Received %d Bytes\n", err);
    }

    /* send header */
    printf("sending header...\n");
    sendto(fd, header, sizeof(header), 0, (struct sockaddr*)&to, tolen);

    /* send data */
    typID = DATA_T;
    seqNr = 0;
    data = buff + sizeof(typID) + sizeof(seqNr);
    while(eof == 0)
    {
        memset(buff, 0, sizeof(buff));
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
        printf("sent DGRAM!\n");
        sendto(fd, buff, sizeof(buff), 0, (struct sockaddr*)&to, tolen);
    }
    fclose(file);
    printf("all DGRAMS sent\n");

    /* send SHA512 (SHA512_T, SHA-512-Hashwert(64Bytes)) */
    typID = SHA512_T;
    printf("Sending SHA512...\n");
    strcat(shabuff, (unsigned char*) &SHA512_T);
    strcat(shabuff, &mySha512);
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
        printf("ERROR: No SHA_CMP_T received\n");
        exit(-1);
    }
    else
    {
        printf("received SHA_CMP_T\n");
    }
    if (cmpResult == SHA512_CMP_OK) printf("SHA_CMP is correct!");
    else printf("SHA_CMP does not match!");
    /* close socket */
    close(fd);
}
