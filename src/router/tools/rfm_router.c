/*
	rfm_router.c ... Read buffer from RFM and write to router.
  Copyright (C) 1997  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  19-JAN-1997 by A. Tamii
  Version 2.1 24-OCT-1997 by A. Tamii (continue data transfer when buffer size = 0)
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
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "router.h"
#include "errlog.h"
#include "rfm_lib.h"

char title[] = "RFM Router Ver 2.1 24-OCT-1997";

#if 0
char title[] = "RFM Router Ver 2.0 19-JAN-1996";
#endif
char *filename;

#define PROCNAME "rfmrt%d"

#define BUFSIZE     0x4000
#define VARLENGTH   0         /* Variable Length (or Fixed Length) */

char buf_read[BUFSIZE];
char zero[BUFSIZE];

void usage(){
	fprintf(stderr, "%s\n", title);
	fprintf(stderr, "  ... Read buffers from RFM and write them to router.\n", filename);
	fprintf(stderr, "Usage: %s rfm# router \n", filename);
  fprintf(stderr, "       rfm#   ... reflective memory stream number\n");
  fprintf(stderr, "       router ... name of the router\n");
  fprintf(stderr, "       file_name ... input file name (default=stdout)\n");
}

#if 0
int copy_mem(src, dst, size)
		 char   *src, *dst;
		 long   size;
{
	long *s, *d;
	int  i;
	int  n;
	n = ((long)dst) & 3;
	for(i=0; i<n && size>0; i++, size--)
		*dst++ = *src++;
	s = (long*)src;
	d = (long*)dst;
	n = size>>2;
	for(i=0; i<n; i++)
		*d++ = *s++;
	n = size & 3;
	src = (char*)s;
	dst = (char*)d;
	for(i=0; i<n; i++)
		*dst++ = *src++;
}
int copy_mem(src, dst, size)
		 char   *src, *dst;
		 long   size;
{
	short *s, *d;
	int  i;
	int  n;
	s = (short*)src;
	d = (short*)dst;
	size = size>>1;
	for(; size>0; size--)
		*d++ = *s++;
}
#endif

main(argc, argv)
		 int    argc;
		 char   *argv[];
{
	int    res;
	char   *buf_write;
	char   pnam[20];
	long   size, count;
	int    i, n;
	int    nrfm;
	int    fd_read;
	int    fd_write;
	
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

	if(argc<2 || 2<argc){
		fprintf(stderr, "Too many or too few arguments\n", *argv);
		usage();
		exit(1);
	}

	nrfm = atoi(argv[0]);
	argv++;
	argc--;

  /* Get Empty Buffer */
	for(i=0; i<BUFSIZE; i++)
		zero[i] = 0;
		
	/* Open Error Log */
	sprintf(pnam, PROCNAME, nrfm);
	errlog_open(pnam);
	msgout(ErrMessage, "%s", title);

  /* open RFM */
	fd_read = rfm_open(nrfm, O_RDONLY|O_BLOCK);
	if(fd_read<0){
		msgout(ErrSever, "Could not open RFM #%d.", nrfm);
		exit(1);
	}
	msgout(ErrComment, "Read data from RFM #%d", nrfm);
	
	/* Make Connection to Router */
	if(strlen(argv[0])>4) argv[0][4] = 0x00;
	msgout(ErrComment, "Write data to router '%s'", argv[0]);
	fd_write = router_connect(argv[0], getuid(), filename, O_WRONLY);
  if(fd_write<0){
		msgout(ErrSever, "Could not connect to the router %s. %s", argv[0], strerror(errno));
		exit(1);
	}
	msgout(ErrComment, "Connected to router '%s'.", argv[0]);
	
	while(1){
		/* request buffer */
		res = router_request_buf(fd_write, BUFSIZE);
		if(res){
			msgout(ErrSever, "Error in router_request_buf. %s", strerror(errno));
			break;
		}

		/* get buffer */
		res = router_get_buf(fd_write, &buf_write, &size);
		if(res){
			msgout(ErrSever, "Error in router_get_buf. %s", strerror(errno));
			break;
		}

		/* read a block */
		size = read(fd_read, buf_read, BUFSIZE);
		if(size==-1){
			msgout(ErrSever, "Error in read. %s", strerror(errno));
			break;
		}

#if 0
		fprintf(stderr, "Size = %4x, Data = %4x %4x %4x %4x\n",
						size,
				((unsigned short*)(buf_read))[0],  ((unsigned short*)(buf_read))[1], 
				((unsigned short*)(buf_read))[2],  ((unsigned short*)(buf_read))[3]);
#endif
		
		/* copy to the buffer */
#if 0
		copy_mem(buf_read, buf_write, size);
#else
		memcpy(buf_write, buf_read, size);
#endif
		
		/* release the block */
		if(size==0){
#if VARLENGTH
#if 1  /* 24-OCT-1997 */
			msgout(ErrWarning, "Received buffer size=0. Change the size to 2.");
			size = 2;
			router_release_buf(fd_write, buf_write, size);
			continue;
#else
			router_release_buf(fd_write, buf_write, size);
			break;
#endif
#else
			msgout(ErrWarning, "Received buffer size=0.");
			memcpy(&buf_write[size], zero, BUFSIZE-size);
			router_release_buf(fd_write, buf_write, BUFSIZE);
#if 1  /* 24-OCT-1997 */
			continue;
#else
			break;
#endif
#endif
		}
		count += size;
#if VARLENGTH
		res = router_release_buf(fd_write, buf_write, size);
#else
		memcpy(&buf_write[size], zero, BUFSIZE-size);
		res =	router_release_buf(fd_write, buf_write, BUFSIZE);
#endif
		if(res){
			msgout(ErrSever, "Error in router_release_buf. %s", strerror(errno));
			break;
		}
	}
	msgout(ErrComment, "Write %d KBytes.", count>>10);

	router_disconnect(fd_write);
	rfm_close(fd_read);

	msgout(ErrMessage, "End of process %s.", filename);
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
