/*
	tcp_cat.c ... Cat buffers from TCP/IP
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  18-FEB-1995 by A. Tamii (for Solaris 2.3) 
            29-NOV-1996 ver 1.9   by A. Tamii  (ROUTER_INET_VERSION_1)
            20-JAN-1997 ver 1.95  by A. Tamii  (Bug Fix)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>

#include "router.h"

main(argc, argv)
		 int    argc;
		 char   *argv[];
{
	int  size, sd;
	struct sockaddr_in name;
  struct servent *serv_p;
	struct hostent *hp;
  rtr_inet_info1_t  info1;
	rtr_inet_info2_t  info2;
  int    bufsize;
	char   *buf;
  int    i;
  int    reply;
  int    count, n;
  char   *ptr;
  char   str[256];
	
	if(argc<3){
		fprintf(stderr, "%s --- cat buffers from TCP/IP\n", argv[0]);
		fprintf(stderr, "Usage: %s [-p port#] host_name [unit# (default=0)] stream\n", argv[0]);
		fprintf(stderr, "       -p port# ... specify the TCP/IP port number on the host\n");
		exit(1);
	}
	if((sd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0){
		perror("socket");
		exit(1);
	}

	strncpy(info1.program, *argv, sizeof(info1.program));
	argv++;
	argc--;

	/* Get service port */
	if(argc>2 && !strcmp(argv[0],"-p")){
		name.sin_port = htons(atoi(argv[1]));
		argv += 2;
		argc -= 2;
	}else{
		if((serv_p=getservbyname(ROUTER_DAQDATA_SERVICE, "tcp"))==NULL){
			fprintf(stderr, "Cannot find service %s. Use the default port.\n",
							ROUTER_DAQDATA_SERVICE);
			name.sin_port = htons(ROUTER_DAQDATA_PORT);
		}else{
			name.sin_port = htons(serv_p->s_port);
		}
	}
	name.sin_family = AF_INET;
	if((hp=gethostbyname(*argv))==NULL){
		fprintf(stderr, "Cannot find the host %s.\n", *argv);
		exit(1);
	}
	strncpy((char*)&name.sin_addr.s_addr, hp->h_addr, hp->h_length);

	fprintf(stderr, "Try to connect to %s port %d.\n",
					*argv, ntohs(name.sin_port));
	if(connect(sd, (struct sockaddr*)&name, sizeof(name))<0){
		close(sd);
		perror("connect");
		exit(2);
	}

	fprintf(stderr, "Connected successfully.\n");

	argv++;
	argc--;
	
	if(argc<=1){
		info1.unit   = htons(0);
		info1.stream = htons(atoi(*argv));
		argv++;
		argc--;
	}else{
		info1.unit   = htons(atoi(argv[0]));
		info1.stream = htons(atoi(argv[1]));
		argv+=2;
		argc-=2;
	}
#if 0
  info1.mode = htonl(0);
#else
  info1.mode = htonl(O_SAMPLE);
#endif
	info1.ver = htonl(ROUTER_INET_VERSION_1);
  cuserid(str);
  if((long)cuserid(str)==(long)NULL)
		strncpy(info1.user, "Anonymous", sizeof(info1.user));
	else
		strncpy(info1.user, str, sizeof(info1.user));

	if(write(sd, &info1, sizeof(info1))<0){
		perror("write");
		close(sd);
	}
	
  count = 0;
  ptr  = (char*)&info2;
	while(count<sizeof(info2)){
		if((n=read(sd, ptr, sizeof(info2)-count))<0){  /* wait for the reply */
			perror("read");
			return;
		}
		if(n==0){
			fprintf(stderr, "Connection was closed from the server.\n");
			return;
		}
		count += n;
	}

	if(ntohl(info2.result)!=E_NOERR){
		fprintf(stderr, "Error Reply from the server (%d).\n", ntohl(info2.result));
		close(sd);
		exit(1);
	}

	bufsize = ntohl(info2.blk_size);
fprintf(stderr, "Bufsize = %xH\n", bufsize);
	buf = (char*)malloc(bufsize);
	if(buf==(char*)NULL){
		fprintf(stderr, "Cannot allocate buffer (size=%d).\n", bufsize);
		close(sd);
		exit(1);
	}
	
	for(i=1;;i++){
		count = 0;
		while(1){
			if((size = read(sd, &buf[count], bufsize-count))<0){
				close(sd);
				perror("read");
				exit(1);
			} else if(size==0) break;
			count += size;
			if(count>=bufsize) break;
		}
		if(size==0) break;
		write(1, buf, bufsize);
		reply = 0;
		if(write(sd, &reply, sizeof(int))<0){
			close(sd);
			perror("write");
			exit(1);
		}
	}
	fprintf(stderr, "Close the connection.\n");
	close(sd);
	free(buf);
}

