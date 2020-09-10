/*
	router_lr3377.c ... Dump LR3377 Data from Router
  Copyright (C) 1996  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  30-MAY-1996 by A. Tamii (for Solaris 2.3)
*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef OSF1
#include <strings.h>
#endif

#include "router.h"
#include "lr3377.h"

#define BlkSize  0x4000

unsigned char buf[BlkSize];

main(argc, argv)
		 int    argc;
		 char   *argv[];
{
	int    res;
	char   *buf, *data;
	int    size;
	int    i, n;
	int    fd;
	
	if(argc<2 || argc>1 && !strcmp(argv[1],"-h")){
		fprintf(stderr, "%s ... dump lr3377 data from a router\n", argv[0]);
		fprintf(stderr, "  Usage: %s stream_num\n", argv[0]);
    fprintf(stderr, "  stream   ... target stream number\n");
		exit(1);
	}
	
	n = atoi(argv[1]);
	
	/* Make connection to router or open file*/
	res = router_connect(0, n, argv[0], O_SAMPLE);
	if(res){
		perror("router_connect");
		exit(1);
		}
	
	while(TRUE){
		res = router_get_buf(&buf, &size);
		if(res){
			perror("router_get_buf");
			break;
		}
		
#if 0
		if((*(unsigned short*)buf & 0xc0ff)==(0x8000|MID_DELIMITER)){
			showLR3377(buf, size/2);
		}
#else
		if((*(unsigned short*)buf & 0xf000)==(0x8000)){
			showLR3377(buf, size/2);
		}
#endif
		
		res = router_release_buf(buf);
		if(res){
			perror("router_get_buf");
			break;
		}
	}

	router_disconnect();
}

