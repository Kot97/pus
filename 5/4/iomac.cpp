/*
 * Data:                2019-05-09
 * Autor:               Marcin Kurdziel
 * Kompilacja:          $ clang++ -o iomac iomac.cpp
 * Uruchamianie:        $ ./iomac <nazwa interfejsu> <adres MAC> <MTU>
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

int main(int argc, char** argv)
{
    int sockfd, retval;
    ifreq req, print;

    if (argc != 4) 
    {
        fprintf(stderr, "Invocation: %s <INTERFACE> <MAC ADDRESS> <MTU>\n", argv[0]);
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) 
    {
        perror("socket()");
        return 1;
    }

    memset(&req, 0, sizeof(ifreq));
    memset(&print, 0, sizeof(ifreq));

    strcpy(print.ifr_ifrn.ifrn_name, argv[1]);
    retval = ioctl(sockfd, SIOCGIFMTU, &print);
    if (retval == -1)
    {
        perror("ioctl()");
        return 1;
    }

    fprintf(stdout, "Before\n");
    fprintf(stdout, "MTU: %d\n", print.ifr_ifru.ifru_mtu);

    retval = ioctl(sockfd, SIOCGIFHWADDR, &print);
    if (retval == -1)
    {
        perror("1 ioctl()");
        return 1;
    }

    fprintf(stdout, "Hwaddr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                static_cast<unsigned char>(print.ifr_ifru.ifru_hwaddr.sa_data[0]),
                static_cast<unsigned char>(print.ifr_ifru.ifru_hwaddr.sa_data[1]),
                static_cast<unsigned char>(print.ifr_ifru.ifru_hwaddr.sa_data[2]),
                static_cast<unsigned char>(print.ifr_ifru.ifru_hwaddr.sa_data[3]),
                static_cast<unsigned char>(print.ifr_ifru.ifru_hwaddr.sa_data[4]),
                static_cast<unsigned char>(print.ifr_ifru.ifru_hwaddr.sa_data[5]));

    // Adres MAC
    req.ifr_ifru.ifru_hwaddr.sa_family = ARPHRD_ETHER; //PERR

    // Interface
    strcpy(req.ifr_ifrn.ifrn_name, argv[1]);

    // Mtu
    req.ifr_ifru.ifru_mtu = std::stoi(argv[3]);

    retval = ioctl(sockfd, SIOCSIFMTU, &req);
    if (retval == -1)
    {
        perror("2 ioctl()");
        return 1;
    }

    req.ifr_ifru.ifru_hwaddr.sa_family = ARPHRD_ETHER; //PERR

    retval = sscanf(
                 argv[2], "%2x:%2x:%2x:%2x:%2x:%2x\n",
                 (unsigned int*)&req.ifr_ifru.ifru_hwaddr.sa_data[0],
                 (unsigned int*)&req.ifr_ifru.ifru_hwaddr.sa_data[1],
                 (unsigned int*)&req.ifr_ifru.ifru_hwaddr.sa_data[2],
                 (unsigned int*)&req.ifr_ifru.ifru_hwaddr.sa_data[3],
                 (unsigned int*)&req.ifr_ifru.ifru_hwaddr.sa_data[4],
                 (unsigned int*)&req.ifr_ifru.ifru_hwaddr.sa_data[5]
             );

    if (retval != 6) 
    {
        fprintf(stderr, "Invalid address format!\n");
        return 1;
    }

    retval = ioctl(sockfd, SIOCSIFHWADDR, &req);
    if (retval == -1)
    {
        perror("3 ioctl()");
        return 1;
    }

    retval = ioctl(sockfd, SIOCGIFMTU, &print);
    if (retval == -1)
    {
        perror("4 ioctl()");
        return 1;
    }

    fprintf(stdout, "After\n");
    fprintf(stdout, "MTU: %d\n", print.ifr_ifru.ifru_mtu);

    retval = ioctl(sockfd, SIOCGIFHWADDR, &print);
    if (retval == -1)
    {
        perror("5 ioctl()");
        return 1;
    }

    fprintf(stdout, "Hwaddr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                 static_cast<unsigned char>(print.ifr_ifru.ifru_hwaddr.sa_data[0]),
                 static_cast<unsigned char>(print.ifr_ifru.ifru_hwaddr.sa_data[1]),
                 static_cast<unsigned char>(print.ifr_ifru.ifru_hwaddr.sa_data[2]),
                 static_cast<unsigned char>(print.ifr_ifru.ifru_hwaddr.sa_data[3]),
                 static_cast<unsigned char>(print.ifr_ifru.ifru_hwaddr.sa_data[4]),
                 static_cast<unsigned char>(print.ifr_ifru.ifru_hwaddr.sa_data[5]));

    close(sockfd);
    return 0;
}
