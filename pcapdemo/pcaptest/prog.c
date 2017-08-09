#include "pcap.h"
#include <stdlib.h>

void getPacket(u_char * arg, const struct pcap_pkthdr * pkthdr, const u_char * packet)
{
  int * id = (int *)arg;
  printf("id: %d\n", ++(*id));
  printf("Packet length: %d\n", pkthdr->len);
  printf("Number of bytes: %d\n", pkthdr->caplen);
  printf("Recieved time: %s", ctime((const time_t *)&pkthdr->ts.tv_sec)); 
  printf("\n\n");
}

main()
{
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_if_t *devs,*alldevs;

    /* 获取本地机器设备列表 */
    if (pcap_findalldevs(&alldevs, errbuf) == -1)
    {
        fprintf(stderr,"Error in pcap_findalldevs_ex: %s\n", errbuf);
        exit(1);
    }
    
    /* 打印列表 */
    for(devs= alldevs; devs != NULL; devs= devs->next)
    {
        if (strcmp(devs->name, "lo") && strcmp(devs->name, "virbr0") && strcmp(devs->name, "eth0")){
            printf("device name %s", devs->name);
            pcap_t * device = pcap_open_live(devs, 65535, 1, 0, errbuf);
            if(!device)
            {
                printf("error: pcap_open_live(): %s\n", errbuf);
                exit(1);
            }
             int id = 0;
             pcap_loop(device, -1, getPacket, (u_char*)&id);
             
             pcap_close(device);
        }
    }

    /* 不再需要设备列表了，释放它 */
    pcap_freealldevs(alldevs);
}