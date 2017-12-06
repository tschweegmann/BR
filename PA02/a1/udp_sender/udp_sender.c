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

int main(int argc, char** argv)
{
    char* adress;
    int port;

    int fd;
    struct sockaddr_in from;
    struct sockaddr_in to;

    void* request[8];
    adress = argv[1];
    port = atoi(argv[2]);
    int tolen;
    struct timeval tv;

    // Header Zeug
    unsigned short namelength;
    char* filename = basename(argv[2]);
    printf("%s", filename);
    unsigned int filesize;
    char* header = malloc(sizeof(HEADER_T) + sizeof(namelength) + sizeof(filename) + sizeof(filesize));
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
    tolen = sizeof(to);

    // REQUEST_T empfangen
    recvfrom(fd, request, sizeof(request), 0, (struct sockaddr*)&to, &tolen);
    if (*(unsigned char*)request == REQUEST_T)
    {
        printf("received Request!\n");
    }

    // HEADER_T senden (HEADER_T, unsigned short namelength, string filename, unsigned integer filesize)
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
