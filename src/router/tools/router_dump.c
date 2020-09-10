/*
	router_dump.c ... Dump buffer from router
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  24-JAN-1995 by A. Tamii (for Solaris 2.3)
  Version 2.1  24-SEP-1999 by A. Tamii (change the output format)
  Version 2.2  30-SEP-2006 by A. Tamii (align the output format)
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "router.h"

#define DEF_NBYTES 16

char title[] = "Router Dump Ver 2.2 30-SEP-2006";
#if 0
char title[] = "Router Dump Ver 2.1 24-SEP-1999";
char title[] = "Router Dump Ver 2.0d 30-OCT-1996";
#endif
char *filename;

void usage(){
	fprintf(stderr, "%s\n", title);
	fprintf(stderr, "  ... dump the top of buffers from router\n", filename);
	fprintf(stderr, "Usage: %s router [size]\n", filename);
  fprintf(stderr, "       router ... name of the router\n");
  fprintf(stderr, "       size   ... number of bytes to dump (HEX)\n");
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
			printf("Buf=%6x, Size=%6x, Addr=%.4x: ", buf, size, i<<1);
		else
			printf("                                %.4x: ", i<<1);
		for(j=0;j<8 && i<n; j++, i++)
			printf("%.4x ", htons(*buf++));
		printf("\n");
	}
}

main(argc, argv)
		 int    argc;
		 char   *argv[];
{
	int    fd;
	int    res;
	unsigned short  *buf;
	long   size;
	
	filename = argv[0];

  argc--;
	argv++;

	while(argc>0){
		if(**argv!='-')break;
		if(strlen(*argv)==2){
			switch((*argv)[1]){
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

	if(argc<1 || 2<argc){
		fprintf(stderr, "Too many or too few arguments\n", *argv);
		usage();
		exit(1);
	}

	if(argc>=2)
		sscanf(argv[1], "%x", &nbytes);
	else
		nbytes = DEF_NBYTES;

  /* show title */
	fprintf(stderr, "%s\n", title);
	if(strlen(argv[0])>4) argv[0][4] = 0x00;
	fprintf(stderr, "Connect to router '%s'.\n", argv[0]);
	
	/* Make Connection to Router */
#if 1
	fd = router_connect(argv[0], getuid(), filename, 0);
#else
	fd = router_connect(argv[0], getuid(), filename, O_SAMPLE);
#endif
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
		dump(buf, size);
		res = router_release_buf(fd, buf, size);
		if(res){
			perror("router_get_buf");
			break;
		}
	}
	router_disconnect(fd);
}

/*
Local Variables:
mode: C
tab-width: 2
End:
*/
