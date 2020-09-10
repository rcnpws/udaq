/*
	tcp_lib.c ... DAQ TCP library
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  08-MAY-1995 by A. Tamii
	          20-JAN-1997 by A. Tamii (Bug Fix)
	          19-JUL-1997 by A. Tamii (Bug Fix)
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
#include <unistd.h>

#include "router.h"

static int  sd;
static int  bufsize;

/*--- show error messages ---*/
static int tcp_showerr_info2(int errorno)
{
	switch(errorno){
	case E_NOERR:
		fprintf(stderr, "stdlib: No error.\n");
		break;
	case E_VERSION:
		fprintf(stderr, "stdlib: Client version is too high.\n");
		break;
	case E_INVAL:
		fprintf(stderr, "stdlib: Invalid connection information.\n");
		break;
	case E_NODEV:
		fprintf(stderr, "stdlib: No router process to connect.\n");
		break;
	default:
		fprintf(stderr, "stdlib: Unkown error.\n");
		break;
	}
	return 0;
}

/*--- open tcp connection ---*/
int tcp_open(host, port, stream, mode, proc_name, buf_size)
		 char *host;       /* server host name (DNS name or IP Address) */
		 int  port;        /* port number (if port==0, default port is used) */
		 int  stream;      /* stream number on RFM (default=1) */
		 int  mode;        /* O_ALL (=0x00) or O_SAMPLE (=0x08) */
		 int  *buf_size;   /* buffer size (returned from this procedure) */
		 char *proc_name;  /* client program name */
{
	struct sockaddr_in name;
  struct servent *serv_p;
	struct hostent *hp;
  rtr_inet_info1_t  info1;
	rtr_inet_info2_t  info2;
	char   *buf;
  int    i;
  int    count, n;
  char   *ptr;
  char   str[256];

	/* get services port */
	if(port==0){
		if((serv_p=getservbyname(ROUTER_DAQDATA_SERVICE, "tcp"))==NULL){
			fprintf(stderr, "tcplib: Couldn't find service %s.\n",
							ROUTER_DAQDATA_SERVICE);
			fprintf(stderr, "tcplib: Trying the default port (%d).\n",
							ROUTER_DAQDATA_PORT);
			port = ROUTER_DAQDATA_PORT;
		}else{
			port = serv_p->s_port;
		}
	}
	name.sin_port = htons(port);

	/* get server IP address */
	name.sin_family = AF_INET;
	if((hp=gethostbyname(host))==NULL){
		fprintf(stderr, "tcplib: Couldn't find the host %s.\n", host);
		return -1;
	}
	strncpy((char*)&name.sin_addr.s_addr, hp->h_addr, hp->h_length);

	/* connect to server */
	for(i=0; i<5; i++){
		fprintf(stderr, "tcplib: Trying to connect to %s at port %d...\n",
					host, ntohs(name.sin_port));
		/* open tcp socket */
		if((sd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0){
			perror("tcplib: socket");
			return -1;
		}
		if(connect(sd, (struct sockaddr*)&name, sizeof(name))==0)
			break;
		perror("tcplib: connect");
		name.sin_port = htons(ntohs(name.sin_port)+1);
		close(sd);
	}
	if(i>=5){
		fprintf(stderr, "Couldn't connect to the host.\n");
		return -1;
	}
	fprintf(stderr, "tcplib: Connected successfully.\n");

	/* prepare information to the server */
	strncpy(info1.program, proc_name, sizeof(info1.program));
	info1.unit   = htons(0);
	info1.stream = htons(stream);
	info1.mode = htonl(mode);
	info1.ver = htonl(ROUTER_INET_VERSION_1);
/*	info1.res1 = htonl(0); */
  cuserid(str);
  if((long)cuserid(str)==(long)NULL)
		strncpy(info1.user, "Anonymous", sizeof(info1.user));
	else
		strncpy(info1.user, str, sizeof(info1.user));
/*  strcpy(info1.res2, ""); */

	/* send information to the server */
	if(write(sd, &info1, sizeof(info1))<0){
		perror("tcplib: write");
		close(sd);
	}
	
	/* wait for response */
  count = 0;
  ptr  = (char*)&info2;
	while(count<sizeof(info2)){
		if((n=read(sd, ptr, sizeof(info2)-count))<0){  /* wait for the reply */
			perror("tcplib: read");
			return -1;
		}
		if(n==0){
			fprintf(stderr, "tcplib: Connection was closed from the server.\n");
			return -1;
		}
		count += n;
	}
	
	/* check the reply from the server */
	i = ntohl(info2.result);
	if(i!=E_NOERR){
		fprintf(stderr, "tcplib: Error Reply from the server (%d).\n", i);
		tcp_showerr_info2((int)i);
		close(sd);
		return -1;
	}

	/* set block size */
	*buf_size = bufsize = ntohl(info2.blk_size);
	return 0;
}

/*--- get one block ---*/
int tcp_get_buf(buf)
	char *buf;
{
	int  count, size;	
  int    reply;

	/* get a block from the server */
	count = 0;
	while(1){
		if((size = read(sd, &buf[count], bufsize-count))<0){
			perror("tcplib: read");
			return -1;
		} else if(size==0) break;
		count += size;
		if(count>=bufsize) break;
	}
	if(count==0) return 1;

	/* send respose to the server */
	reply = 0;
	if(write(sd, &reply, sizeof(int))<0){
		perror("tcplib: write");
		return(-1);
	}

  return 0;
}

/*--- close tcp connetion ---*/
int tcp_close(){
	fprintf(stderr, "tcplib: Close the connection.\n");
	close(sd);
	return 0;
}

/******--- Fortran Interfaces ---*****/
int tcp_open_(host, port, stream, mode, proc_name, buf_size)
		 char *host;       /* server host name (DNS name or IP Address) */
		 int  *port;       /* port number (if port==0, default port is used) */
		 int  *stream;     /* stream number on RFM (default=1) */
		 int  *mode;       /* O_ALL (=0x00) or O_SAMPLE (=0x08) */
		 int  *buf_size;   /* buffer size (returned from this procedure) */
		 char *proc_name;  /* client program name */
{
	return tcp_open(host, *port, *stream, *mode, proc_name, buf_size);
}

int tcp_get_buf_(buf)
	char *buf;
{
	return tcp_get_buf(buf);
}

int tcp_close_(){
	return tcp_close();
}

/*****--- Old Fortran Interfaces ---******/
int tcpopen_(host, stream, mode, buf_size)
		 char  *host;
		 int   *stream;
		 int   *mode;
		 int  *buf_size;
{
	return tcp_open(host, 0, *stream, *mode, "TCPLIB_FORT", buf_size);
}

int tcpgtbuf_(buf)
	char *buf;
{
	return tcp_get_buf(buf);
}

int tcpclose_(){
	return tcp_close();
}

/*
  Local Variables:
  mode: C
  tab-width: 2
  End:
*/
