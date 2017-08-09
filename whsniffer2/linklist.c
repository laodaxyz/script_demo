#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

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
#define HASHSIZE 10000
typedef htnode **lists;
lists list;
int lnum=0;

int inlist()
{
	htnode *hv;
	hv = calloc(1, sizeof(struct node));
	hv->ip=lnum;
	list[lnum] = hv;
	lnum++;
	return 1;
}

void delist(lists tlist)
{
	int i = 0;
	for (; i < lnum; ++i)
	{
		if(tlist[i]==NULL)
			continue;
		printf("%d\n", tlist[i]->ip);
		free(tlist[i]);
	}
}

int main(int argc, char const *argv[])
{
	list = calloc(HASHSIZE, sizeof(struct node*));
	int i = 0;
	for (; i < 100; ++i)
	{
		inlist();
	}
	delist(list);
	return 0;
}