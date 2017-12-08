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
    void* request[8]; /* buffer for REQUEST_T (probably irrelevant)*/
    int tolen;
    struct timeval tv; /* to make recvfrom() wait */
    FILE* file;
    unsigned char* buff[1024];

    /* Zip creation stuff */
    char* zipstring = "zip -r dir.zip ";
    char* command = malloc(sizeof(zipstring) + sizeof(path));

    /* Header stuff*/
    char* filename;
    unsigned short namelength;
    unsigned int filesize;
    char* header = malloc(sizeof(HEADER_T) + sizeof(namelength) + sizeof(filename) + sizeof(filesize));

    /* SHA512 stuff*/
    unsigned char* shabuff[65];
    unsigned char* mySha512[64];
    unsigned char* recvSha512[64];

    int c;
    int i;
    int eof = 0;

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
    filename = basename(path);
    namelength = strlen(filename);
    fseek(file, 0L, SEEK_END);
    filesize = ftell(file);
    rewind(file);
    printf("Filesize: %d", filesize);

    // creaete Header 
    strcat(header, (unsigned char*) &HEADER_T);
    strcat(header, (unsigned char*) &namelength);
    strcat(header, (unsigned char*) &filename);
    strcat(header, (unsigned char*) &filesize);

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

    // Data exchange
    // initial REQUEST_T
    printf("waiting for request...\n");
    err = recvfrom(fd, request, sizeof(request), 0, (struct sockaddr*)&to, &tolen);
    if (err == -1)
    {
        printf("ERROR: nothing received\n");
    }
    else 
    {
        printf("Received %d Bytes\n", err);
    }

    // send HEADER_T
    printf("sending header...\n");
    sendto(fd, header, sizeof(header), 0, (struct sockaddr*)&to, tolen);

    // DATA_T senden (DATA_T, unsigned integer SeqNmbr(0-n), Daten)
    while(eof == 0)
    {
        for (i = 0; i < 1024; i++)
        {
            c = fgetc(file);
            if (c == EOF) 
            {
                eof = 1;
                break;
            }
            buff[i] = (unsigned char)c;
            printf("%c", buff[i]);
        }
        printf("sent DGRAM!\n");
        sendto(fd, buff, sizeof(buff), 0, (struct sockaddr*)&to, tolen);
    }
    printf("all DGRAMS sent\n");

    // SHA512_T senden (SHA512_T, SHA-512-Hashwert(64Bytes))
    printf("Sending SHA512...\n");
    strcat(shabuff, (unsigned char*) &SHA512_T);
    strcat(shabuff, &mySha512);
    err = sendto(fd, shabuff, sizeof(shabuff), 0, (struct sockaddr*) &to, tolen);
    if (err == -1)
    {
        printf("ERROR: couldnt send SHA!\n");
    }
    else
    {
        printf("Send SHA512 size: %d Bytes", err);
    }
    // SHA512_CMP_T empfangen (SHA512_CMP_T, Vergleichsergebniss(1Bytes))
    //recvfrom(fd, recvSha512, sizeof(recvSha512), 0, (struct sockaddr*) &to, &tolen);

    //close socket
    close(fd);
}
