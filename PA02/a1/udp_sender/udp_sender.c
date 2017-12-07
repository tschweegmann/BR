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
    char* path;
    int port;

    int fd, err;
    struct sockaddr_in from;
    struct sockaddr_in to;

    void* request[8];
    path = argv[2];
    port = atoi(argv[1]);
    int tolen;
    struct timeval tv;
    FILE* file;

    from.sin_family = AF_INET;
    from.sin_port = htons(port);
    from.sin_addr.s_addr = htonl(INADDR_ANY);


    // Zip file
    printf("File path: %s\n", path);
    char* zipstring = "zip -r dir.zip ";
    char* command = malloc(sizeof(zipstring) + sizeof(path));
    strcat(command, zipstring);
    strcat(command, path);
    printf("zip command: %s\n", command);
    system(command);

    file = fopen("dir.zip", "rw");
    // Header Zeug
    char* filename = basename(path);
    unsigned short namelength = strlen(filename);
    printf("%s", filename);
    unsigned int filesize;
    fseek(file, 0L, SEEK_END);
    filesize = ftell(file);
    rewind(file);
    printf("Filesize: %d", filesize);
    char* header = malloc(sizeof(HEADER_T) + sizeof(namelength) + sizeof(filename) + sizeof(filesize));
    // Daten in header speichern
    strcat(header, (unsigned char*) &HEADER_T);
    strcat(header, (unsigned char*) &namelength);
    strcat(header, (unsigned char*) &filename);
    strcat(header, (unsigned char*) &filesize);

    unsigned char* mySha512[64];
    unsigned char* recvSha512[64];

    //SHA-512 berechnen
    SHA512_CTX ctx;
    SHA512_Init(&ctx);
    //SHA512(const unsigned char* text, size_t buffer size, shabuffer); //returns pointer to hash

    //socket()
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    //setTimeout to 10 sec
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    tolen = sizeof(struct sockaddr_in);
    bind(fd, (struct sockaddr*) &from, sizeof(from));
    // REQUEST_T empfangen
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

    // HEADER_T senden (HEADER_T, unsigned short namelength, string filename, unsigned integer filesize)
    printf("sending header\n");
    sendto(fd, header, sizeof(header), 0, (struct sockaddr*)&to, tolen);

    // DATA_T senden (DATA_T, unsigned integer SeqNmbr(0-n), Daten)
    //while(/*Not all Data sent*/)
    //{
    //    sendto(fd, /*Datagram*/, /*sizeof(Datagram)*/, 0, (struct sockaddr*)&to, tolen);
    //}

    // SHA512_T senden (SHA512_T, SHA-512-Hashwert(64Bytes))
    //sendto(fd, shabuffer, sizeof(shabuffer), 0, (struct sockaddr*) &to, tolen);

    // SHA512_CMP_T empfangen (SHA512_CMP_T, Vergleichsergebniss(1Bytes))
    //recvfrom(fd, recvSha512, sizeof(recvSha512), 0, (struct sockaddr*) &to, &tolen);

    //close socket
    close(fd);
}
