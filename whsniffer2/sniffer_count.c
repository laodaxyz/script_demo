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
#include <errno.h>
#include <sys/syslog.h>

unsigned  long long flows,packets;
bpf_u_int32 netp,maskp;

void callPacket(u_char *arg,const struct pcap_pkthdr* pack,const u_char *content)  { 
    struct ether_header *ethernet;
    struct iphdr *ip;
    ethernet=(struct ether_header *)content;
    if(ntohs(ethernet->ether_type)==ETHERTYPE_IP) {
      ip=(struct iphdr*)(content+14);
      //ip包大小 + 以太网包头包尾
      flows += ntohs(ip->tot_len) + 18;
      packets++;
    }
    // else if(ntohs (ethernet->ether_type) == ETHERTYPE_ARP) {
    //   flows += 60;
    //   packets++;
    // } 
}

void handler(){
  time_t rawtime;
  struct tm * timeinfo;
  time ( &rawtime );
  timeinfo = localtime ( &rawtime );

  FILE *fp;
  fp=fopen("sniffer_count.txt", "a");
  fprintf(fp,"%sflows  :%lld\npackets:%lld\n-------------------------\n", asctime(timeinfo), flows, packets);
  fclose(fp);

  flows=0; packets=0;
  signal(SIGALRM, handler);
  alarm(300);
}

void Daemon(void){
    pid_t mypid1, mypid2;
    u_short n = 0;
    openlog("./sniffer_count.txt",LOG_PID, LOG_LOCAL7);
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
  //Daemon();
  flows,packets = 0;
  char *devname = "eth2";
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
  int forint=0;
  for (;;){
    forint++; sleep(60);
  }
  return 0;
}