/*
	tcp_cat.c ... cat buffers from TCP/IP
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Research Center for Nuclear Physics
  Created:  18-FEB-1995 by A. Tamii (for Solaris 2.3) 
            29-NOV-1996 ver 1.9 by A. Tamii   (ROUTER_INET_VERSION_1)
            30-NOV-1996 ver 2.0 by A. Tamii   (ROUTER_INET_VERSION_2)
            21-JAN-1997 ver 2.1 by A. Tamii   (Bug Fix)
            08-MAY-2000 ver 3.0 by A. Tamii   (For Linux)
            17-MAY-2000 ver 3.1 by A. Tamii   (send error reply when HUP)
            19-JAN-2018 ver 3.2 by A. Tamii   
            27-JAN-2018 ver 3.3 by A. Tamii   
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

#define BUFSIZE 0x100000    /*  Default Buffer size is 1MB */

static char title[] = "TCP Cat Ver 3.3 27-JAN-2018";
//static char title[] = "TCP Cat Ver 3.2 19-JAN-2018";
//static char title[] = "TCP Cat Ver 3.1 17-MAY-2000";
char     *filename;
unsigned char *zero = (unsigned char*)NULL;
int  sd;
int  output_bufsize = BUFSIZE;

#define DEF_NBYTES 16

void usage(){
	fprintf(stderr, "%s\n", title);
	fprintf(stderr, "Retrieve data via TCP/IP (in sampling mode)\n");
	fprintf(stderr,
				"  Usage:  %s [-p port#] [-b blksize] host_name user_name router\n",
					filename);
  fprintf(stderr,
			"         host_name  ... remote host name by name of ip address.\n");
  fprintf(stderr,
			"         user_name  ... user name on the remote host.\n");
  fprintf(stderr,
			"         router     ... router name on the remote host. If a number, the .bld file of the specified run number will be taken \n");
  fprintf(stderr,
			"  Option: -p port#  ... specify the TCP/IP port number on the host\n");
  fprintf(stderr,
			"          -b blksize ... output block size in kB (default = %d kB)\n", BUFSIZE/1024);
  fprintf(stderr,
			"          -v         ... variable length mode of the block size\n");
}

static void cleanup(sig)
		 int  sig;
{
	int reply;
	/* send an acknowledge */
	reply = ntohl(-1);
  write(sd, &reply, sizeof(reply));
	
  shutdown(sd,2);
  close(sd);
	exit(0);
}

long nbytes;

void dump(buf, size)
		 unsigned short   *buf;
		 long   size;
{
	int    i, j, n;
	n = ((nbytes<=size ? nbytes : size)+1)>>1;
	for(i=0; i<n;){
		if(i==0)
			printf("Buf=%6x, Size=%6x, Data=", buf, size);
		else
			printf("                           %4x:", i<<1);
		for(j=0;j<8 && i<n; j++, i++)
			printf("%.4x ", htons(*buf++));
		printf("\n");
	}
}

