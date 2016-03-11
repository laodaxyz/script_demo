#include <pcap.h>
#include <stdlib.h>
#include <malloc.h>
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

#define HASHSIZE 10000
typedef struct node{
  u_int32_t ip;
  unsigned long long bytes;
  unsigned long long packets;
  unsigned long long tcp;
  unsigned long long udp;
  unsigned long long icmp;
  unsigned long long arp;
  unsigned long long other;
  struct node *next;
}htnode;
typedef htnode **hashtable;
unsigned  long long flowtotal,packetnums;
bpf_u_int32 netp,maskp;
hashtable ht,ht_in;

typedef struct value{
  u_int32_t sip;
  unsigned long long packets;
  unsigned long long tcp;
  unsigned long long udp;
  unsigned long long icmp;
  unsigned long long other;
  unsigned long long bytes;
}value;

typedef struct{
  value v;
  unsigned long long fpacket;
  unsigned long long fbytes;
}xvalue;

void handler();
void callPacket(u_char *arg,const struct pcap_pkthdr* pack,const u_char *content);
int hash(unsigned long long key);
hashtable hashtable_init(int size);
void hashtable_descrty(hashtable h);
htnode * hashtable_search(hashtable T, unsigned long long key);
int hashtable_insert(hashtable T, htnode *s);

int main(){
  flowtotal,packetnums = 0;
  char errBuf[PCAP_ERRBUF_SIZE], *devname;
  devname = pcap_lookupdev(errBuf);
  //devname = "eth2";
  printf("devname:%s---\n", devname);
  
  pcap_t *device = pcap_open_live(devname, 65535, 1, 0, errBuf);
  char net[20],mask[20];
  struct in_addr addr;
  int ret;
  ret = pcap_lookupnet(devname, &netp, &maskp, errBuf);
  addr.s_addr = netp;
  strcpy(net,inet_ntoa(addr));
  addr.s_addr = maskp;
  strcpy(mask,inet_ntoa(addr));
  printf("net %s ,mask %s\n",net,mask);
  printf("The PID of this process is %d\n",getpid());
  printf("------------------------------------\n");

  signal(SIGALRM, handler);
  alarm(10);
  ht = hashtable_init(HASHSIZE);
  ht_in = hashtable_init(HASHSIZE);
  pcap_loop(device, -1, callPacket, NULL);
  pcap_close(device);
  return 0;
}

void handler(){
  printf("flowtotal : %lld\n",flowtotal);
  printf("packetnums: %lld\n",packetnums);
  signal(SIGALRM, handler);
  alarm(10);
  flowtotal,packetnums=0;
  hashtable oldht,oldht_in;
  oldht = ht;
  oldht_in = ht_in;
  ht = hashtable_init(HASHSIZE);
  ht_in = hashtable_init(HASHSIZE);
  hashtable_descrty(oldht);
  hashtable_descrty(oldht_in);
  printf("------------------------------------\n");
}

