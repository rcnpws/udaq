/*
	router_check.c ... check raw data
  Copyright (C) 1999  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Version 2.1  24-SEP-1999 by A. Tamii
  Version 2.2  21-DEC-1999 by A. Tamii  Add PCOS
  Version 2.21 21-DEC-1999 by A. Tamii  (Change SSD_TDC GSO_TDC messages)
  Version 2.22 30-SEP-2006 by A. Tamii  (Change A-Z of -w expression)
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

char title[] = "Router Check Format Ver 2.22 30-SEP-2006";
#if 0
char title[] = "Router Check Format Ver 2.21 22-JUN-2000";
char title[] = "Router Check Format Ver 2.2 21-DEC-1999";
char title[] = "Router Check Format Ver 2.1 24-SEP-1999";
#endif
char *filename;

extern int show_inpreg_itvl;
extern int show_fera_itvl;
extern int show_3377_itvl;
extern int show_pcos_itvl;
extern int dump_flag;
extern int wire_flag;
extern int pcos_flag;

void usage(){
	fprintf(stderr, "%s\n", title);
	fprintf(stderr,
					" ... check the format of the buffers received from a router\n",
					filename);
	fprintf(stderr,
					"Usage: %s [-h] [-n] [d] [-f] [-3] router [interval]\n", filename);
  fprintf(stderr, "    -h       ... show this help\n");
  fprintf(stderr, "    -n       ... no sampling read (default=sampling)\n");
  fprintf(stderr, "    -d       ... dump the data (with -f, -3, -w or -p)\n");
  fprintf(stderr, "    -i       ... show input register data\n");
  fprintf(stderr, "    -f       ... show fera data\n");
  fprintf(stderr, "    -3       ... show 3377 data\n");
  fprintf(stderr, "    -w       ... show 3377 wire hit\n");
  fprintf(stderr, "    -p       ... show PCOS wire hit\n");
  fprintf(stderr, "    router   ... name of the router (usually BLD)\n");
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
	int    sflag, iflag, fflag, mflag, tflag, wflag, pflag;
	int    itvl;
	char   router[256];
	int    cnt=0;
  int    swap;

	sflag = 1;
  mflag = 1;
	iflag = 0;
	fflag = 0;
	tflag = 0;
	wflag = 0;
	pflag = 0;
	itvl  = 10;
	filename = argv[0];
  swap = (ntohl(0x01020304)!=0x01020304);

  argc--;
	argv++;

	while(argc>0){
		if(**argv!='-')break;
		if(strlen(*argv)==2){
			switch((*argv)[1]){
			case 'n':
				sflag = 0;
				break;
			case 'h':
			case 'H':
				usage();
				exit(0);
			case 'f':
				fflag = 1;
				mflag = 0;
				show_fera_itvl = 1000;
				break;
			case 'i':
				iflag = 1;
				mflag = 0;
				show_inpreg_itvl = 1000;
				break;
			case '3':
				tflag = 1;
				mflag = 0;
				show_3377_itvl = 1000;
				break;
			case 'w':
				wflag = 1;
				wire_flag = 1;
				mflag = 0;
				show_3377_itvl = 1000;
				break;
			case 'p':
				pflag = 1;
				pcos_flag = 1;
				mflag = 0;
				show_pcos_itvl = 1000;
				break;
			case 'd':
				dump_flag = 1;
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
		if(iflag)
			show_inpreg_itvl = itvl;
		if(fflag)
			show_fera_itvl = itvl;
		if(tflag || wflag)
			show_3377_itvl = itvl;
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
		fd = router_connect(router, getuid(), filename, 0);
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
		if(mflag){
			if((++cnt)>=itvl){
				show_counters();
				cnt = 0;
			}
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
