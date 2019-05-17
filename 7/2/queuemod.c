/*
 * Data:                2019-05-17
 * Autor:               Marcin Kurdziel
 * Kompilacja:          clang -o queuemod queuemod.c -lnetfilter_queue
 * Uruchamianie:        ./queuemod
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <net/if.h> /* if_indextoname() */
#include <linux/types.h>
#include <linux/netfilter.h> /* NF_ACCEPT */
#include <limits.h> /* Wymagane przez <linux/netfilter_ipv4.h> (MIN_INT, ...) */
#include <linux/netfilter_ipv4.h> /* Nazwy punktow zaczepienia Netfilter. */
#include <libnetfilter_queue/libnetfilter_queue.h>

#include <memory.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip.h>
#include "libqueue.h"

void print_error(const char* msg)
{
    fprintf(stderr, msg);
    exit(EXIT_FAILURE);
}

static u_int32_t print_packet(struct nfq_data *tb);

static int callback(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data)
{
    u_int32_t id = print_packet(nfa);

    unsigned char *data_;
    int len = nfq_get_payload(nfa, &data_);
    if(len == -1) print_error("nfq_get_payload error\n");

    struct iphdr *ipheader = (struct iphdr*)data_;
    if(ipheader->protocol != IPPROTO_ICMP) print_error("there is no icmp header\n");

    // struct icmphdr *icmpheader = (struct icmphdr*)(data_ + sizeof(*ipheader));

    // iphdr: data_, ..., data_ + sizeof(iphdr) - 1
    // icmphdr: data_ + sizeof(iphdr), ..., data_ + sizeof(iphdr) + sizeof(icmphdr) - 1
    // icmp payload: data_ + sizeof(iphdr) + sizeof(icmphdr), ..., data_ + len- 1 ??? PERR

    swap_bytes(data_ + sizeof(*ipheader) + sizeof(struct icmphdr), len - sizeof(*ipheader) - sizeof(struct icmphdr));
    memset(data_ + sizeof(*ipheader) + 2*sizeof(uint8_t), 0, sizeof(uint16_t)); // set checksum to 0

    uint16_t sum = internet_checksum((unsigned short *)(data_ + sizeof(*ipheader)), len - sizeof(*ipheader));
    memcpy(data_ + sizeof(*ipheader) + 2*sizeof(uint8_t), &sum, sizeof(sum)); // set checksum

    fprintf(stdout, "Leaving callback.\n\n");
    return nfq_set_verdict(qh, id, NF_ACCEPT, len, data_);
}

int main(int argc, char** argv)
{
    struct nfq_handle *h;
    struct nfq_q_handle *qh;
    int sockfd, ret;
    char buf[4096] __attribute__((aligned));

    h = nfq_open();
    if(!h) print_error("nfq_open error\n");

    if(nfq_unbind_pf(h, PF_INET) < 0) print_error("nfq_unbind_pf error\n");
    if(nfq_bind_pf(h, PF_INET) < 0) print_error("nfq_bind_pf error\n");

    qh = nfq_create_queue(h, 5, &callback, NULL);
    if(!qh) print_error("nfq_create_queue error\n");

    if(nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) print_error("nfq_set_mode error\n");

    fprintf(stdout, "Waiting for packets...\n");
    sockfd = nfq_fd(h);

    while((ret = recv(sockfd, buf, 4096, 0)) >= 0)
    {
        fprintf(stdout, "Packet received. Entering callback.\n");
        nfq_handle_packet(h, buf, ret);
        sleep(1);
    }

    nfq_destroy_queue(qh);
    nfq_close(h);

    exit(EXIT_SUCCESS);
}

static u_int32_t print_packet(struct nfq_data *tb) 
{

    int                             id = 0; /* ID pakietu. */
    struct nfqnl_msg_packet_hdr     *ph; /* Naglowek meta-danych. */
    struct nfqnl_msg_packet_hw      *hwph; /* Adres warstwy lacza anych. */
    u_int32_t                       ifi; /* Indeks interfejsu. */
    int                             retval; /* Wartosc zwracana. */
    unsigned char                   *data; /* Bufor na payload. */
    char                            *hook; /* Punkt zaczepienia Netf. */
    char                            ifname[IF_NAMESIZE]; /* Nazwa interf. */

    /* Naglowek z meta-danymi: */
    ph = nfq_get_msg_packet_hdr(tb);
    if (ph) 
    {
        /* ID pakietu w porzadku sieciowym: */
        id = ntohl(ph->packet_id);
        /* Wartosc pola EtherType/Type ramki Ethernet: */
        fprintf(stdout, "ID: '%u', EtherType: '0x%04x', ",
                id, ntohs(ph->hw_protocol));

        fprintf(stdout, "Netfilter Hook: ");

        switch (ph->hook) 
        {
        case NF_IP_PRE_ROUTING:
            hook = "PREROUTING";
            break;
        case NF_IP_LOCAL_IN:
            hook = "INPUT";
            break;
        case NF_IP_FORWARD:
            hook = "FORWARD";
            break;
        case NF_IP_LOCAL_OUT:
            hook = "OUTPUT";
            break;
        case NF_IP_POST_ROUTING:
            hook = "POSTROUTING";
            break;
        default:
            hook = "UNKNOWN";
        }

        fprintf(stdout, "'%s',\n", hook);

    }

    /* Pobranie adresu warstwy lacza danych (zrodlowego): */
    hwph = nfq_get_packet_hw(tb);
    if (hwph) 
    {
        int i, hlen = ntohs(hwph->hw_addrlen);

        fprintf(stdout, "Source Address: '");
        for (i = 0; i < hlen-1; i++)
            fprintf(stdout, "%02x:", hwph->hw_addr[i]);
        fprintf(stdout, "%02x', ", hwph->hw_addr[hlen-1]);
    }


    /* Pobranie indeksu interfejsu wejsciowego: */
    ifi = nfq_get_indev(tb);
    if (ifi) 
    {
        /* Konwersja indeksu na nazwe interfejsu: */
        if (if_indextoname(ifi, ifname) == NULL) {
            perror("if_indextoname()");
            exit(EXIT_FAILURE);
        }
        fprintf(stdout, "Input Dev: '%s', ", ifname);
    }

    /*
     * Pobranie indeksu interfejsu wyjsciowego
     * (na danym etapie moze nie byc znany):
     */
    ifi = nfq_get_outdev(tb);
    if (ifi) 
    {

        if (if_indextoname(ifi, ifname) == NULL) 
        {
            perror("if_indextoname()");
            exit(EXIT_FAILURE);
        }
        fprintf(stdout, "Output Dev: '%s', ", ifname);
    }


    retval = nfq_get_payload(tb, &data);
    if (retval >= 0) fprintf(stdout, "\nPayload Length: '%d'\n", retval);

    return id;
}
