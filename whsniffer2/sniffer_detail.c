#include <pcap.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 
#include <signal.h>
#include <time.h>
#include <sys/socket.h> 
#include <unistd.h> 
#include <sys/types.h>  
#include <fcntl.h>

#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/in.h>  
#include <netinet/if_ether.h>  
#include <netinet/ip.h>  
#include <netinet/udp.h>  
#include <netinet/tcp.h>  
#include <netinet/ip_icmp.h>
 
#define max 1024

unsigned  long long flowtotal,packetnums;
bpf_u_int32 netp,maskp;

void callPacket(u_char *arg,const struct pcap_pkthdr* pack,const u_char *content)  
{  
    int m=0,n;  
    const u_char *buf,*iphead;  
    u_char *p;  
    struct ether_header *ethernet;  
    struct iphdr *ip;  
    struct tcphdr *tcp;  
    struct udphdr *udp;  
    struct icmphdr *icmp;
    printf("==================================================\n");  
    // printf("The Frame is \n");  
    // int i;  
    // for(i=0; i<pack->len; ++i) 
    // {  
    //   printf(" %02x", content[i]);  
    //   if( (i + 1) % 16 == 0 )  
    //   {  
    //     printf("\n");  
    //   }  
    // }  
    // printf("\n");
    printf("Grabbed packet of length %d\n",pack->len);  
    printf("Recieved at ..... %s",ctime((const time_t*)&(pack->ts.tv_sec)));   
    printf("Ethernet address length is %d\n",ETHER_HDR_LEN);  
  
    ethernet=(struct ether_header *)content;  
    p=ethernet->ether_dhost;  
    n=ETHER_ADDR_LEN;  
    printf("Dest MAC is:");  
    do{  
        printf("%02x:",*p++);  
    }while(--n>0);  
    printf("\n");  
    p=ethernet->ether_shost;  
    n=ETHER_ADDR_LEN;  
    printf("Source MAC is:");  
    do{  
        printf("%02x:",*p++);  
    }while(--n>0);  
    printf("\n");  
      
    if(ntohs(ethernet->ether_type)==ETHERTYPE_IP)  
    {  
        printf("It's a IP packet\n");  
        ip=(struct iphdr*)(content+14);
        flowtotal += ntohs(ip->tot_len);
        printf("IP Version:%d\n",ip->version);  
        printf("TTL:%d\n",ip->ttl);

        char sip[20],dip[20];
        struct in_addr addr;
        addr.s_addr = ip->saddr;
        strcpy(sip,inet_ntoa(addr));
        printf("Source address:%s\n", sip);
        addr.s_addr = ip->daddr;
        strcpy(dip,inet_ntoa(addr));
        printf("Destination address:%s\n",dip);
        printf("key: %x %x --> %x\n", netp, ip->saddr & maskp , ip->daddr & maskp);

        printf("Protocol:%d\n",ip->protocol);
        switch(ip->protocol)
        {
            case 6:  
                printf("The Transport Layer Protocol is TCP\n");  
                tcp=(struct tcphdr*)(content+14+20);  
                printf("Source Port:%d\n",ntohs(tcp->source));  
                printf("Destination Port:%d\n",ntohs(tcp->dest));  
                printf("Sequence Number:%u\n",ntohl(tcp->ack_seq));  
                break;  
            case 17:  
                printf("The Transport Layer Protocol is UDP\n");  
                udp=(struct udphdr*)(content+14+20);  
                printf("Source port:%d\n",ntohs(udp->source));  
                printf("Destination port:%d\n",ntohs(udp->dest));  
                break;  
            case 1:  
                printf("The Transport Layer Protocol is ICMP\n");  
                icmp=(struct icmphdr*)(content+14+20);  
                printf("ICMP Type:%d\n", icmp->type);  
                switch(icmp->type)  
                {
                    case 8:  
                        printf("ICMP Echo Request Protocol\n");  
                        break;  
                    case 0:  
                        printf("ICMP Echo Reply Protocol\n");  
                        break;  
                    default:  
                        break;  
                }
                break;  
            default:  
                break;  
        }
        // printf("iphead: %x\n", *iphead);
        // if(*iphead==0x48) 
        // { 
        //     printf("Source ip :%d.%d.%d.%d\n",iphead[12],iphead[13],iphead[14],iphead[15]); 
        //     printf("Dest ip :%d.%d.%d.%d\n",iphead[16],iphead[17],iphead[18],iphead[19]); 
        // }
        // tcp= (struct tcp_header*)(iphead);  
        // source_port = ntohs(tcp->tcp_source_port);  
        // dest_port = ntohs(tcp->tcp_destination_port);  
    }  
    else if(ntohs (ethernet->ether_type) == ETHERTYPE_ARP)  
    { 
        printf("This is ARP packet.\n");  
        iphead=content+14; 
        if (*(iphead+2)==0x08)  
        {
            printf("Source ip:\t %d.%d.%d.%d\n",iphead[14],iphead[15],iphead[16],iphead[17]);  
            printf("Dest ip:\t %d.%d.%d.%d\n",iphead[24],iphead[25],iphead[26],iphead[27]);  
            printf("ARP TYPE: %d (0:request;1:respond)\n",iphead[6]);  
        }
    }
    packetnums++; 
}

void handler(){
  printf("flowtotal : %lld\n",flowtotal);
  printf("packetnums: %lld\n",packetnums);
  printf("------------------------------------\n");
  signal(SIGALRM, handler);
  alarm(1);
}

int main(){
  flowtotal,packetnums = 0;

  char *devname = "eno16777736";
  //char *devname = "eth2";
  char errBuf[PCAP_ERRBUF_SIZE];
  pcap_t * device = pcap_open_live(devname, 65535, 1, 0, errBuf);

  char net[20],mask[20];
  struct in_addr addr;
  int ret;
  ret = pcap_lookupnet(devname, &netp, &maskp, errBuf);
  addr.s_addr = netp;
  strcpy(net,inet_ntoa(addr));
  addr.s_addr = maskp;
  strcpy(mask,inet_ntoa(addr));
  printf("net %s ,mask %s\n",net,mask);
  printf("------------------------------------\n");

  handler();
  pcap_loop(device, -1, callPacket, NULL);
  pcap_close(device);
  return 0;
}