void callPacket(u_char *arg,const struct pcap_pkthdr* pack,const u_char *content)  
{
  htnode *hv;
  const u_char *iphead;
  u_char *p;
  struct ether_header *ethernet;
  struct iphdr *ip;
  struct tcphdr *tcp;
  struct udphdr *udp;
  struct icmphdr *icmp;
  struct ether_arp *arp;
  struct arphdr *arph;

  ethernet=(struct ether_header *)content;
  //printf("%x\n", ethernet->ether_type);
  if(ntohs(ethernet->ether_type)==ETHERTYPE_IP)  
  {
    ip=(struct iphdr*)(content+14);
    int tot_len=ntohs(ip->tot_len);
    flowtotal += tot_len;
    hv = (htnode *)malloc(sizeof(htnode));
    hv->bytes   = tot_len;
    hv->packets = 1;
    hv->tcp     = 0;
    hv->udp     = 0;
    hv->icmp    = 0;
    hv->arp     = 0;
    hv->other   = 0;
    switch(ip->protocol)
    {
      case 6:
        hv->tcp = 1;
        break;
      case 17:
        hv->udp = 1;
        break;
      case 1:
        hv->icmp = 1;
        break;
      default:
        hv->other = 1;
        break;
    }
    //neiwang out
    if ((ip->saddr & maskp) == netp && (ip->daddr & maskp) != netp){
      hv->ip = ip->saddr;
      hashtable_insert(ht, hv);
    }else{
      hv->ip = ip->saddr;
      hashtable_insert(ht_in, hv);
    }
    //printf("key: %x %x --> %x\n", netp, ip->saddr & maskp , ip->daddr & maskp);
  }
  else if(ntohs (ethernet->ether_type) == ETHERTYPE_ARP){
    // arp=(struct ether_arp*)(content+14);
    // hv = (htnode *)malloc(sizeof(htnode));
    // flowtotal  += 42;
    // hv->bytes   = 42;
    // hv->packets = 1;
    // hv->tcp     = 0;
    // hv->udp     = 0;
    // hv->icmp    = 0;
    // hv->arp     = 1;
    // hv->other   = 0;
    // struct in_addr addr;
    // addr.s_addr = arp->arp_spa;
    // printf("key: %s --> %d\n", inet_ntoa(addr), arp->arp_tpa);
    //neiwang out
    // if ((arp->arp_spa & maskp) == netp && (arp->arp_tpa & maskp) != netp){
    //   hv->ip = arp->arp_spa;
    //   hashtable_insert(ht, hv);
    // }else{
    //   hv->ip = arp->arp_spa;
    //   hashtable_insert(ht_in, hv);
    // }
  }
  packetnums++;
}

int hash(unsigned long long key ) {
  return key % HASHSIZE;
}

hashtable hashtable_init(int size){
  hashtable h;
  h = (htnode * *)malloc(sizeof(htnode *)*size);
  if(h == NULL) {
    printf("Out of memory!\n");
    exit(-1);
  }
  int i;
  for(i = 0; i < size; i++)
  {
    h[i] = (htnode *)malloc(sizeof(htnode));
    if(h[i] == NULL){
      printf("Out of memory!\n");
      exit(-1);
    }else{
      h[i]->ip = i;
      h[i]->next = NULL;
    }
  }
  return h;
}

void hashtable_descrty(hashtable h){
  int i;
  // value *v;
  // xvalue vs[1025];
  // int sock,j;
  // struct sockaddr_in svraddr;
  // if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){ exit(1); }
  // svraddr.sin_family = AF_INET;
  // svraddr.sin_port = htons(4100);
  // if(inet_pton(AF_INET, "127.0.0.1", &svraddr.sin_addr) < 0){ exit(1); }
  // if(connect(sock, (const struct sockaddr *)&svraddr, sizeof(svraddr)) < 0){ close(sock);return; }
  // memset(&vs[0], 0, sizeof(xvalue));
  // vs[0].v.other = 0;
  // vs[0].fbytes  = flowtotal;
  // vs[0].fpacket = packetnums;

  for (i = 0; i < HASHSIZE; i++)
  {
    htnode *p;
    p = h[i];
    while(p->next != NULL){
      h[i] = p->next;
      // vs[j].v.sip     = p->ip;
      // vs[j].v.tcp     = p->tcp;
      // vs[j].v.udp     = p->udp;
      // vs[j].v.icmp    = p->icmp;
      // vs[j].v.other   = p->other;
      // vs[j].v.bytes   = p->bytes;
      // vs[j].v.packets = p->packets;
      // vs[j].fbytes    = p->bytes;
      // vs[j].fpacket   = p->packets;
      // j++;
      free(p);
      p=h[i];
    }
    free(h[i]);
  }
  free(h);
  // write(sock, vs, sizeof(xvalue) * j);
  // close(sock);
}

htnode * hashtable_search(hashtable T, unsigned long long key){
  htnode *p=T[hash(key)];
  while(p!=NULL && p->ip!=key)
    p=p->next;
  return p;
}

int hashtable_insert(hashtable T, htnode *s) {
  int d;
  htnode *p=hashtable_search(T,s->ip);
  if(p!=NULL){
    p->bytes   += s->bytes;
    p->packets += s->packets;
    p->tcp     += s->tcp;
    p->udp     += s->udp;
    p->icmp    += s->icmp;
    p->arp     += s->arp;
    p->other   += s->other;
    free(s);
  }else{
    d=hash(s->ip);
    s->next = T[d];
    T[d]=s;
  }
}