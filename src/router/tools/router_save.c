/*
	router_save.c ... Save buffers from router
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  03-MAY-1995 by A. Tamii
	Ver 2.00  31-OCT-1995 by A. Tamii
	Ver 2.10  14-JUL-1997 by A. Tamii (add -v option)
	Ver 2.20  18-JUL-1997 by A. Tamii (Catch SIGINT, SIGHUP, SIGQUIT)
	Ver 2.40  13-JUL-2000 by A. Tamii
  Ver 3.00  10-NOV-2000 by A. Tamii (for AIX and -c option)
  Ver 3.10  14-JUL-2014 by A. Tamii (for GR-VDC VMEDAQ)
	Ver 3.20  10-NOV-2014 by A. Tamii (replaced write to fwrite for saving a file)
	Ver 3.30  04-OCT-2015 by A. Tamii (reduced buffer size from 1024 to 64 kB)
	Ver 3.40  17-OCT-2015 by A. Tamii (fflush after fwrite)
	Ver 3.50  27-DEC-2017 by A. Tamii (no fflush)
	Ver 3.60  27-DEC-2017 by A. Tamii (fflush with specified write-size or time))
	Ver 3.70  15-JUN-2018 by A. Tamii (fflush anytime)
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "router.h"
#include "assign_j11.h"
#include "errlog.h"

#define FFLASH_ANYTIME  1

char title[] = "Router Save Ver 3.7 15-JUN-2018 (fflush anytime)";
#if 0
char title[] = "Router Save Ver 3.6 27-DEC-2017 (fflush with write-size or time)";
char title[] = "Router Save Ver 3.5 27-DEC-2017 (no fflush)";
char title[] = "Router Save Ver 3.4 17-OCT-2015";
char title[] = "Router Save Ver 3.3 04-OCT-2015";
char title[] = "Router Save Ver 3.2 10-NOV-2014";
char title[] = "Router Save Ver 3.1 14-JUL-2014";
char title[] = "Router Save Ver 3.0 10-NOV-2000";
char title[] = "Router Save Ver 2.4 13-JUL-2000";
char title[] = "Router Save Ver 2.3 09-JUL-2000";
char title[] = "Router Save Ver 2.2 18-JUL-1997";
char title[] = "Router Save Ver 2.1 14-JUL-1997";
#endif
char *filename;

FILE   *fd_write;
int    fd_read;

#define BUFSIZE  16384
#define MAXSIZE  2013265920   /* 2^31 - 2^27 */
#if 0
#define MAXSIZE  134217728 /* for test */
#endif

#define FFLUSH_SIZE (1*1024*1024) /*  1 MB */
#define FFLUSH_TIME (1.0)         /*  1.0 sec */

#define PNAMSAVE "recorder"

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

static void cleanup(){
	router_disconnect(fd_read);
	fclose(fd_write);
	errlog_close();
}

static void quit(){
	errout(ErrMessage, "Terminate the process.");
	cleanup();
	exit(0);
}

