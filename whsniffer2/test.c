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
typedef htnode * position;
typedef position *hashtable;
hashtable ht;   

int main(){
  htnode *hv;
  hv = (htnode *)malloc(sizeof(htnode));
  hv->ip    =1;
  hv->tcp   =2;
  free(hv);
  hv=NULL;
  char *p;
  p=(char *)malloc(100); 
  free(p);
  p=NULL;
  int a=1;
  int b=2;
  //hashtable_insert(ht,hv);
  return 0;   
}