main(argc, argv)
		 int    argc;
		 char   *argv[];
{
	int  size;
	struct sockaddr_in name;
  struct servent *serv_p;
	struct hostent *hp;
  rtr_inet_info1_t  info1;
	rtr_inet_info2_t  info2;
	rtr_inet_info3_t  info3;
  int    bufsize;
	char   *buf;
  int    i;
  int    reply;
  int    count, n;
  char   *ptr;
  char   str[256];
	char   router[10];
	int    v_flag = 0;
	int    bld_run_number = 0;
	
	filename = *argv++;
	argc--;
	
	nbytes = DEF_NBYTES;

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
		case 'v':
			v_flag = 1;
			break;
		case 'b':
			argv ++;
			argc --;
			output_bufsize = atoi(*argv)*1024;
			break;
		default:
			fprintf(stderr, "Unknown flag '%s'.\n",argv[0]);
			usage();
			exit(1);
		}
		argc--;
		argv++;
	}
	
	if(argc<3 || 3<argc){
		fprintf(stderr, "Too many or too few arguments\n", *argv);
		usage();
		exit(1);
	}

	if((sd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0){
		perror("socket");
		exit(1);
	}

	strncpy(info1.program, filename, sizeof(info1.program));

	/* Get service port */
	if(name.sin_port==0){
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
		exit(1);
	}

	fprintf(stderr, "Connected successfully.\n");

	info1.unit   = htons(0);
	info1.stream = htons(0);
  info1.mode = htonl(O_SAMPLE);
	info1.ver = htonl(ROUTER_INET_VERSION_2);
	strncpy(info1.uname, argv[1], sizeof(info1.uname));
	info1.uname[sizeof(info1.uname)-1]=0x00;

	bld_run_number = atoi(argv[2]);
	if(bld_run_number==0){
		strncpy(router, argv[2], 4);
		router[4]=0x00;
		strcat(router, "    ");
		router[4]=0x00;
		strcpy(info1.router, router);
	}else{
		strcpy(info1.router, ".bld");
		info1.stream = bld_run_number;
	}
  cuserid(str);
  if((long)cuserid(str)==(long)NULL)
		strncpy(info1.user, "Anonymous", sizeof(info1.user));
	else
		strncpy(info1.user, str, sizeof(info1.user));

	fprintf(stderr, "Try to connect to router '%4s' with username '%s'\n",
					router, info1.uname);

	if(write(sd, &info1, sizeof(info1))<0){
		perror("write");
		close(sd);
	}
	
  count = 0;
  ptr  = (char*)&info2;
	while(count<sizeof(info2)){
		if((n=read(sd, ptr, sizeof(info2)-count))<0){  /* wait for reply */
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
		fprintf(stderr, "Error Reply from the server (%d).\n",
						ntohl(info2.result));
		close(sd);
		exit(1);
	}

	if(v_flag){
    fprintf(stderr, "variable length mode of the output block size\n");
	}else{
    fprintf(stderr, "block size = %d\n", output_bufsize);
	}
  zero = (unsigned char*)calloc(sizeof(unsigned char), output_bufsize);

	for(i=1; i<16; i++)
		signal(i, cleanup);
	
	for(i=1;;i++){
		/* get info 3 */
		count = 0; bufsize = sizeof(info3);
		while(1){
			if((size = read(sd, &info3, bufsize-count))<0){
				close(sd);
				perror("read");
				exit(2);
			} else if(size==0) break;
			count += size;
			if(count>=bufsize) break;
		}
		if(size==0) break;
#if 0
		fprintf(stderr, "Block Number = %d\n", ntohl(info3.blk_count));
		fprintf(stderr, "Block Size   = %d\n", ntohl(info3.blk_size));
#endif

		/* allocate memory */
		bufsize = ntohl(info3.blk_size);
		buf = (char*)malloc(bufsize);
		if(buf==(char*)NULL){
			fprintf(stderr, "Could not allocate buffer (size=%d).\n", bufsize);
			break;
		}
	
		/* read a block */
		count = 0;
		while(1){
			if((size = read(sd, &buf[count], bufsize-count))<0){
				close(sd);
				perror("read");
				exit(2);
			} else if(size==0) break;
			count += size;
			if(count>=bufsize) break;
		}
		if(size==0) break;

		/* Output buffer */
		if(v_flag){
			if(write(1, buf, bufsize)<0){
					close(sd);
					perror("write");
					exit(2);
			}
		}else	if(bufsize <= output_bufsize){
			write(1, buf, bufsize);
			if(write(1, zero, output_bufsize-bufsize)<0){
				close(sd);
				perror("write");
				exit(2);
			}
		}else{
			fprintf(stderr, "Received size (%x) is larger than the output block size (%x).\n",
							bufsize, output_bufsize);
			if(write(1, buf, output_bufsize)<0){
				close(sd);
				perror("write");
				exit(2);
			}
		}

		/* send an acknowledge */
		reply = ntohl(0);
		if(write(sd, &reply, sizeof(reply))<0){
			close(sd);
			perror("write");	
			exit(2);
		}

		/* free buffer */
		free(buf);
		buf = (char*)NULL;
	}
	fprintf(stderr, "Close the connection.\n");
	close(sd);
	free(buf);
}

/*
  Local Variables:
  mode: C
  tab-width: 2
  End:
*/
