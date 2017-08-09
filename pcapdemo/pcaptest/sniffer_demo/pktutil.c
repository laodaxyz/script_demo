#define _BSD_SOURCE 1

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <net/ethernet.h>
 
#ifdef LINUX
#include <netinet/ether.h>
#endif
 
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
 
#include <fcntl.h>
#include <getopt.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <pcap.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

u_char* ip_handler (u_char *args,const struct pcap_pkthdr* pkthdr,
                                         const u_char* packet)
{
    const struct nread_ip* ip;   /* packet structure         */
    const struct nread_tcp* tcp; /* tcp structure            */
    u_int length = pkthdr->len;  /* packet header length  */
    u_int off, version;             /* offset, version       */
    int len;                        /* length holder         */
    ip = (struct nread_ip*)(packet + sizeof(struct ether_header));
    length -= sizeof(struct ether_header);
    tcp = (struct nread_tcp*)(packet + sizeof(struct ether_header) +
                                            sizeof(struct nread_ip));

    len     = ntohs(ip->ip_len); /* get packer length */
    version = IP_V(ip);          /* get ip version    */

    off = ntohs(ip->ip_off);

    fprintf(stdout,"ip: ");
    fprintf(stdout,"%s:%u->%s:%u ",
                    inet_ntoa(ip->ip_src), tcp->th_sport,
                    inet_ntoa(ip->ip_dst), tcp->th_dport);
    fprintf(stdout,
            "tos %u len %u off %u ttl %u prot %u cksum %u ",
                    ip->ip_tos, len, off, ip->ip_ttl,
                    ip->ip_p, ip->ip_sum);
    fprintf(stdout,"seq %u ack %u win %u ",
                    tcp->th_seq, tcp->th_ack, tcp->th_win);
    fprintf(stdout,"%s", payload);
    printf("\n");
    return NULL;
}

u_int16_t ethernet_handler (u_char *args, const struct pcap_pkthdr* pkthdr,
                                          const u_char* packet)

{
    u_int caplen = pkthdr->caplen; /* length of portion present from bpf  */
    u_int length = pkthdr->len;    /* length of this packet off the wire  */
    struct ether_header *eptr;     /* net/ethernet.h                      */
    u_short ether_type;            /* the type of packet (we return this) */
    eptr = (struct ether_header *) packet;
    ether_type = ntohs(eptr->ether_type);
    fprintf(stdout,"eth: ");
    fprintf(stdout,
    "%s ",ether_ntoa((struct ether_addr*)eptr->ether_shost));
    fprintf(stdout,
    "%s ",ether_ntoa((struct ether_addr*)eptr->ether_dhost));

    if (ether_type == ETHERTYPE_IP) {
            fprintf(stdout,"(ip)");
    } else  if (ether_type == ETHERTYPE_ARP) {
            fprintf(stdout,"(arp)");
    } else  if (eptr-ether_type == ETHERTYPE_REVARP) {
            fprintf(stdout,"(rarp)");
    } else {
            fprintf(stdout,"(?)");
    }
    fprintf(stdout," %d\n",length); /* print len */
    return ether_type;
}

void pcap_callback(u_char *args, const struct pcap_pkthdr* pkthdr,
                                         const u_char* packet)
{
    u_int16_t type = ethernet_handler(args, pkthdr, packet);

    if (type == ETHERTYPE_IP) {
            ip_handler(args, pkthdr, packet);
     } else if (type == ETHERTYPE_ARP) {
            /* noop */
    } else if (type == ETHERTYPE_REVARP) {
            /* noop */
    }
}

int main(void)
{
    struct bpf_program filter;     /* The compiled filter       */
    pcap_t *descr;                 /* Session descr             */
    char *dev;                     /* The device to sniff on    */
    char errbuf[PCAP_ERRBUF_SIZE]; /* Error string              */
    struct bpf_program filter;     /* The compiled filter       */
    bpf_u_int32 mask;              /* Our netmask               */
    bpf_u_int32 net;               /* Our IP address            */
    u_char* args = NULL;           /* Retval for pcacp callback */

    dev = pcap_lookupdev(errbuf);

    if (dev == NULL) {
        printf("%s\n", errbuf);
        return (1);
    }

    descr = pcap_open_live(dev, BUFSIZ, 1, 0, errbuf);

    if (descr == NULL) {
        printf("pcap_open_live(): %s\n", errbuf);
        return (1);
    }
    pcap_lookupnet(dev, &net, &mask, errbuf);

     if (pcap_compile(descr, &filter, " ", 0, net) == -1) {
        fprintf(stderr,"Error compiling pcap\n");
        return (1);
    }

    if (pcap_setfilter(descr, &filter))  {
        fprintf(stderr, "Error setting pcap filter\n");
        return (1);
    }

    pcap_loop(descr, -1, pcap_callback, args);
}