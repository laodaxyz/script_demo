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

#define HASHSIZE 997
typedef struct node{
    unsigned long long ip;
    unsigned long long bytes;
    unsigned long long packets;
    unsigned long long tcp;
    unsigned long long udp;
    unsigned long long icmp;
    unsigned long long other;
    struct node *next;
}htnode;
typedef htnode *ht[HASHSIZE];
unsigned  long long flowtotal,packetnums;
bpf_u_int32 netp,maskp;
ht htt;

int hash(unsigned long long key);
htnode * hashtable_search(ht T, unsigned long long key);
int hashtable_insert(ht T, htnode *s);
void hashtable_create(ht h);
void callPacket(u_char *arg,const struct pcap_pkthdr* pack,const u_char *content);
void handler();

int main(){
  flowtotal,packetnums = 0;

  char errBuf[PCAP_ERRBUF_SIZE], * devname;
  devname = pcap_lookupdev(errBuf);
  //devname = "eth2";
  printf("devname:%s---\n", devname);
  
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
  hashtable_create(htt);
  pcap_loop(device, -1, callPacket, NULL);
  pcap_close(device);
  return 0;
}

int hash(unsigned long long key ) {
  return key % HASHSIZE;
}

htnode * hashtable_search(ht T, unsigned long long key){
  htnode *p=T[hash(key)];
  while(p!=NULL && p->ip!=key)
    p=p->next;
  return p;
}

int hashtable_insert(ht T, htnode *s) {
  int d;
  htnode *p=hashtable_search(T,s->ip);
  if(p!=NULL){
    p->bytes += s->bytes;
    p->packets += s->packets;
  }else{
    d=hash(s->ip);
    s->next = T[d];
    T[d]=s;
  }
}

void hashtable_create(ht h){
  int i;
  for (i = 0; i < HASHSIZE; ++i)
    h[i] = NULL;
}

void callPacket(u_char *arg,const struct pcap_pkthdr* pack,const u_char *content)  
{
  htnode hv;  
  const u_char *iphead;  
  u_char *p;  
  struct ether_header *ethernet;  
  struct iphdr *ip;  
  struct tcphdr *tcp;  
  struct udphdr *udp;  
  struct icmphdr *icmp;

  ethernet=(struct ether_header *)content;
  if(ntohs(ethernet->ether_type)==ETHERTYPE_IP)  
  {  
    ip=(struct iphdr*)(content+14);
    int tot_len=ntohs(ip->tot_len);
    flowtotal += tot_len;
    hv.ip = ip->saddr;
    hv.bytes = tot_len;
    hv.packets = 1;
    //printf("key: %x %x --> %x\n", netp, ip->saddr & maskp , ip->daddr & maskp);
    switch(ip->protocol)
    {
      case 6:
        tcp=(struct tcphdr*)(content+14+20);
        hv.tcp = 1;
        break;
      case 17:
        udp=(struct udphdr*)(content+14+20);
        hv.udp = 1;
        break;
      case 1:
        icmp=(struct icmphdr*)(content+14+20);
        hv.icmp = 1;
        break;
      default:
        hv.other = 1;
        break;
    }
    hashtable_insert( htt, &hv);
  }else if(ntohs (ethernet->ether_type) == ETHERTYPE_ARP){
    iphead=content+14;
    flowtotal += ntohs(ip->tot_len);
  }
  packetnums++;
}

void handler(){
  printf("flowtotal : %lld\n",flowtotal);
  printf("packetnums: %lld\n",packetnums);
  printf("------------------------------------\n");
  flowtotal,packetnums=0;
  signal(SIGALRM, handler);
  alarm(1);
}