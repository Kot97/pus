/*
 * Data:                2009-02-27
 * Autor:               Jakub Gasior <quebes@mars.iti.pk.edu.pl>
 * Kompilacja:          $ gcc udp.c -o udp
 * Uruchamianie:        $ ./udp <adres IP lub nazwa domenowa> <numer portu>
 */

#include <iostream>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/generic/datagram_protocol.hpp>
#include <boost/asio/io_context.hpp>
#include "checksum.h"

#define SOURCE_PORT 5050
#define SOURCE_ADDRESS "192.0.2.1"

namespace asio = boost::asio;
namespace ip = asio::ip;
using boost::system::error_code;

void error(const std::string& what, error_code err)
{
    if(err == asio::error::operation_aborted) return;

    std::cerr << what << ": " << err.message() << std::endl;
}

int main(int argc, char** argv) 
{
    /* SPrawdzenie argumentow wywolania: */
    if (argc != 3) 
    {
        std::cerr << "Invocation: " << argv[0] << " <HOSTNAME OR IP ADDRESS> <PORT>\n" << std::endl;
        
        return 1;
    }

    asio::io_context context;
    error_code err;

    ip::udp::resolver resolver(context);
    ip::udp::endpoint endpoint;
    ip::udp::socket socket(context);

    auto result = resolver.resolve(argv[1], argv[2], err);

    if(err)
    {
        error("resolve", err);
        return 1;
    }

    for(auto&& i : result)
        endpoint = i.endpoint();

    asio::generic::datagram_protocol datagram(endpoint.protocol().family(), endpoint.protocol().protocol());
    asio::generic::datagram_protocol::socket socket(context, datagram.family(), datagram.protocol());

    asio::generic::datagram_protocol::socket::protocol_type protocol(datagram.protocol());
    

    /* Przechodzimy kolejno przez elementy listy: */
    for (rp = result; rp != NULL; rp = rp->ai_next) {

        /* Utworzenie gniazda dla protokolu UDP: */
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) {
            perror("socket()");
            continue;
        }

        /* Ustawienie opcji IP_HDRINCL: */
        retval = setsockopt(
                     sockfd,
                     IPPROTO_IP, IP_HDRINCL,
                     &socket_option, sizeof(int)
                 );

        if (retval == -1) {
            perror("setsockopt()");
            exit(EXIT_FAILURE);
        } else {
            /* Jezeli gniazdo zostalo poprawnie utworzone i
             * opcja IP_HDRINCL ustawiona: */
            break;
        }
    }

    /* Jezeli lista jest pusta (nie utworzono gniazda): */
    if (rp == NULL) {
        fprintf(stderr, "Client failure: could not create socket.\n");
        exit(EXIT_FAILURE);
    }

    /********************************/
    /* Wypelnienie pol naglowka IP: */
    /********************************/
    ip_header->ip_hl                =       5; /* 5 * 32 bity = 20 bajtow */
    ip_header->ip_v                 =       4; /* Wersja protokolu (IPv4). */
    ip_header->ip_tos               =       0; /* Pole TOS wyzerowane. */

    /* Dlugosc (naglowek IP + dane). Wg MAN: "Always filled in": */
    ip_header->ip_len               =       sizeof(struct ip)
                                            + sizeof(struct udphdr);

    /* Wypelniane przez jadro systemu, jezeli podana wartosc to zero: */
    ip_header->ip_id                =       0; /* Pole Identification. */

    ip_header->ip_off               =       0; /* Pole Fragment Offset. */
    ip_header->ip_ttl               =       255; /* TTL */

    /* Identyfikator enkapsulowanego protokolu: */
    ip_header->ip_p                 =       IPPROTO_UDP;

    /* Adres zrodlowy ("Filled in when zero"): */
    ip_header->ip_src.s_addr        =       inet_addr(SOURCE_ADDRESS);

    /* Adres docelowy (z argumentu wywolania programu): */
    ip_header->ip_dst.s_addr        = ((struct sockaddr_in*)rp->ai_addr)
                                      ->sin_addr.s_addr;

    /* Suma kontrolna naglowka IP - "Always filled in":
     *
     * ip_header->ip_sum            =       internet_checksum(
     *                                              (unsigned short *)ip_header,
     *                                              sizeof(struct ip)
     *                                              );
     */

    /*********************************/
    /* Wypelnienie pol naglowka UDP: */
    /*********************************/

    /* Port zrodlowy: */
    udp_header->uh_sport            =       htons(SOURCE_PORT);
    /* Port docelowy (z argumentu wywolania): */
    udp_header->uh_dport            =       htons(atoi(argv[2]));

    /* Rozmiar naglowka UDP i danych. W tym przypadku tylko naglowka: */
    udp_header->uh_ulen             =       htons(sizeof(struct udphdr));

    /************************************/
    /* Wypelnienie pol pseudo-naglowka: */
    /************************************/

    /* Zrodlowy adres IP: */
    pseudo_header->ip_src.s_addr    =       ip_header->ip_src.s_addr;
    /* Docelowy adres IP: */
    pseudo_header->ip_dst.s_addr    =       ip_header->ip_dst.s_addr;
    /* Pole wyzerowane: */
    pseudo_header->unused           =       0;
    /* Identyfikator enkapsulowanego protokolu: */
    pseudo_header->protocol         =       ip_header->ip_p;
    /* Rozmiar naglowka UDP i danych: */
    pseudo_header->length           =       udp_header->uh_ulen;
    /* Obliczenie sumy kontrolnej na podstawie naglowka UDP i pseudo-naglowka: */
    udp_header->uh_sum              =       0;
    checksum                        =       internet_checksum(
                                                (unsigned short *)udp_header,
                                                sizeof(struct udphdr)
                                                + sizeof(struct phdr)
                                            );

    udp_header->uh_sum              = (checksum == 0) ? 0xffff : checksum;

    fprintf(stdout, "Sending UDP...\n");

    /* Wysylanie datagramow co 1 sekunde: */
    for (;;) {

        /*
         * Prosze zauwazyc, ze pseudo-naglowek nie jest wysylany
         * (ale jest umieszczony w buforze za naglowkiem UDP dla wygodnego
         * obliczania sumy kontrolnej):
         */
        retval = sendto(
                     sockfd,
                     datagram, ip_header->ip_len,
                     0,
                     rp->ai_addr, rp->ai_addrlen
                 );

        if (retval == -1) {
            perror("sendto()");
        }

        sleep(1);
    }

    exit(EXIT_SUCCESS);
}
