#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/syslog.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "/usr/local/mysql/include/mysql/mysql.h"

#define NUM 1025
#define PORT 4200

#define MY_HOST "115.182.1.175"
#define MY_USER "sniffer"
#define MY_PASS "t9YArK_&j(.sN$"
#define MY_DB "sniffer_monitor"
#define MY_SOCK "/tmp/mysql.sock"

typedef struct value{
  u_int32_t sip;
  unsigned long long packets;
  unsigned long long tcp;
  unsigned long long udp;
  unsigned long long icmp;
  unsigned long long other;
  unsigned long long bytes;
}value;
typedef struct{
  value v;
  unsigned long long fpacket;
  unsigned long long fbytes;
}xvalue;

ssize_t Read(int fd, char *vptr, size_t n)
{
	size_t nleft;
	int nread;
	char *ptr;
	ptr = vptr;
	nleft = n;
	errno = 0;
	while (nleft > 0)
	{
		if ((nread = read(fd, ptr, nleft)) < 0){
			if (errno == EINTR)
				nread = 0;
			else
				return -1;
		}else if (nread == 0){
			errno = EHOSTDOWN;
			break;
		}
		nleft -= nread;
		ptr += nread;
	}
	return(n - nleft);
}

char *getcurtime(void){
    time_t times;
    struct tm *p;
    char *ptrs,minstr[3];
    int min;
    ptrs = malloc(17 + 11);
 
    time(&times);
    p = localtime(&times);
    //2016-04-12 13:
    strftime(ptrs,15,"%Y-%m-%d %H:",p);
    //格式化时间 保证时间间隔为5分钟
    min = (p->tm_min/5)*5;
    if (min>9){
      sprintf(minstr, "%d" , min);
    }else{
      sprintf(minstr, "0%d" , min);
    }
    strcat(ptrs,minstr);
    //2012-01-00 月份
    strftime(&ptrs[17], 11, "%Y-%m-00", p);
    return ptrs;
}

