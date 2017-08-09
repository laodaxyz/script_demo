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
#include <errno.h>
#include <sys/syslog.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <pthread.h>

//流量过大，会丢包   考虑另外的方案：解包后放入一个写入链表，不加锁   5分钟信号开始后，加锁，链表指向新表头，全局表头清空，解锁。新表头循环插入hash表，判断最后一个。。。。

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
unsigned  long long in_bytes, in_packets, out_bytes, out_packets=0;
bpf_u_int32 netp,netp2,maskp;
hashtable ht,ht_out;
pthread_mutex_t hash_lock;
pthread_attr_t attr;
sigset_t mask_sig;

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

//哈希表销毁
void hashtable_descrty(hashtable h, int size, int in_out){
  value *v;
  int max = 1000;
  xvalue vs[max];
  int sock,j=1;
  struct sockaddr_in svraddr;
  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){ exit(1); }
  svraddr.sin_family = AF_INET;
  svraddr.sin_port = htons(4200);
  if(inet_pton(AF_INET, "115.182.1.230", &svraddr.sin_addr) < 0){ exit(1); }
  if(connect(sock, (const struct sockaddr *)&svraddr, sizeof(svraddr)) < 0){ close(sock);return; }
  memset(&vs[0], 0, sizeof(xvalue));
  if(in_out==0){
    vs[0].v.other = 0;
    vs[0].fbytes  = out_bytes;
    vs[0].fpacket = out_packets;
  }
  else{
    vs[0].v.other = 1;
    vs[0].fbytes  = in_bytes;
    vs[0].fpacket = in_packets;
  }
  int i;
  for (i = 0; i < size; i++)
  {
    if (j>=max) break; 
    htnode *p,*t;
    p = h[i];
    if (p ==NULL ) continue;
    while(p->next != NULL){
      vs[j].v.sip     = p->ip;
      vs[j].v.tcp     = p->tcp;
      vs[j].v.udp     = p->udp;
      vs[j].v.icmp    = p->icmp;
      vs[j].v.other   = p->other;
      vs[j].v.bytes   = p->bytes;
      vs[j].v.packets = p->packets;
      vs[j].fbytes    = p->fbytes;
      vs[j].fpacket   = p->fpacket;
      j++;
      t = p->next;
      free(p);
      p=t;
    }
    vs[j].v.sip     = p->ip;
    vs[j].v.tcp     = p->tcp;
    vs[j].v.udp     = p->udp;
    vs[j].v.icmp    = p->icmp;
    vs[j].v.other   = p->other;
    vs[j].v.bytes   = p->bytes;
    vs[j].v.packets = p->packets;
    vs[j].fbytes    = p->fbytes;
    vs[j].fpacket   = p->fpacket;
    j++;
    free(p);
    p=NULL;
  }
  free(h);
  h=NULL;
  write(sock, vs, sizeof(xvalue) * j);
  close(sock);
}

int insert_top(hashtable T, htnode *p, int newsize){
  struct in_addr addr;
  htnode *t,*f;
  int i;
  for (i = 0; i < newsize; ++i)
  {
    if (T[i] != NULL){
      if(p->bytes > T[i]->bytes){
        t = T[i];
        int j=i;
        while(j<(newsize-1) && t!=NULL){
          j++;
          f=T[j];
          T[j]=t;
          t=f;
        }
        if(t!=NULL) free(t);
        p->next = NULL;
        T[i] = p;
        return 0;
      }
    }else{
      p->next = NULL;
      T[i] = p;
      return 0;
    }
  }
  return 1;
}

hashtable hashtable_top(hashtable h, int size, int newsize){
  hashtable topht;
  if((topht = (struct node **)calloc(newsize, sizeof(struct node*))) == NULL) exit(-1);
  int i;
  for (i = 0; i < size; i++)
  {
    htnode *p,*t;
    p = h[i];
    if (p ==NULL ) continue;
    while(p->next != NULL){
      t = p->next;
      if (insert_top(topht,p,newsize)){
        free(p);
        p=NULL;
      }
      p=t;
    }
    if (insert_top(topht,p,newsize)){
      free(p);
      p=NULL;
    }
  }
  free(h);
  h=NULL;
  return topht;
}

