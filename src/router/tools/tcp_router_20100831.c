/*
	tcp_router.c ... Get buffers from TCP/IP and pass it to Router
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  18-FEB-1995 by A. Tamii (for Solaris 2.3) 
            29-NOV-1996 ver 1.9  by A. Tamii   (ROUTER_INET_VERSION_1)
            30-NOV-1996 ver 2.0  by A. Tamii   (ROUTER_INET_VERSION_2)
            30-NOV-1996 ver 2.1  by A. Tamii   (tcp_save)
            21-JAN-1997 ver 2.2  by A. Tamii   (Bug Fix)
            21-JAN-1997 ver 2.3  by A. Tamii   (tcp_router)
            05-JAN-2001 ver 2.4  by A. Tamii   (Bug Fix for info3 read error)
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
#include "errlog.h"

#define PROCNAME "tcp_route"

#define FIXEDLENGTH       1
#define PRE_ACKNOWLEDGE   1

#if FIXEDLENGTH
#define BUFSIZE  16384
#endif

static char title[] = "TCP Router Ver 2.4 05-JAN-2001";
#if 0
static char title[] = "TCP Router Ver 2.3 21-JAN-1997";
#endif

char     *filename;

void usage(){
	fprintf(stderr, "%s\n", title);
	fprintf(stderr, "Get buffers from TCP/IP and write it to a local router\n");
	fprintf(stderr,
			"  Usage:  %s [-p port#] host_name user_name src_router dst_router\n",
					filename);
  fprintf(stderr,
			"         host_name  ... remote host name by name of ip address.\n");
  fprintf(stderr,
			"         user_name  ... user name on the remove host.\n");
  fprintf(stderr,
			"         src_router ... router name on the remote host.\n");
  fprintf(stderr,
			"         dst_router ... router name on the local host.\n");
  fprintf(stderr,
			"  Option: -p port#  ... specify the TCP/IP port number on the host\n");
}

main(argc, argv)
		 int    argc;
		 char   *argv[];
{
	int  sd;
	struct sockaddr_in name;
  struct servent *serv_p;
	struct hostent *hp;
  rtr_inet_info1_t  info1;
	rtr_inet_info2_t  info2;
	rtr_inet_info3_t  info3;
  long   bufsize;
	char   *buf;
	long   size;
  int    i, j;
	int    res;
  int    reply;
  int    count, n;
	int    fd_write;
  char   *ptr;
  char   str[256];
	char   router[10];
	
	filename = *argv++;
	argc--;
	
	/* check the arguments */
	name.sin_port = htons(0);
	while(argc>0 && argv[0][0]=='-'){
		if(strlen(argv[0])!=2){
			fprintf(stderr, "Unknown flag '%s'.\n",argv[0]);
			usage();
			exit(1);
		}
		switch(argv[0][1]){
		case 'p':
			if(argc<=1){
				fprintf(stderr, "Port number was not specified after '-p'.\n");
				usage();
				exit(1);
			}
			name.sin_port = htons(atoi(argv[1]));
			argv ++;
			argc --;
			break;
		case 'h':
		case 'H':
			usage();
			exit(0);
		default:
			fprintf(stderr, "Unknown flag '%s'.\n",argv[0]);
			usage();
			exit(1);
		}
		argc--;
		argv++;
	}
	
	if(argc<4 || 4<argc){
		fprintf(stderr, "Too many or too few arguments\n", *argv);
		usage();
		exit(1);
	}

	if((sd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0){
		perror("socket");
		exit(1);
	}

	/* Open Error Log */
	errlog_open(PROCNAME);
	msgout(ErrMessage, "%s", title);

	/* Make Connection to Router */
	if(strlen(argv[3])>4) argv[3][4] = 0x00;
	msgout(ErrComment, "Write data to router '%s'", argv[3]);
	fd_write = router_connect(argv[3], getuid(), filename, O_WRONLY);
  if(fd_write<0){
		msgout(ErrSever, "Could not connect to the router %s. %s",
					 argv[3], strerror(errno));
		exit(1);
	}
	msgout(ErrComment, "Connected to router '%s'.", argv[3]);
	

	/* Get program name */
	strncpy(info1.program, filename, sizeof(info1.program));

	/* Get service port */
	if(name.sin_port==htons(0)){
		if((serv_p=getservbyname(ROUTER_DAQDATA_SERVICE, "tcp"))==NULL){
			msgout(ErrWarning, "Cannot find service %s. Use the default port.",
							ROUTER_DAQDATA_SERVICE);
			name.sin_port = htons(ROUTER_DAQDATA_PORT);
		}else{
			name.sin_port = htons(serv_p->s_port);
		}
	}
	name.sin_family = AF_INET;
	if((hp=gethostbyname(*argv))==NULL){
		msgout(ErrSever, "Cannot find the host %s.", *argv);
		exit(1);
	}
	strncpy((char*)&name.sin_addr.s_addr, hp->h_addr, hp->h_length);

	msgout(ErrComment, "Try to connect to %s port %d.",
					*argv, ntohs(name.sin_port));
	if(connect(sd, (struct sockaddr*)&name, sizeof(name))<0){
		close(sd);
		perror("connect");
		exit(2);
	}

	msgout(ErrComment, "Connected successfully.");

	info1.unit   = htons(0);
	info1.stream = htons(0);
  info1.mode = htonl(O_ALL);
	info1.ver = htonl(ROUTER_INET_VERSION_2);
	strncpy(info1.uname, argv[1], sizeof(info1.uname));
	info1.uname[sizeof(info1.uname)-1]=0x00;
	strncpy(router, argv[2], 4);
	router[4]=0x00;
	strcat(router, "    ");
	router[4]=0x00;
	strcpy(info1.router, router);
  cuserid(str);
  if((long)cuserid(str)==(long)NULL)
		strncpy(info1.user, "Anonymous", sizeof(info1.user));
	else
		strncpy(info1.user, str, sizeof(info1.user));

	msgout(ErrComment, "Try to connect to router '%4s' with username '%s'",
					router, info1.uname);

	if(write(sd, &info1, sizeof(info1))<0){
		msgout(ErrSever, "Error in write(). %s", strerror(errno));
		close(sd);
	}
	
  count = 0;
  ptr  = (char*)&info2;
	while(count<sizeof(info2)){
		if((n=read(sd, ptr, sizeof(info2)-count))<0){  /* wait for reply */
			msgout(ErrSever, "Error in read(). %s", strerror(errno));
			return;
		}
		if(n==0){
			msgout(ErrWarning, "Connection was closed from the server.");
			return;
		}
		count += n;
	}

	if(ntohl(info2.result)!=E_NOERR){
		msgout(ErrSever, "Error Reply from the server (%d).",
						ntohl(info2.result));
		close(sd);
		exit(1);
	}

#if PRE_ACKNOWLEDGE
	/* send extra acknowledges prior to buffers (in order to be faster) */
	for(i=0; i<20; i++){
		reply = htonl(0);
		if(write(sd, &reply, sizeof(reply))<0){
			close(sd);
			msgout(ErrSever, "Error in write(). %s\n", strerror(errno));
			exit(1);
		}
	}
#endif

	for(i=1;;i++){
		/* get info 3 */
		count = 0; bufsize = sizeof(info3);
		while(1){
#if 1 /* Bug fix on 05-JAN-2001 by A. Tamii */
			if((size = read(sd, &((char*)&info3)[count], bufsize-count))<0){
#else /* bug */
			if((size = read(sd, &info3, bufsize-count))<0){
#endif
				close(sd);
				msgout(ErrSever, "Error in read(). %s", strerror(errno));
				exit(1);
			} else if(size==0){
				msgout(ErrSever, "Warning: read data size is 0 (info3).");
				break;
			}
			count += size;
      if(count>bufsize){
				msgout(ErrSever, "Warning: count>bufsize (info3).");
			}
			if(count>=bufsize) break;
		}
		if(size==0){
			msgout(ErrSever, "Received header size is zero.");
			/* break;*/
		}
#if 0
		fprintf(stderr, "Block Number = %d\n", ntohl(info3.blk_count));
		fprintf(stderr, "Block Size   = %d\n", ntohl(info3.blk_size));
#endif

		/* request buffer */
		bufsize = ntohl(info3.blk_size);
#if 0
		fprintf(stderr, "bufsize = %xH\n", bufsize);
#endif
#if 1
#if 0
		if(bufsize<=0 || bufsize>0x4000){
#else
		if(bufsize!=0x4000){
#endif
			fprintf(stderr, "tcp_router: bufsize error: bufsize = %xH\n", bufsize);
			msgout(ErrSever, "tcp_router: Buffer size error: bufsize = %xH", bufsize);
			bufsize = 0x4000;
			for(j=0; j<16; j++){
				fprintf(stderr, "info3[%2x] = %4xH\n", j, ((int *)&info3)[j]);
			}
		}
#endif
#if FIXEDLENGTH
		res = router_request_buf(fd_write, BUFSIZE);
#else
		res = router_request_buf(fd_write, bufsize);
#endif
		if(res){
			msgout(ErrSever, "Error in router_request_buf. %s", strerror(errno));
			break;
		}

		/* get buffer */
		res = router_get_buf(fd_write, &buf, &size);
		if(res){
			msgout(ErrSever, "Error in router_get_buf. %s", strerror(errno));
			break;
		}
#if 1 /* Bug fixed on 05-JAN-2001 by A. Tamii */
		if(size<bufsize){
#else /* Bug */
		if(size>bufsize){
#endif
			msgout(ErrSever, "Output buffer size is too small");
			msgout(ErrSever, "data size = %xH, buffer size = %xH", size, bufsize);
			break;
		}

		/* read a block */
		count = 0;
		while(1){
			if((size = read(sd, &buf[count], bufsize-count))<0){
				close(sd);
				msgout(ErrSever, "Error in read(). %s", strerror(errno));
				exit(1);
			} else if(size==0){
				msgout(ErrSever, "Warning: read data size is 0 (data).");
				break;
			}
			count += size;
      if(count>bufsize){
				msgout(ErrSever, "Warning: count>bufsize (data).");
			}
			if(count>=bufsize) break;
		}
		if(size==0){
			msgout(ErrSever, "Received data size is zero.");
			/* break;*/
		}
if(count!=0x4000)
  fprintf(stderr, "size = %x\n", count);

#if FIXEDLENGTH
		bzero(&buf[bufsize], BUFSIZE-bufsize);
#endif

		/* release the block to router */
#if FIXEDLENGTH
		router_release_buf(fd_write, buf, BUFSIZE);
#else
		router_release_buf(fd_write, buf, bufsize);
#endif

		/* send an acknowledge */
		reply = ntohl(0);
		if(write(sd, &reply, sizeof(reply))<0){
			close(sd);
			msgout(ErrSever, "Error in write(). %s", strerror(errno));
			exit(1);
		}
	}
  msgout(ErrMessage, "Close the connection.");
	close(sd);
	router_disconnect(fd_write);
}

msgout(type,str,data1,data2,data3,data4)
		 int  type;
		 char *str;
		 long data1, data2, data3, data4;
{
	char text[256];
  sprintf(text, str, data1, data2, data3, data4);
	fprintf(stderr, "%s\n",text);
	errlog_msg(type, text);
}

/*
  Local Variables:
  tab-width: 2
  End:
*/
