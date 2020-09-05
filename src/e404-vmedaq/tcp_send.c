/*
	tcp_send.c ... Send buffers via TCP/IP
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  26-APR-1995 by A. Tamii (for Solaris 2.3)
	Ver 2.0   20-JAN-1997 by A. Tamii (Bug Fix)
	Ver 2.1   07-MAR-1997 by A. Tamii (for Degital Unix/INETVERSION_1)
	Ver 3.0   04-JUL-2014 by A. Tamii (for GR-VDC DAQ with VME)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#include "router.h"

#define BLKSIZE 16384

static char title[] = "TCP Send Ver 2.1 07-MAR-1997";
static char *filename;

struct sockaddr addr;
int addrlen;
int sd;
int fd;
int blksize;
char *buf = (char*)NULL;

void usage(){
	fprintf(stderr, "%s\n", title);
	fprintf(stderr, "  Send Data via TCP/IP\n");
	fprintf(stderr, "  Usage: %s [-b block_size] [file_name]\n", filename);
	fprintf(stderr, "  Default block size = %d Bytes\n", BLKSIZE);
	fprintf(stderr, "  Default input = stdin\n");
}

int main(argc, argv)
		 int    argc;
		 char   *argv[];
{
  void send_data();
	struct sockaddr_in name;
  struct servent *serv_p;
	int ld, reply;
	char  pname[80];
	
  filename = *argv;

	strcpy(pname, argv[0]);
	argc--;
	argv++;

	if(argc>=1 && !strcmp(argv[0], "-h")){
		usage();
		exit(1);
	}

	fprintf(stderr, "%s\n", title);

	if(argc>=2 && !strcmp(argv[0], "-b")){
		blksize = atoi(argv[1]);
		argc -= 2;
		argv += 2;
	}else
		blksize = BLKSIZE;
	fprintf(stderr, "Block Size = %d\n", blksize);
	
	if(argc>=1){
		fprintf(stderr, "Open file %s\n", argv[0]);
		fd = open(argv[0], O_RDONLY, NULL);
		if(fd<0){
			perror("open");
			exit(1);
		}
		argc--;
		argv++;
	}else
		fd = 0;

	if(argc>0){
    usage();
		exit(1);
	}

	/* Get Buffer */
	buf = (char*)malloc(blksize);
	if(buf==(char*)NULL){
		perror("malloc");
		exit(1);
	}
	
	/* Make a socket */
	if((ld=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0){
		perror("socket");
		exit(1);
	}
	
  /* Get service port */
  if((serv_p=getservbyname(ROUTER_DAQDATA_SERVICE, "tcp"))==NULL){
    fprintf(stderr, "Cannot find service %s. The default port %d is used.\n",
					 ROUTER_DAQDATA_SERVICE, ROUTER_DAQDATA_PORT);
		name.sin_port = htons(ROUTER_DAQDATA_PORT);
	}else{
		name.sin_port = serv_p->s_port;
	}
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = INADDR_ANY;
	
	/* Bind */
	while(1){
		if(bind(ld, (struct sockaddr*)&name, sizeof(name))<0){
			perror("bind");
			name.sin_port = htons(ntohs(name.sin_port)+1);
			continue;
		}
	break;
	}
	
	fprintf(stderr, "Waiting for connections at port %d.\n",
					ntohs(name.sin_port));

  /* Listen */
	if(listen(ld, 5)<0){
		close(ld);
		perror("listen");
		exit(1);
	}
	
  //signal(SIGCHLD, SIG_IGN);   /* ignore the signal SIGCHLD */
	
	/* Accept Loop */
	addrlen = sizeof(addr);
	if((sd=accept(ld, &addr, (socklen_t*)&addrlen))<0){
		close(ld);
		perror("accept");
		exit(1);
	}
	close(ld);
	send_data(sd);

  fprintf(stderr, "Waiting for the client to close the socket.\n");
  while(read(sd, &reply, sizeof(reply))<0);
  fprintf(stderr, "Done\n");

	close(sd);

	if(fd!=0)
		close(fd);
	
	if(buf!=(char*)NULL){
		free(buf);
		buf = (char*)NULL;
	}
	
	exit(0);
}

