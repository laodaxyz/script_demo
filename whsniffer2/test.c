#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

int main()  
{
  int ip;
  char *net;
  ip = inet_addr("255.255.255.255");
  struct in_addr addr;
  addr.s_addr = ip;

  printf("%d\n", ip);
  printf("%s\n", inet_ntoa(addr));
  
} 
