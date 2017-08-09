#include <stdio.h>
#include <string.h>
#include <pcap.h>
/*
*  显示所有设备
*/
int main()
{
  char errBuf[PCAP_ERRBUF_SIZE];
  pcap_if_t *devpointer;
  int i;
  
  if (pcap_findalldevs(&devpointer, errBuf) < 0){
      printf("error: %s\n", errBuf);
  }else{
    for (i = 0; devpointer != NULL; i++) {
        printf("%d.%s", i+1, devpointer->name);
        if (devpointer->description != NULL)
          printf(" (%s)", devpointer->description);
        if (devpointer->flags != 0)
          printf(" [%u]", devpointer->flags);
        printf("\n");
        devpointer = devpointer->next;
    }  
  }
  return 0;
}