void callPacket(u_char *arg, const struct pcap_pkthdr* pack, const u_char *content)  
{
  struct ether_header *ethernet;
  struct iphdr *ip;
  ethernet=(struct ether_header *)content;
  //ip
  if(ntohs(ethernet->ether_type)==ETHERTYPE_IP)
  {
    ip=(struct iphdr*)(content+14);
    int tot_len=ntohs(ip->tot_len) + 18;
    //外网包
    htnode *hv_out;
    if( (hv_out = (struct node*)calloc(1, sizeof(struct node))) ==NULL) exit(-1);
    hv_out->bytes = tot_len;
    hv_out->packets = 1;
    //内网包
    htnode *hv;
    if( (hv    = (struct node*)calloc(1, sizeof(struct node))) ==NULL) exit(-1);
    hv->bytes = tot_len;
    hv->packets = 1;
    switch(ip->protocol)
    {
      case 6:
        hv_out->tcp = 1; 
        hv->tcp     = 1;
        break;
      case 17:
        hv_out->udp = 1; 
        hv->udp     = 1;
        break;
      case 1:
        hv_out->icmp = 1; 
        hv->icmp     = 1;
        break;
      default:
        hv_out->other = 1; 
        hv->other     = 1;
        break;
    }
    if      ( (((ip->saddr & maskp)==netp)  && ((ip->daddr & maskp)!=netp)) ||
              (((ip->saddr & maskp)==netp2) && ((ip->daddr & maskp)!=netp2)) ){ //出网包
      //内网ip
      hv->ip = ip->saddr;
      pthread_mutex_lock(&hash_lock);
      hashtable_insert(ht, HASHSIZE, hv);
      pthread_mutex_unlock(&hash_lock);
      //外网ip
      hv_out->ip = ip->daddr;
      pthread_mutex_lock(&hash_lock);
      hashtable_insert(ht_out, HASHSIZEIN, hv_out);
      pthread_mutex_unlock(&hash_lock);
      out_bytes += tot_len;
      out_packets++;
    }else if( (((ip->daddr & maskp)==netp)  && ((ip->saddr & maskp)!=netp)) ||
              (((ip->daddr & maskp)==netp2) && ((ip->saddr & maskp)!=netp2)) ){  //进网包
      //内网ip
      hv->fbytes  = tot_len;
      hv->fpacket = 1;
      hv->ip = ip->daddr;
      pthread_mutex_lock(&hash_lock);
      hashtable_insert(ht, HASHSIZE, hv);
      pthread_mutex_unlock(&hash_lock);
      //外网ip
      hv_out->fbytes  = tot_len;
      hv_out->fpacket = 1;
      hv_out->ip = ip->saddr;
      pthread_mutex_lock(&hash_lock);
      hashtable_insert(ht_out, HASHSIZEIN, hv_out);
      pthread_mutex_unlock(&hash_lock);
      in_bytes += tot_len;
      in_packets++;
    }else if( (((ip->daddr & maskp)==netp)  && ((ip->saddr & maskp)==netp)) ||
              (((ip->daddr & maskp)==netp2) && ((ip->saddr & maskp)==netp2)) || ((ip->saddr == 0) || (ip->daddr == -1)) ){ //内网广播包
      free(hv);
      hv=NULL;
      free(hv_out);
      hv_out=NULL;
      in_bytes += tot_len;
      in_packets++;
    }else{  //外网包
      //内网ip
      hv->ip = ip->saddr;
      pthread_mutex_lock(&hash_lock);
      hashtable_insert(ht, HASHSIZE, hv);
      pthread_mutex_unlock(&hash_lock);
      // //外网ip
      // hv_out->ip = ip->daddr;
      // pthread_mutex_lock(&hash_lock);
      // hashtable_insert(ht_out, HASHSIZEIN, hv_out);
      // pthread_mutex_unlock(&hash_lock);

      // free(hv);
      // hv=NULL;
      free(hv_out);
      hv_out=NULL;

      out_bytes += tot_len;
      out_packets++;
    }

  }/*else if(ntohs (ethernet->ether_type) == ETHERTYPE_ARP) {
      in_bytes += 60;
      in_packets++;
  }*/
}

void *th_works(void *devname){
  char errBuf[PCAP_ERRBUF_SIZE];
  pcap_t *device = pcap_open_live(devname, 65535, 1, 0, errBuf);
  pcap_loop(device, -1, callPacket, NULL);
  pcap_close(device);
}

