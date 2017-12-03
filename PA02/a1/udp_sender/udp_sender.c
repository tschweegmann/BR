#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <uinstd.h>
#include "Aufgabe2.h"

int main(int argc, char** argv[])
{
    char* adress;
    int port;
    int fd;
    struct sockaddr_in from;
    struct sockaddr_in to;

    adress = argv[1];
    port = atoi(argv[2]);

    //socket()

    // REQUEST_T empfangen

    // HEADER_T senden (HEADER_T, unsigned short namelength, string filename, unsigned integer filesize)

    // DATA_T senden (DATA_T, unsigned integer SeqNmbr(0-n), Daten)

    // SHA512_T senden (SHA512_T, SHA-512-HAshwert(64Bytes))

    // SHA512_CMP_T empfangen (SHA512_CMP_T, Vergleichsergebniss(1Bytes))

    //close socket
}
