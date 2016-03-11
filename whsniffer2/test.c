#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#define HASHSIZE 7
typedef struct nodes{
     long long ip;
     long long bytes;
     long long packets;
     long long tcp;
     long long udp;
     long long icmp;
     long long other;
    struct nodes *next;
}htnode;
typedef htnode **hashtable;
hashtable ht;   

int main(){
  for (int i = 0; i < 10; ++i)
  {
    if (i>4) break;
    printf("%d\n", i);
  }
  return 0;   
}
