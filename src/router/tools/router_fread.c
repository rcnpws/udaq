/*
	router_fread.c ... Output Buffers for fread
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  08-JUL-1997 by A. Tamii
	Ver 1.00  08-JUL-1997 by A. Tamii
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "router.h"
#include "assign_j11.h"
#include "errlog.h"

char title[] = "Router FRead Ver 1.0 08-JUL-1997";
char *filename;

#define FIXEDLENGTH 1

#if FIXEDLENGTH
#define BUFSIZE  65536
#endif

#define PNAMFREAD "fread"

errout(type,str,data1,data2,data3,data4)
		 int  type;
		 char *str;
		 long data1, data2, data3, data4;
{
	char text[256];
  sprintf(text, str, data1, data2, data3, data4);
	fprintf(stderr, "%s\n",text);
	errlog_msg(type, text);
}

void check_buf(buf, size)
		 unsigned char *buf;
		 int           size;
{
	static int blk_cnt=0;
	static int run_size=0;
	int   id, cnt;
	char  str[256];
	id = assign_j11(buf, size, &cnt, str);
	if(id&BLK_UNKNOWN){
	}
	if(id&BLK_START){
		errout(ErrComment, "Run Start Block ... Run #%d, %s", cnt, str);
		blk_cnt = 0;
		run_size = 0;
	}
	blk_cnt++;
	run_size += size;
	if(id&BLK_END){
		errout(ErrComment, "Run End Block ... Run #%d, %s", cnt, str);
		errout(ErrComment, "%d Bytes (%d Blocks) Recorded.", run_size, blk_cnt);
	}
	if(id&BLK_MIDDLE){
		errout(ErrComment, "Run Middle Block");
	}
}

void usage(){
	fprintf(stderr, "%s\n", title);
	fprintf(stderr, "  ... save buffers from the router\n", filename);
	fprintf(stderr, "Usage: %s router [file_name]\n", filename);
  fprintf(stderr, "  router     ... name of the target router\n");
	fprintf(stderr, "  file_name  ... file name to save buffers (default=stdout)\n");
}

main(argc, argv)
		 int    argc;
		 char   *argv[];
{
	int    res;
	char   *buf, *data;
	long   size;
	int    i, n;
	int    fd_write;
	int    fd_read;
#if FIXEDLENGTH
	char   zero[BUFSIZE];
#endif
	
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

  /* Open Error Log */
	errlog_open(PNAMFREAD);

  /* show title */
	errout(ErrMessage, "%s", title);
	if(strlen(argv[0])>4) argv[0][4] = 0x00;
	errout(ErrComment, "Connect to router '%s'.", argv[0]);

	/* Make Connection to Router */
	fd_read = router_connect(argv[0], getuid(), filename, O_SAMPLE);
  if(fd_read<0){
		perror("router_connect");
		errout(ErrSever, "Terminate Recorder Process.");
		exit(1);
	}
	
	/* Open File to Save */
	if(argc>=2){
		fd_write  = open(argv[1], O_WRONLY|O_CREAT|O_EXCL, 0666);
		if(fd_write==-1){
			fprintf(stderr, "Could not open the file: %s\n", argv[1]);
			perror("open");
			router_disconnect(fd_read);
			errout(ErrSever, "Terminate Recorder Process.");
			exit(1);
		}
		errout(ErrComment, "Output File=%s", argv[1]);
	}else{
		fd_write = 1;  /* stdout */
		errout(ErrComment, "Output File=%s", "stdout");
	}
#if FIXEDLENGTH
	for(i=0; i<BUFSIZE; i++)
		zero[i] = 0;
#endif

	while(TRUE){
		res = router_get_buf(fd_read, &buf, &size);
		if(res){
			perror("router_get_buf");
			break;
		}

#if FIXEDLENGTH
		if(size>BUFSIZE){
			errout(ErrWarning, "Block size is too big(%d). Reduced to %d.",
						 size, BUFSIZE);
			size = BUFSIZE;
		}
		if(write(fd_write, buf, size)<0){
			errout(ErrSever, "Error in write(). %s", strerror(errno));
			break;
		}
		if(size<BUFSIZE){
			if(write(fd_write, zero, BUFSIZE-size)<0){
				errout(ErrSever, "Error in write(). %s", strerror(errno));
				break;
			}
		}
#else
		if(write(fd_write, buf, size)<0){    /* write to the standard output */
			errout(ErrSever, "Error in write(). %s", strerror(errno));
			break;
		}
#endif
		check_buf(buf, size);

		res = router_release_buf(fd_read, buf, size);
		if(res){
			perror("router_release_buf");
			break;
		}
	}
	router_disconnect(fd_read);
	close(fd_write);
	errout(ErrMessage, "Terminate Recorder Process.");
	errlog_close();
}

