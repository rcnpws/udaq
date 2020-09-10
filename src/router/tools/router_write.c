/*
	router_write.c ... Write buffer to Router
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  30-OCT-1996 by A. Tamii (for Solaris 2.3)
  Ver 2.1   20-MAY-2000 by A. Tamii (Add sleep)
  Ver 2.2   25-MAY-2000 by A. Tamii (Add -b option)
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "router.h"

#define BUFSIZE 0x4000

char title[] = "Router Write Ver 2.2 25-MAY-2000";
#if 0
char title[] = "Router Write Ver 2.0d 30-OCT-1996";
#endif
char *filename;

void usage(){
	fprintf(stderr, "%s\n", title);
	fprintf(stderr, "  ... write data to router\n", filename);
	fprintf(stderr, "Usage: %s [-s time] router [file_name]\n", filename);
  fprintf(stderr, "       router ... name of the router\n");
  fprintf(stderr, "       -s time .. sleep time in msec per buffer\n");
  fprintf(stderr,
					"       -b size .. write block size in kbytes (default = %d)\n",
					BUFSIZE/1024);
  fprintf(stderr, "       file_name ... input file name (default=stdout)\n");
}

main(argc, argv)
		 int    argc;
		 char   *argv[];
{
	int    res;
	char   *buf, *data;
	long   size, k, count;
	int    i, n;
	int    fd_read;
	int    fd_write;
  int    bufsize;
  int    st;  /* sleep time in usec */
	
	filename = argv[0];
  st = 0;

  argc--;
	argv++;
  bufsize = BUFSIZE;

	while(argc>0){
		if(**argv!='-')break;
		if(strlen(*argv)==2){
			switch((*argv)[1]){
			case 'h':
			case 'H':
				usage();
				exit(0);
			case 's':
        argv++;
        argc--;
        st = atoi(*argv)*1000;
        break;
			case 'b':
        argv++;
        argc--;
        bufsize = atoi(*argv)*1024;
        break;
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

  /* open target file */
	fd_read = 0;           /* standard input */
	if(argc>=2){
		fd_read = open(argv[1], O_RDONLY, NULL);
		if(fd_read==-1){
			fprintf(stderr, "Error on opening file %s.\n", *argv);
			perror("open");
			exit(1);
		}
	}

	if(strlen(argv[0])>4) argv[0][4] = 0x00;
  fprintf(stderr, "Write data to router '%s' with block size %d kBytes \n",
					argv[0], bufsize/1024);
	fprintf(stderr, "Connect to router '%s'.\n", argv[0]);
	
	/* Make Connection to Router */
	fd_write = router_connect(argv[0], getuid(), filename, O_WRONLY);
  if(fd_write<0){
		perror("router_connect");
		exit(1);
	}
	
	/* Write to Reflective Memory */
	count = 0;
	srand48(0);
	while(1){
#if 0
		do{
			size = ((lrand48() & 0xFFFF)>>2)<<2;
		}while(size<=0);
		res = router_request_buf(fd_write, size);
#else
		res = router_request_buf(fd_write, bufsize);
#endif
		if(res){
			perror("router_request_buf");
			break;
		}
		res = router_get_buf(fd_write, &buf, &size);
		if(res){
			perror("router_get_buf");
			break;
		}
#if 0
		fprintf(stderr, "buf = %d, size = %d\n", buf, size);
#endif
    size = 0;
    while(size<bufsize){
  		k = read(fd_read, &buf[size], bufsize-size);
  		if(k==-1){
  			perror("read");
  			break;
  		}		
      size += k;
		}
 		res = router_release_buf(fd_write, buf, size);
		if(res){
			perror("router_release_buf");
			break;
		}
    if(size<bufsize) break;
    usleep(st);
 		count += size;
	}
	fprintf(stderr, "Write %d bytes.\n", count);
	router_disconnect(fd_write);
}