void work(void)
{
	int i, id, maxi, listenfd, clientfd, maxfd;
	char sip[INET_ADDRSTRLEN], dip[INET_ADDRSTRLEN], clientip[INET_ADDRSTRLEN];
	int client[FD_SETSIZE], nready;
	sigset_t zeromask;
	ssize_t n;
	fd_set rset, allset;
	struct sockaddr_in clientaddr, servaddr;
	socklen_t socklen;
	xvalue ret[NUM];
	char sql[300], *time;
	MYSQL my_conn;
	MYSQL_RES *res;
	MYSQL_ROW row;

	mysql_init(&my_conn);
	if ( ! mysql_real_connect(&my_conn, MY_HOST, MY_USER, MY_PASS, MY_DB, 3309, MY_SOCK,0))
	{
		syslog(LOG_ERR, "Connection error %d: %s\n", mysql_errno(&my_conn), mysql_error(&my_conn)); exit(2);
	}

	sigemptyset(&zeromask);

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1){
		syslog(LOG_ERR, "socket: %s", strerror(errno)); exit(1);
	}

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		syslog(LOG_ERR, "bind: %s", strerror(errno)); exit(1);
	}

	id = 1;
	if ((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &id, sizeof(int))) < 0){
		syslog(LOG_ERR, "setsockopt: %s", strerror(errno)); exit(1);
	}

	maxfd = listenfd;
	maxi = -1;
	for(i = 0; i < FD_SETSIZE; i++)
		client[i] = -1;

	FD_ZERO(&allset);
	FD_SET(listenfd, &allset);

	listen(listenfd, 1024);
	for(;;)
	{
		rset = allset;
		if ((nready = pselect(maxfd + 1, &rset, NULL, NULL, NULL, &zeromask)) < 0){
			syslog(LOG_ERR, "pselect: %s", strerror(errno));
			exit(1);
		}

		if (FD_ISSET(listenfd, &rset)){
			socklen = sizeof(clientaddr);
			clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, &socklen);
			for(i = 0; i < FD_SETSIZE; i++)
				if (client[i] < 0){
					client[i] = clientfd;
					break;
				}

			FD_SET(clientfd, &allset);

			if (clientfd > maxfd)
				maxfd = clientfd;
			if (i > maxi)
				maxi = i;

			if (--nready <= 0)
				continue;
		}

		for(i = 0; i <= maxi; i++)
		{
			if (client[i] < 0)
				continue;
			if (FD_ISSET(client[i], &rset)){
				socklen = sizeof(clientaddr);
				if (getpeername(client[i], &clientaddr, &socklen) >= 0)
					inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, clientip, INET_ADDRSTRLEN);
				else{
					close(client[i]);
					FD_CLR(client[i], &allset);
					client[i] = -1;
					continue;
				}
				snprintf(sql, sizeof(sql), "select id from svr_info where ip = \"%s\"", clientip);
				if (mysql_query(&my_conn, sql)){
					syslog(LOG_ERR, "Mysql query error %d: %s", mysql_errno(&my_conn), mysql_error(&my_conn));
					exit(2);
				}
				res = mysql_store_result(&my_conn);
				if ((row = mysql_fetch_row(res)) != NULL)
					id = atoi(row[0]);
				else{
					syslog(LOG_ERR, "Not find %s in svr_info", clientip);
					close(client[i]);
					FD_CLR(client[i], &allset);
					client[i] = -1;
					continue;
				}

				mysql_free_result(res);
				if ((n = Read(client[i], (char *)&ret, sizeof(ret))) < 0){
					syslog(LOG_ERR, "read error code :%i", errno);
					close(client[i]);
					FD_CLR(client[i], &allset);
					client[i] = -1;
					continue;
				}

				if (errno == EHOSTDOWN){
					close(client[i]);
					FD_CLR(client[i], &allset);
					client[i] = -1;
				}
//				syslog(LOG_NOTICE, "client:%s recv %i bytes", clientip, n);
				if (n > 0){
					int j;
					for(j = 0; j < n / sizeof(xvalue); j++){
						if (inet_ntop(AF_INET, &ret[j].v.sip, sip, INET_ADDRSTRLEN) == NULL){
							syslog(LOG_ERR, "inet_ntop: %s", strerror(errno));
							continue;
						}
						if ((time = getcurtime()) == NULL){
							syslog(LOG_ERR, "Get cur time error"); exit(3);
						}

						if (ret[0].v.other == 0){
							if (j == 0)
								snprintf(sql, sizeof(sql), "insert into external(svr_id, sip, packs, bytes, updatetime) values(%u, \"0.0.0.0\", %u, %lu, \"%s\")",
										id, ret[0].fpacket, ret[0].fbytes, time);
								//snprintf(sql, sizeof(sql), "insert into external(svr_id, sip, packs, fpacks, rpacks, updatetime, bytes, rbytes, fbytes) values(%u, \"0.0.0.0\", %u, %u, %u, \"%s\", %lu, %lu, %lu)",
								//	                                             id, sip, ret[j].v.packets, ret[j].fpacket, ret[j].v.packets-ret[j].fpacket, time, ret[j].v.bytes, ret[j].v.bytes-ret[j].fbytes, ret[j].fbytes);
							else
								snprintf(sql, sizeof(sql), "insert into external(svr_id, sip, packs, fpacks, rpacks, tcp, udp, icmp, other, updatetime, bytes, rbytes, fbytes) values(%u, \"%s\", %u, %u, %u, %u, %u, %u, %u, \"%s\", %lu, %lu, %lu)",
									id, sip, ret[j].v.packets, ret[j].fpacket, ret[j].v.packets - ret[j].fpacket, ret[j].v.tcp, ret[j].v.udp, ret[j].v.icmp, ret[j].v.other, time, ret[j].v.bytes, ret[j].v.bytes - ret[j].fbytes, ret[j].fbytes);
						}
						else if (ret[0].v.other == 1){
							if (j == 0)
								snprintf(sql, sizeof(sql), "insert into internal(svr_id, sip, packs, bytes, updatetime) values(%u, \"0.0.0.0\", %u, %lu, \"%s\")",
										id, ret[0].fpacket, ret[0].fbytes, time);
								//snprintf(sql, sizeof(sql), "insert into internal(svr_id, sip, packs, fpacks, rpacks, updatetime, bytes, rbytes, fbytes) values(%u, \"0.0.0.0\", %u, %u, %u, \"%s\", %lu, %lu, %lu)",
								//	                                             id, sip, ret[j].v.packets, ret[j].fpacket, ret[j].v.packets-ret[j].fpacket, time, ret[j].v.bytes, ret[j].v.bytes-ret[j].fbytes, ret[j].fbytes);
							else
								snprintf(sql, sizeof(sql), "insert into internal(svr_id, sip, packs, fpacks, rpacks, tcp, udp, icmp, other, updatetime, mth, bytes, rbytes, fbytes) values(%u, \"%s\", %u, %u, %u, %u, %u, %u, %u, \"%s\", \"%s\", %lu, %lu, %lu)",
									id, sip, ret[j].v.packets, ret[j].fpacket, ret[j].v.packets - ret[j].fpacket, ret[j].v.tcp, ret[j].v.udp, ret[j].v.icmp, ret[j].v.other, time, &time[17], ret[j].v.bytes, ret[j].v.bytes - ret[j].fbytes, ret[j].fbytes);
						}
						free(time);
						//syslog(LOG_NOTICE, sql);
						if (mysql_query(&my_conn, sql)){
							syslog(LOG_ERR, "Mysql query error %d: %s", mysql_errno(&my_conn), mysql_error(&my_conn)); exit(2);
						}
					}
				}
				if (--nready <= 0)
					break;
			}
		}
	}
	mysql_close(&my_conn);
}

int main(void)
{
	pid_t mypid1, mypid2;
	int n;
	openlog("sniffersvr",LOG_PID, LOG_LOCAL7);
	umask(0);
	if ((mypid1 = fork()) == -1){
		syslog(LOG_ERR, "fork: %s", strerror(errno)); exit(1);
	}
	if (mypid1 > 0) exit(0);
	setsid();
	signal(SIGHUP, SIG_IGN);
	if ((mypid2 = fork()) == -1){
		syslog(LOG_ERR, "fork: %s", strerror(errno)); exit(1);
	}
	if (mypid2 > 0) exit(0);
	chdir("/");
	for(n = 0; n < 1025; n++)
		close(n);
	open("/dev/null", O_RDWR);
	dup(0); dup(0);
	work();
	closelog();
	return 0;
}
