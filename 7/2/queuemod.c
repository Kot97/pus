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

void print_error(const char* msg)
{
    fprintf(stderr, msg + "() failed!\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
    struct nfq_handle h;
    struct nfq_q_handle q;
    int sockfd, ret;

    h = nfq_open();
    if(!h) print_error("nfq_open");

    if(nfq_unbind_pf(h, PF_INET) < 0) print_error("nfq_unbind_pf");
    if(nfq_bind_pf(h, PF_INET) < 0) print_error("nfq_bind_pf");

    q = nfq_create_queue(h, 5, &callback, NULL);
    if(!q) print_error("nfq_create_queue");
}
