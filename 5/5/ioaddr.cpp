/*
 * Data:                2019-05-09
 * Autor:               Marcin Kurdziel
 * Kompilacja:          $ gcc ioarp.c -o ioarp
 * Uruchamianie:        $ ./ioarp <adres IPv4> <adres MAC>
 */

#include <cstdio>
#include <cstdlib>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h> /* inet_pton() */
#include <net/if_arp.h>
#include <netinet/in.h> /* struct sockaddr_in */
#include <sys/ioctl.h>
#include <net/if.h>

int main(int argc, char **argv)
{
    if(argc < 3)
    {
        fprintf(stderr, "Invocation: %s <INTERFACE> add <IPv4> <mask>\n", argv[0]);
        fprintf(stderr, "Invocation: %s <INTERFACE> down\n", argv[0]);
        return 1;
    }
    if(argv[3] == "add")
    {
        if(argc != 5)
        {
            fprintf(stderr, "Invocation: %s <INTERFACE> add <IPv4> <mask>\n", argv[0]);
            fprintf(stderr, "Invocation: %s <INTERFACE> down\n", argv[0]);
            return 1;
        }

        
    }
    else if(argv[3] == "down")
    {
        if(argc != 3)
        {
            fprintf(stderr, "Invocation: %s <INTERFACE> add <IPv4> <mask>\n", argv[0]);
            fprintf(stderr, "Invocation: %s <INTERFACE> down\n", argv[0]);
            return 1;
        }


    }
    else
    {
        fprintf(stderr, "Invocation: %s <INTERFACE> add <IPv4> <mask>\n", argv[0]);
        fprintf(stderr, "Invocation: %s <INTERFACE> down\n", argv[0]);
        return 1;
    }
    
}