void th_sigs(){
  for(;;){
    int sig;
    if (sigwait(&mask_sig, &sig) != 0){ printf("wait signal error\n"); exit(1); }
    hashtable oldht, oldht_out, topht, topht_out;
    switch (sig){
      case SIGTERM:
        printf("Recv signal term, proc will exit...\n"); exit(0);
      case SIGINT:
        printf("Ctrl + C, proc will exit...\n"); exit(0);
      case SIGALRM:
        oldht = ht; oldht_out = ht_out;
        if((ht     = (struct node **)calloc(HASHSIZE,   sizeof(struct node*))) == NULL) exit(-1);
        if((ht_out = (struct node **)calloc(HASHSIZEIN, sizeof(struct node*))) == NULL) exit(-1);
        alarm(300);
        //printf("in_bytes:%lld in_packets:%lld out_bytes:%lld out_packets:%lld\n", in_bytes, in_packets, out_bytes, out_packets);
        syslog(LOG_NOTICE, "in_bytes:%llu in_packets:%llu out_bytes:%llu out_packets:%llu", in_bytes, in_packets, out_bytes, out_packets);
        //内网ip
        hashtable_descrty(oldht, HASHSIZE, 1);
        //外网ip排序，取前20
        topht_out = hashtable_top(oldht_out, HASHSIZEIN, 20);
        hashtable_descrty(topht_out, 20, 0);
        in_bytes=0; in_packets=0; out_bytes=0; out_packets=0;
        break;
      default:
        printf("Recv signum = %i\n", sig); break;
    }
  }
}

void Daemon(void){
    pid_t mypid1, mypid2;
    u_short n = 0;
    openlog("sniffer3",LOG_PID, LOG_LOCAL7);
    umask(0);
    if ((mypid1 = fork()) == -1)
        {syslog(LOG_ERR, "fork: %s", strerror(errno));exit(1);}
    if (mypid1 > 0) exit(0);
    setsid();
    signal(SIGHUP, SIG_IGN);
    if ((mypid2 = fork()) == -1)
        {syslog(LOG_ERR, "fork: %s", strerror(errno));exit(1);}
    if (mypid2 > 0) exit(0);
    chdir("/");
    for(; n < 1025; n++)
        close(n);
    open("/dev/null", O_RDWR);
    dup(0);  dup(0);
}

int main(){
  Daemon();
  char errBuf[PCAP_ERRBUF_SIZE], *devname, *devname2;
  //devname = pcap_lookupdev(errBuf);
  devname  = "eth2";
  //devname2 = "eth2";
  netp2 = 8011306;
  
  char net[20],net2[20],mask[20];
  struct in_addr addr;
  pcap_lookupnet(devname, &netp, &maskp, errBuf);
  addr.s_addr = netp;
  strcpy(net,inet_ntoa(addr));
  addr.s_addr = maskp;
  strcpy(mask,inet_ntoa(addr));
  addr.s_addr = netp2;
  strcpy(net2,inet_ntoa(addr));
  //printf("net %d ,mask %d ..The PID is %d\n",netp, maskp, getpid());
  syslog(LOG_NOTICE, "devname:%s ,net:%s ,net2:%s ,mask:%s . The PID is %d", devname, net, net2, mask, getpid());
  
  pthread_mutex_init(&hash_lock, NULL);
  pthread_t sigtid,workid,workid2;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

  ht     = (struct node **)calloc(HASHSIZE  , sizeof(struct node*));
  ht_out = (struct node **)calloc(HASHSIZEIN, sizeof(struct node*));
  sigfillset(&mask_sig);
  if (pthread_sigmask(SIG_BLOCK, &mask_sig, NULL) != 0 ){
    printf("pthread_sigmask error\n"); exit(1);
  }
  if (pthread_create(&sigtid, &attr, (void *)th_sigs, NULL) != 0){
    printf("pthread_th_sigs error\n"); exit(1);
  }
  if (pthread_create(&workid, NULL, th_works, devname) != 0 ){
    printf("pthread_th_works error\n"); exit(1);
  }
  // if ((perr = pthread_create(&workid2, NULL, th_works, devname2)) != 0 ){
  //   printf("pthread_th_works error\n"); exit(1);
  // }
  alarm(300);
  int forint=0;
  for (;;){
    forint++; sleep(60);
  }
  return 0;
}
