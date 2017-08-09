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

#define HASHSIZE 10000
#define HASHSIZEIN 1000
typedef struct node{
  u_int32_t ip;
  unsigned long long bytes;
  unsigned long long packets;
  unsigned long long fbytes;
  unsigned long long fpacket;
  unsigned long long tcp;
  unsigned long long udp;
  unsigned long long icmp;
  unsigned long long other;
  struct node *next;
}htnode;
typedef htnode **hashtable;
unsigned  long long flowtotal,packetnums;
bpf_u_int32 netp,maskp;
hashtable ht,ht_in;

void handler();
void callPacket(u_char *arg, const struct pcap_pkthdr* pack, const u_char *content);
int hash(u_int32_t ip, int size);
htnode * hashtable_search(hashtable T, int size, u_int32_t ip);
int hashtable_insert(hashtable T, int size, htnode *s);
hashtable hashtable_init(int size);
void hashtable_descrty(hashtable h, int size);
hashtable hashtable_top(hashtable T, int size);

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

  ht = hashtable_init(HASHSIZE);
  ht_in = hashtable_init(HASHSIZEIN);
  signal(SIGALRM, handler);
  alarm(1);
  pcap_loop(device, -1, callPacket, NULL);
  pcap_close(device);
  return 0;
}

void handler(){
  printf("flowtotal : %lld\n",flowtotal);
  printf("packetnums: %lld\n",packetnums);
  flowtotal,packetnums=0;
  hashtable oldht,oldht_in;
  oldht = ht;
  oldht_in = ht_in;
  //printf("-------------------\n");
  ht = hashtable_init(HASHSIZE);
  ht_in = hashtable_init(HASHSIZEIN);
  signal(SIGALRM, handler);
  alarm(1);
  //printf("-------------------\n");
  hashtable_descrty(oldht,HASHSIZE);
  hashtable_descrty(oldht_in,HASHSIZEIN);
  //hashtable topht_in;
  //topht_in = hashtable_init(20);
  //topht_in = hashtable_top(oldht_in, HASHSIZEIN);
  //hashtable_descrty(topht_in, 20);

  time_t timep; time(&timep);
  printf("%s\n",asctime(gmtime(&timep)));
}

void callPacket(u_char *arg,const struct pcap_pkthdr* pack,const u_char *content)  
{
  struct ether_header *ethernet;
  struct iphdr *ip;
  ethernet=(struct ether_header *)content;
  //ip
  if(ntohs(ethernet->ether_type)==ETHERTYPE_IP)
  {
    ip=(struct iphdr*)(content+14);
    int tot_len=ntohs(ip->tot_len);
    //外网包
    htnode *hv_in;
    if( (hv_in = (htnode *)calloc(1, sizeof(htnode))) ==NULL) exit(-1);
    //内网包
    htnode *hv;
    if( (hv = (htnode *)calloc(1, sizeof(htnode))) ==NULL) exit(-1);
    switch(ip->protocol)
    {
      case 6:
        hv_in->tcp = 1; 
        hv->tcp    = 1;
        break;
      case 17:
        hv_in->udp = 1; 
        hv->udp    = 1;
        break;
      case 1:
        hv_in->icmp = 1; 
        hv->icmp    = 1;
        break;
      default:
        hv_in->other = 1; 
        hv->other    = 1;
        break;
    }
    if      ( ((ip->saddr & maskp)==netp) && ((ip->daddr & maskp)!=netp) ){ //出网包
      //内网包
      hv->ip = ip->daddr;
      hashtable_insert(ht, HASHSIZE, hv);
      //外网包
      hv_in->ip = ip->saddr;
      hashtable_insert(ht_in, HASHSIZEIN, hv_in);
      flowtotal += tot_len;
      packetnums++;
    }else if( ((ip->daddr & maskp)==netp) && ((ip->saddr & maskp)!=netp) ){  //进网包
      //内网包
      hv->fbytes  = tot_len;
      hv->fpacket = 1;
      hv->ip = ip->saddr;
      hashtable_insert(ht, HASHSIZE, hv);
      //外网包
      hv_in->fbytes  = tot_len;
      hv_in->fpacket = 1;
      hv_in->ip = ip->daddr;
      hashtable_insert(ht_in, HASHSIZEIN, hv_in);
      flowtotal += tot_len;
      packetnums++;
    }else{  //广播包 不抓取
      free(hv);
      hv=NULL;
      free(hv_in);
      hv_in=NULL;
      // hv->fbytes  = tot_len;
      // hv->fpacket = 1;
      // hv->ip = ip->saddr;
      // hashtable_insert(ht, HASHSIZE, hv);

      // hv_in->fbytes  = tot_len;
      // hv_in->fpacket = 1;
      // hv_in->ip = ip->daddr;
      // hashtable_insert(ht_in, HASHSIZEIN, hv_in);
      // flowtotal += tot_len;
      // packetnums++;
    }
  }
}