void get_buf(buf, size)
		 char *buf;
		 int  *size;
{
	int   i;
	char  *ptr;
	int   count;
	*size = 0;
	while(*size<blksize){
		count = read(fd, &buf[*size], blksize-*size);
		if(count==0){
			sleep(1);  /* I know this isn't a good way, but... */
			count = read(fd, &buf[*size], blksize-*size);
			if(count==0)
				return;
		}
		if(count<0){
			perror("read");
			return;
		}
		*size+=count;
	}
	if(*size<blksize){
		ptr = &buf[*size];
		for(i=*size; i<blksize; i++)
			*ptr++ = 0;
	}
}

void send_data()
{
  rtr_inet_info1_t  info1;
	rtr_inet_info2_t  info2;
	rtr_inet_info3_t  info3;
  struct hostent  *hp;
  struct sockaddr_in *in_p;
  int   count, n, size;
  char  *ptr;
  char  name[256];
  char  str[256];
  int   i;
  int   reply;
  long long total_size = 0;
	
	/* get client host name */
  in_p = (struct sockaddr_in*)&addr;
	hp = gethostbyaddr((char*)&in_p->sin_addr,
			 sizeof(struct in_addr), AF_INET);
  if(hp!=NULL && *hp->h_aliases!=(char*)NULL)
		strcpy(name, *hp->h_aliases);
	else
		strcpy(name, (char*)(long)inet_ntoa(in_p->sin_addr));

	fprintf(stderr, "Connection request from %s\n", name);
	
  /* read request from client */
  count = 0;
  ptr  = (char*)&info1;
	while(count<sizeof(info1)){
		if((n=read(sd, ptr, sizeof(info1)-count))<0){  /* wait for the reply */
			perror("read");
			return;
		}
	 if(n==0){
			fprintf(stderr, "Connection was closed from the client.\n");
			return;
		}
		count += n;
    ptr = &ptr[n];
	}

  fprintf(stderr, "Client: %s@%s:%s\n", info1.user, name, info1.program);

  /* check the version */
	if(ntohl(info1.ver) != ROUTER_INET_VERSION_2){
		info2.result = htonl(E_VERSION);
    write(sd, &info2, sizeof(info2));
		fprintf(stderr, "Client version %d may not be supported.\n", htonl(E_VERSION));
	}
  
  if((ptr=(char*)strchr(name, '.'))!=(char*)NULL)   /* get the host name */
		*ptr = 0x00;
	strncpy(str, (char*)info1.user, sizeof(info1.user));
	strcat(str, "@");
  strcat(str, name);
	strcat(str, ":");
	strncat(str, (char*)info1.program, sizeof(info1.program));

	info2.result = htonl(E_NOERR);
	info2.blk_size = htonl(blksize);
  info2.res1 = info2.res2 = 0;

	/* send reply */
	if(write(sd, &info2, sizeof(info2))<0){
		perror("write");
		return;
	}
	
	total_size = 0;
	for(i=0;;i++){
		get_buf(buf, &size);
		if(size<0){
			perror("get_buf");
			break;
		}

		info3.blk_count = htonl(i+1);
		info3.blk_size = htonl(size);
		if(write(sd, &info3, sizeof(info3))<0)   /* write info3 */
			perror("write");                       
		if(write(sd, buf, size)<0)               /* write data via TCP/IP */
			perror("write");  
		if(size==0){
			fprintf(stderr, "File Transfer Completed.\n");
			break;
		}
		total_size += size;

		count = 0;
		while(count<sizeof(int)){
			n = read(sd, &((char*)&reply)[count], sizeof(reply)-count);
			if(n<0){  /* wait for the reply */
				perror("read");
				break;
			}
			count += n;
		}
		reply = ntohl(reply);
		if(reply!=0){
			fprintf(stderr, "Error reply from the client(%d).\n", reply);
		}
	}

  fprintf(stderr, "Total data size: %lld bytes (%f MB).\n", total_size, total_size/1024./1024.);
  return;
}

/*
  Local Variables:
  mode: C
  tab-width: 2
  End:
*/


