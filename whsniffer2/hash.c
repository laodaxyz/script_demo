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
typedef struct header{
	int count;
	struct nodes *next;
}header_t;
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

void hashtable_descrty(hashtable h,int size){
	int i;
	for (i = 0; i < size; i++)
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

hashtable hashtable_init(int size){
	hashtable h;
	h = (htnode **)malloc(sizeof(htnode *)*size);
	if(h == NULL) {
		printf("Out of memory!\n");
		exit(-1);
	}
	int i;
	for (i = 0; i < size; i++)
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
//排序
hashtable hashtable_top(hashtable h,int size){
	header_t header;
	int i;
	for (i = 0; i < size; i++)
	{
		htnode *p,*tp,*tmp;
		p = h[i];
		if (p->next != NULL)
		{
			while(p->next != NULL){
				h[i] = p->next;
				// int ti;
				// if(header.count==0){
				// 	p->next = NULL;
				// 	header.next = p;
				// }esle{
				// 	tp = header.next;
				// 	for (int ti = 0; ti < header.count; ++ti)
				// 	{
				// 		if(p->bytes < tp->bytes){
				// 			if (header.count ==20){
				// 				tp->next=p;
				// 			}esle{
				// 				tp->next=p;
				// 			}
				// 		}
				// 	}
				// }
				
				free(p);
				p = h[i];
			}
		}else{
			free(h[i]);
		}
	}
	return newht;
}

void call(){
	int i;
	for (i = 10; i < 14; ++i)
	{
		htnode *hv;
		hv = (htnode *)malloc(sizeof(htnode));
		// hv->ip    =i;
		// hv->tcp   =i+2;
		hv = {0,2,0,0};
		hashtable_insert(ht,hv);
	}
	hashtable oldht,topht;
	oldht = ht;
	ht = hashtable_init(HASHSIZE);
	topht = hashtable_top(oldht,HASHSIZE);
	hashtable_descrty(oldht,HASHSIZE);
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
	ht = hashtable_init(HASHSIZE);
	call();
	return 0;
}