/*
	tcp_read.c ... DAQ TCP library test program
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  08-MAY-1995 by A. Tamii
*/

#include <stdio.h>
#include <stdlib.h>
#ifdef OSF1
#include <strings.h>
#endif
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>

#include "router.h"

main(argc, argv)
		 int  argc;
		 char *argv[];
{
	int   stream;
	int   mode;
	int   bufsize;
	int   i;
	int   result;
	char  *buf;
	char host[256];

	if(argc>1)
		strcpy(host, argv[1]);
	else
		strcpy(host, "miho");
	stream = 1;
	mode = 0;

	result = tcp_open(host, 0, stream, mode, "tcp_test", &bufsize);
	if(result<0){
		fprintf(stderr, "Error in tcp_open().\n");
		exit(1);
	}

	buf = (char*)malloc(bufsize);
	if(buf==(char*)NULL){
		perror("malloc");
		tcp_close();
		exit(1);
	}

	while(1){
		result = tcp_get_buf(buf);
		if(result<0){
			fprintf(stderr, "Error in tcp_get_buf().\n");
			break;
		}else if(result==1){
			fprintf(stderr, "End of data Transfer.\n");
			break;
		}
		result = write(1, buf, bufsize);
		if(result<0){
			fprintf(stderr, "Error in write().\n");
			break;
		}
	}

	free(buf);
	buf = (char*)NULL;

	result = tcp_close();
	if(result<0){
		fprintf(stderr, "Error in tcp_close().\n");
		exit(1);
	}
}
