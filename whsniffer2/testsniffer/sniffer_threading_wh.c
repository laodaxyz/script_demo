#include <pcap.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <pthread.h>

unsigned  long long in_bytes, in_packets, out_bytes, out_packets=0;
bpf_u_int32 netp,netp2,maskp;
pthread_mutex_t hash_lock;
pthread_attr_t attr;
sigset_t mask_sig;

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
    if      ( ((ip->saddr & maskp)==netp)  && ((ip->daddr & maskp)!=netp) ){ //出网包
      //
    }else if( ((ip->daddr & maskp)==netp)  && ((ip->saddr & maskp)!=netp) ){  //进网包
      //
    }else if( (((ip->daddr & maskp)==netp)  && ((ip->saddr & maskp)==netp)) ){ //内网广播包
      //
    }else{  //外网包
      char sip[20],dip[20];
      struct in_addr addr;
      addr.s_addr = ip->saddr;
      strcpy(sip,inet_ntoa(addr));
      addr.s_addr = ip->daddr;
      strcpy(dip,inet_ntoa(addr));
      printf("out: %s->%s\n", sip, dip);
      in_bytes += tot_len;
      in_packets++;
    }

  }
}

void *th_works(char *devname){
  char errBuf[PCAP_ERRBUF_SIZE];
  pcap_t *device = pcap_open_live(devname, 65535, 1, 0, errBuf);
  pcap_loop(device, -1, callPacket, NULL);
  pcap_close(device);
}

void th_sigs(){
  for(;;){
    int sig;
    if (sigwait(&mask_sig, &sig) != 0){ printf("wait signal error\n"); exit(1); }
    switch (sig){
      case SIGTERM:
        printf("Recv signal term, proc will exit...\n"); exit(0);
      case SIGINT:
        printf("Ctrl + C, proc will exit...\n"); exit(0);
      case SIGALRM:
        alarm(300);
        printf("in_bytes:%lld in_packets:%lld out_bytes:%lld out_packets:%lld\n", in_bytes, in_packets, out_bytes, out_packets);
        in_bytes=0; in_packets=0; out_bytes=0; out_packets=0;
        break;
      default:
        printf("Recv signum = %i\n", sig); break;
    }
  }
}
void myexit(void){
  pthread_mutex_destroy(&hash_lock);
  pthread_attr_destroy(&attr);
}

int main(){
  char errBuf[PCAP_ERRBUF_SIZE], *devname, *devname2;
  devname  = "eth1";
  devname2 = "eth2";
  
  char net[20],mask[20];
  struct in_addr addr;
  pcap_lookupnet(devname, &netp, &maskp, errBuf);
  addr.s_addr = netp;
  strcpy(net,inet_ntoa(addr));

  addr.s_addr = maskp;
  strcpy(mask,inet_ntoa(addr));
  printf("net %s ,mask %s\n",net, mask);
  printf("net %d ,mask %d ..The PID is %d\n",netp, maskp, getpid());
  
  pthread_mutex_init(&hash_lock, NULL);
  pthread_t sigtid,workid,workid2;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
  atexit(myexit);

  sigfillset(&mask_sig);
  if (pthread_sigmask(SIG_BLOCK, &mask_sig, NULL) != 0 ){
    printf("pthread_sigmask error\n"); exit(1);
  }
  if (pthread_create(&sigtid, &attr, (void *)th_sigs, NULL) != 0){
    printf("pthread_th_sigs error\n"); exit(1);
  }
  if (pthread_create(&workid, NULL, (void *)th_works, devname) != 0 ){
    printf("pthread_th_works error\n"); exit(1);
  }
  if (pthread_create(&workid2, NULL, (void *)th_works, devname2) != 0 ){
    printf("pthread_th_works error\n"); exit(1);
  }
  alarm(300);
  int forint=0;
  for (;;){
    forint++; sleep(60);
  }
  return 0;
}