int hash(u_int32_t ip, int size) {
  return ip % size;
}

htnode * hashtable_search(hashtable T, int size, u_int32_t ip){
  htnode *p=T[hash(ip, size)];
  while(p!=NULL && p->ip!=ip)
    p=p->next;
  return p;
}

int hashtable_insert(hashtable T, int size, htnode *s) {
  int d;
  htnode *p=hashtable_search(T, size, s->ip);
  if(p!=NULL){
    p->fbytes  += s->fbytes;
    p->fpacket += s->fpacket;
    p->bytes   += s->bytes;
    p->packets += s->packets;
    p->tcp     += s->tcp;
    p->udp     += s->udp;
    p->icmp    += s->icmp;
    p->other   += s->other;
    free(s);
    s=NULL;
  }else{
    d=hash(s->ip, size);
    s->next = T[d];
    T[d]=s;
  }
}

//哈希表初始化
hashtable hashtable_init(int size){
  hashtable h;
  if( (h = (struct node **)calloc(size, sizeof(struct node*))) == NULL){
    printf("Out of memory!\n"); exit(-1);
  }
  printf("init:%p\n", h);
  // int i;
  // for(i = 0; i < size; ++i)
  // {
  //   if( (h[i] = (struct node *)calloc(1, sizeof(struct node))) == NULL){
  //     printf("Out of memory!\n"); exit(-1);
  //   }
  //   // else{
  //   //   h[i]->ip = i;
  //   //   h[i]->next = NULL;
  //   // }
  // }
  return h;
}

//哈希表销毁
void hashtable_descrty(hashtable h,int size){
  // value *v;
  // xvalue vs[400];
  // int sock,j;
  // struct sockaddr_in svraddr;
  // if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){ exit(1); }
  // svraddr.sin_family = AF_INET;
  // svraddr.sin_port = htons(4200);
  // if(inet_pton(AF_INET, "127.0.0.1", &svraddr.sin_addr) < 0){ exit(1); }
  // if(connect(sock, (const struct sockaddr *)&svraddr, sizeof(svraddr)) < 0){ close(sock);return; }
  // memset(&vs[0], 0, sizeof(xvalue));
  // vs[0].v.other = 0;
  // vs[0].fbytes  = flowtotal;
  // vs[0].fpacket = packetnums;
  struct in_addr addr;
  int i;
  for (i = 0; i < size; i++)
  {
    htnode *p,*t;
    p = h[i];
    if (p ==NULL ) continue;
    while(p->next != NULL){
      //调试打印 hash表中的ip
      // addr.s_addr = p->ip;
      // printf("%s\n", inet_ntoa(addr));
      t = p->next;
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
      p=t;
    }
    free(p);
    p=NULL;
  }
  // free(h);
  // h=NULL;
  // write(sock, vs, sizeof(xvalue) * j);
  // close(sock);
}

int insert_top(hashtable T,htnode *p){
  struct in_addr addr;
  int i;
  for (i = 0; i < 20; ++i)
  {
    if (T[i] != NULL){
      if(p->bytes > T[i]->bytes){
        addr.s_addr = p->ip;
        printf("in %d:%s\n",i, inet_ntoa(addr));
        T[i] = p;
        return 0;
      }
    }else{
      addr.s_addr = p->ip;
      printf("in %d:%s-----\n",i, inet_ntoa(addr));
      T[i] = p;
      return 0;
    }
  }
  return 1;
}

hashtable hashtable_top(hashtable h,int size){
  hashtable topht;
  int i;
  for (i = 0; i < size; ++i)
  {
    htnode *p;
    p = h[i];
    while(p->next != NULL){
      h[i] = p->next;
      if (insert_top(topht,p)){
        free(p);
        p=NULL;
      }
      p=h[i];
    }
    free(p);
    p=NULL;
  }
  free(h);
  h=NULL;
  return topht;
}