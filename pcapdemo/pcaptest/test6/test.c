#include <pcap.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

long tbyte,inbyte,outbyte;

void getPacket(u_char * arg, const struct pcap_pkthdr * pkthdr, const u_char * packet){
  tbyte +=  pkthdr->caplen;
  printf("Packet length: %d\n", pkthdr->len);
  printf("Number of bytes: %d\n", pkthdr->caplen);
  printf("tbyte : %d\n", tbyte);
}

int main(){
  char errBuf[PCAP_ERRBUF_SIZE];
  tbyte  = 0;
  inbyte = 0;
  outbyte= 0;
  
  /* open a device, wait until a packet arrives */
  pcap_t * device = pcap_open_live("eth2", 65535, 1, 1, errBuf);
  
  /* wait loop forever */
  long id = 0;
  pcap_loop(device, -1, getPacket, (u_char*)&id);
  //alarm(300);
  pcap_close(device);
  return 0;
}