void check_buf(buf, size, rsize)
		 unsigned char *buf;
		 int           size;
		 int           rsize;
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
	run_size += rsize;
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
	fprintf(stderr, "Usage: %s [-sv] router [file_name]\n", filename);
  fprintf(stderr, "  -s         ... sampling connection\n");
  fprintf(stderr, "  -c         ... change the output file when the file size becomes\n");
  fprintf(stderr, "                 larger than (or equal to) %d Bytes (default=no change).\n",MAXSIZE);
  fprintf(stderr, "  -b         ... set block size (by kB) (Default=%d)\n", BUFSIZE/1024);
  fprintf(stderr, "  -v         ... variable length\n");
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
	long   bsize;
	long   count;  /* number of bytes in a file */
	long   count_fflush;
	clock_t clock_fflush;
	int    i, n;
	int    vflag;
  int    sflag;
	int    cflag;
	int    nfile;
	char   *zero;
	char  str[256];
	char   *outfilename;
	
	filename = argv[0];

  argc--;
	argv++;

  vflag = 0; //variable length flag
  sflag = 0; //sampling flag
	cflag = 0; /* changed to 0, 14-JUL-2014 */
  nfile = 1;
	count = 0;
	count_fflush = 0;
	clock_fflush = clock();
	bsize = BUFSIZE;
	
	while(argc>0){
		if(**argv!='-')break;
		if(strlen(*argv)==2){
			switch((*argv)[1]){
			case 'h':
			case 'H':
				usage();
				exit(0);
			case 'v':  /* variable length */
				vflag = 1;
				break;
			case 'b':  /* set block size */
				argc--;
				argv++;
				bsize = atoi(*argv)*1024;
				break;
			case 's':  /* sampling */
				sflag = 1;
				break;
			case 'c':  /* change output filename */
				cflag = 1;
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

  /* Open Error Log */
	errlog_open(PNAMSAVE);

  /* get starge of zero */
	zero = (char*)malloc(bsize);

  /* show title */
	errout(ErrMessage, "%s", title);
	if(strlen(argv[0])>4) argv[0][4] = 0x00;

	/* Make Connection to Router */
  if(sflag){
  	errout(ErrComment, "Connect to router '%s' by sampling.", argv[0]);
  	fd_read = router_connect(argv[0], getuid(), filename, O_SAMPLE);
	}else{
  	errout(ErrComment, "Connect to router '%s' by all.", argv[0]);
  	fd_read = router_connect(argv[0], getuid(), filename, 0);
	}
  if(fd_read<0){
		errout(ErrSever, "router_connect: %s", strerror(errno));
		errout(ErrSever, "Terminate Recorder Process.");
		cleanup();
		exit(1);
	}
	
  /* add signal hooks */
	signal(SIGINT,  quit);
	signal(SIGQUIT, quit);
	signal(SIGHUP,  quit);

	/* Open File to Save */
	if(argc>=2){
		outfilename = argv[1];
		fd_write  = fopen(outfilename, "w");
		if(fd_write==(FILE*)NULL){
			errout(ErrSever, "Could not create file: %s", outfilename);
			errout(ErrSever, "open: %s", strerror(errno));
			errout(ErrSever, "Terminate Recorder Process.");
			cleanup();
			exit(1);
		}
		setvbuf(fd_write, NULL, _IOFBF, 1024*64);  // changed from 2014 kB to 64 kB on 04-OCT-2015
		errout(ErrComment, "Output File=%s", outfilename);
	}else{
		fd_write = stdout;  /* stdout */
		errout(ErrComment, "Output File=%s", "stdout");
	}
	for(i=0; i<bsize; i++)
		zero[i] = 0;

	while(TRUE){
		res = router_get_buf(fd_read, &buf, &size);
		if(res){
			errout(ErrSever, "router_get_buf: %s", strerror(errno));
			break;
		}

		if(vflag){
			/* variable length */
			if(fwrite(buf, sizeof(unsigned char), size, fd_write)<0){    /* write to the standard output */
				errout(ErrSever, "Error in write(). %s", strerror(errno));
				break;
			}
			count += size;
#if FFLASH_ANYTIME
			fflush(fd_write);
#else
			if(count > count_fflush + FFLUSH_SIZE || clock() > clock_fflush + FFLUSH_TIME*CLOCKS_PER_SEC){
			  fflush(fd_write);
				count_fflush = count;
				clock_fflush = clock();
			}
#endif
			check_buf(buf, size, size);
		}else{
			/* fixed length */
			if(size>bsize){
				errout(ErrWarning, "Block size is too big(%d). Reduced to %d.",
							 size, bsize);
				size = bsize;
			}
			if(fwrite(buf, sizeof(unsigned char), size, fd_write)<0){
				errout(ErrSever, "Error in write(). %s", strerror(errno));
				break;
			}
			if(size<bsize){
				if(fwrite(zero, sizeof(unsigned char), bsize-size, fd_write)<0){
					errout(ErrSever, "Error in write(). %s", strerror(errno));
					break;
				}
			}
#if FFLASH_ANYTIME
			fflush(fd_write);
#else
#endif
			count += bsize;
			check_buf(buf, size, bsize);
		}

		res = router_release_buf(fd_read, buf, size);
		if(res){
			errout(ErrSever, "router_release_buf: %s", strerror(errno));
			break;
		}

		if(cflag && fd_write!=stdout){
			if(count>=MAXSIZE){
				fclose(fd_write);
				nfile++;
				sprintf(str, "%s%.3d", outfilename, nfile);
				fd_write  = fopen(str, "w");
				if(fd_write==(FILE*)NULL){
					errout(ErrSever, "Could not create file: %s", str);
					errout(ErrSever, "open: %s", strerror(errno));
					errout(ErrSever, "Terminate Recorder Process.");
					cleanup();
					exit(1);
				}
				setvbuf(fd_write, NULL, _IOFBF, 1024*2014);
				count = 0;
				errout(ErrComment, "Output File = %s", str);
			}
		}
	}
	errout(ErrMessage, "Terminate Recorder Process.");
	cleanup();
}

/*
Local Variables:
mode: C
tab-width: 2
End:
*/

