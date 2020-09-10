/*
	router_dump.c ... Dump buffer from router
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  24-JAN-1995 by A. Tamii (for Solaris 2.3)
  Version 2.1  24-SEP-1999 by A. Tamii
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef OSF1
#include <strings.h>
#endif
#include <time.h>
#include <errno.h>

#include "router.h"

#define DEF_NBYTES 16

char title[] = "Router Check Format Ver 2.1 24-SEP-1999";
#if 0
#endif
char *filename;

void usage(){
	fprintf(stderr, "%s\n", title);
	fprintf(stderr,
					" ... check the format of the buffers received from a router\n",
					filename);
	fprintf(stderr, "Usage: %s [-h] [-s] router [interval]\n", filename);
  fprintf(stderr, "    -h       ... show this help\n");
  fprintf(stderr, "    -s       ... sampling read\n");
  fprintf(stderr, "    router   ... name of the router\n");
  fprintf(stderr, "    interval ... status display interval\n");
}


#define BUFSIZE   0x100000

main(argc, argv)
		 int    argc;
		 char   *argv[];
{
	int    fd;
	int    res;
	unsigned short  *buf;
	unsigned short  bufs[BUFSIZE/2];
	long   size;
	int    sflag;
	int    itvl;
	char   router[256];
	int    cnt;
  int    swap;

	sflag = 0;
	itvl  = 10;
	filename = argv[0];
  swap = (ntohl(0x01020304)!=0x01020304);

  argc--;
	argv++;

	while(argc>0){
		if(**argv!='-')break;
		if(strlen(*argv)==2){
			switch((*argv)[1]){
			case 's':
				sflag = 1;
				break;
			case 'h':
			case 'H':
				usage();
				exit(0);
			default:
				fprintf(stderr, "Illegal argument '%s'\n", *argv);
				usage();
				exit(1);
			}
		}else{
			fprintf(stderr, "Illegal argument '%s'\n", *argv);
			usage();
			exit(1);
		}
		argc--;
		argv++;
	}

	if(argc<1){
		fprintf(stderr, "Too many few arguments\n", *argv);
		usage();
		exit(1);
	}
	strcpy(router, *argv);
	router[4] = 0x00;
	argc--;
	argv++;

	if(argc>0){
		itvl = atoi(*argv);
		argc--;
		argv++;
	}

  /* show title */
	fprintf(stderr, "%s\n", title);
	fprintf(stderr, "Connect to router '%s'.\n", router);
	
	/* Make Connection to Router */
	if(sflag)
		fd = router_connect(router, getuid(), filename, O_SAMPLE);
	else
		fd = router_connect(router, getuid(), filename, NULL);
  if(fd<0){
		perror("router_connect");
		exit(1);
	}
	
	while(TRUE){
		res = router_get_buf(fd, &buf, &size);
		if(res){
			perror("router_get_buf");
			break;
		}
    if(swap){
			if(size>BUFSIZE){
				fprintf(stderr, "Buffer size is too large (%x).\n", size);
				size = BUFSIZE;
			}
      swab(buf, bufs, size);
			check_buf(bufs, size/2);
		}else{
			check_buf(buf, size/2);
		}
		res = router_release_buf(fd, buf, size);
		if(res){
			perror("router_get_buf");
			break;
		}
		if((--cnt)<=0){
			show_counters();
			cnt = itvl;
		}
	}
	router_disconnect(fd);
}

