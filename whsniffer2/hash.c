#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <signal.h>
#include <fcntl.h>

#define HASHSIZE 3
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

int hash( long long key ) {
  return key % HASHSIZE;
}

htnode * hashtable_search(hashtable T,  long long key){
  htnode *p=T[hash(key)];
  while(p!=NULL && p->ip!=key)
    p=p->next;
  return p;
}

int hashtable_insert(hashtable T, htnode *s) {
  int d;
  htnode *p,newnode;
  p=hashtable_search(T,s->ip);
  if(p!=NULL){
    p->bytes   += s->bytes;
    p->packets += s->packets;
    p->tcp     += s->tcp;
    p->udp     += s->udp;
    p->icmp    += s->icmp;
    p->other   += s->other;
    free(s);
  }else{
    d=hash(s->ip);
    s->next = T[d];
    T[d]=s;
  }
}

void hashtable_descrty(hashtable h){
	int i;
	for (i = 0; i < HASHSIZE; i++)
	{
		htnode *p;
		p = h[i];
		if (p->next != NULL)
		{
			while(p->next != NULL){
				printf("%lld\n", p->ip);
				h[i] = p->next;
				free(p);
				p = h[i];
			}
		}else{
			free(h[i]);
		}
	}
	free(h);
}

hashtable hashtable_init(){
	hashtable h;
	h = (htnode **)malloc(sizeof(htnode *)*HASHSIZE);
	if(h == NULL) {
		printf("Out of memory!\n");
		exit(-1);
	}
	int i;
	for (i = 0; i < HASHSIZE; i++)
	{
		h[i] = (htnode *)malloc(sizeof(htnode));
		if (h[i] == NULL){
			printf("Out of memory!\n");
			exit(-1);
		}else{
			h[i]->ip = i;
			h[i]->next = NULL;
		}
	}
	return h;
}

void call(){
	int i;
	for (i = 10; i < 14; ++i)
	{
		htnode *hv;
		hv = (htnode *)malloc(sizeof(htnode));
		hv->ip    =i;
		hv->tcp   =i+2;
		hashtable_insert(ht,hv);
	}
	hashtable oldht;
	oldht = ht;
	ht = hashtable_init();
	hashtable_descrty(oldht);
}

// void Daemon()
// {
//     pid_t mypid1, mypid2;
//     u_short n = 0;
//     umask(0);
//     if ((mypid1 = fork()) == -1) exit(1);
//     if (mypid1 > 0) exit(0);
//     setsid();
//     signal(SIGHUP, SIG_IGN);
//     if ((mypid2 = fork()) == -1) exit(1);
//     if (mypid2 > 0) exit(0);
//     chdir("/");
//     for(; n < 1025; n++) close(n);
//     open("/dev/null", O_RDWR);
//     dup(0);
//     dup(0);
// }
int main(int argc, char const *argv[])
{
	//Daemon();
	ht = hashtable_init();
	call();
	return 0;
}