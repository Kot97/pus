/*
 * Data:                2019-05-12
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang++ -o ioaddr ioaddr.cpp
 * Uruchamianie:        ./ioaddr <INTERFACE> add <IPv4> <mask>
 *                      ./ioaddr <INTERFACE> down
 */

#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h> /* inet_pton() */
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

    ifreq ifr;
    memset(&ifr, 0, sizeof(ifreq));
    strcpy(ifr.ifr_name, argv[1]);

    if(!std::strcmp(argv[2], "add"))
    {
        if(argc != 5)
        {
            fprintf(stderr, "Invocation: %s <INTERFACE> add <IPv4> <mask>\n", argv[0]);
            return 1;
        }

        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd == -1) 
        {
            perror("socket()");
            return 1;
        }

        ifreq check;
        memset(&check, 0, sizeof(ifreq));
        strcpy(check.ifr_name, argv[1]);
        if(ioctl(sockfd, SIOCGIFFLAGS, &check) == -1)
        {
            perror("check if interface is up");
            return 1;
        }
        if(!(check.ifr_flags & IFF_UP))
        {
            printf("interface isn't up\n");
            close(sockfd);
            return 1;
        }

        ifr.ifr_addr.sa_family = AF_INET;
        sockaddr_in *addr = (sockaddr_in*)&ifr.ifr_addr;

        if(inet_pton(AF_INET, argv[3], &addr->sin_addr) != 1)
        {
            perror("inet_pton() address");
            close(sockfd);
            return 1;
        }

        if(ioctl(sockfd, SIOCSIFADDR, &ifr) == -1)
        {
            perror("address add");
            close(sockfd);
            return 1;
        }

        if(inet_pton(AF_INET, argv[4], &addr->sin_addr) != 1)
        {
            perror("inet_pton() mask");
            close(sockfd);
            return 1;
        }

        if(ioctl(sockfd, SIOCSIFNETMASK, &ifr) == -1)
        {
            perror("mask set");
            close(sockfd);
            return 1;
        }

        close(sockfd);
    }
    else if(!std::strcmp(argv[2], "down"))
    {
        if(argc != 3)
        {
            fprintf(stderr, "Invocation: %s <INTERFACE> down\n", argv[0]);
            return 1;
        }

        int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd == -1) 
        {
            perror("socket()");
            return 1;
        }

        ifr.ifr_flags &= ~IFF_UP;
        if(ioctl(sockfd, SIOCSIFFLAGS, &ifr) == -1)
        {
            perror("address remove");
            close(sockfd);
            return 1;
        }

        close(sockfd);
    }
    else
    {
        fprintf(stderr, "Invocation: %s <INTERFACE> add <IPv4> <mask>\n", argv[0]);
        fprintf(stderr, "Invocation: %s <INTERFACE> down\n", argv[0]);
        return 1;
    }
}