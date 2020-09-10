/*
	daq_info.c ... get DAQ information program
  Copyright (C) 2000  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, University of Tokyo
	          & Research Center for Nuclear Physics
  Created:  20-MAY-2000 by A. Tamii
  Ver 1.00  20-MAY-2000 by A. Tamii
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

#include "errlog.h"
#include "daqinfo.h"

char title[] = "DAQ Info Ver 1.0 20-MAY-2000";
#if 0
#endif
char *filename;

int shm_id;
int shm_key;

daq_info_t  *info = (daq_info_t*)NULL;

void writestr(fd, str)
		 int  fd;
		 char *str;
{
	write(fd, str, strlen(str));
}

static int attach_shm(){
	shm_id= shmget(shm_key, 0, 0);
	if(shm_id==-1){
		errout(ErrSever, "Error in shmget(). errno=%d: %s", errno, strerror(errno));
		return(-1);
	}	
  info = (daq_info_t*)shmat(shm_id, 0, SHM_W|SHM_R);
	if(info==(daq_info_t*)-1){
		errout(ErrSever, "Could not attach to the shared memory (key=%x).",
					 shm_key);
#ifdef AIX
#else
		perror("Error");
#endif
    return(-1);
	}

	return(0);
}

static int detach_shm(){
  if(info){
    shmdt(info);
    info = (daq_info_t*)NULL;
	}
}

void usage(){
	fprintf(stderr, "%s\n", title);
	fprintf(stderr, "%s ... get DAQ information\n", filename);
	fprintf(stderr, "Usage: %s \n", filename);
}

static int show_info(fd)
  FILE *fd;
{
  int i;
  fprintf(fd, "version:  %7d.%.2d\n",
					info->vermajor, info->verminor);
  fprintf(fd, "exp:%s\n", info->namexp);
  fprintf(fd, "run:%d\n", info->run);
  fprintf(fd, "comment:%s\n", info->comment);
  fprintf(fd, "ndata:%d\n", info->ndata);
  fprintf(fd, "nblk:%d\n", info->nblk);
  fprintf(fd, "ndatarun:%d\n", info->ndatarun);
  fprintf(fd, "nblkrun:%d\n", info->nblkrun);
  fprintf(fd, "nmodes:%d\n", info->nmodes);
  fprintf(fd, "nchs:%d\n", info->nchs);
  fprintf(fd, "nscalers:%d\n", info->nsclrs);

	fprintf(fd, "scaler:");
	for(i=0; i<info->nsclrs; i++)
		fprintf(fd, "%d %d ", i, info->scaler[i]);
	fprintf(fd, " \n");

	fprintf(fd, "sums:");
	for(i=0; i<info->nchs; i++)
		fprintf(fd, "%d %d ", i, info->sum[i]);
	fprintf(fd, " \n");

	fprintf(fd, "rate:");
  for(i=0; i<info->nchs; i++)
		fprintf(fd, "%d %f ", i, info->rate[i]);
	fprintf(fd, " \n");
}



int main(argc, argv)
		int    argc;
		 char   *argv[];
{
	int    res;
	char   *buf, *data;
	long   size;
	int    i, n;
	char   *zero;
	int    fd_read;
  int    rflag = 0;
	char   *bufs;
	
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

	if(argc<0 || 0<argc){
		fprintf(stderr, "Too many or too few arguments\n", *argv);
		usage();
		exit(1);
	}

  /* show title */
	errout(ErrMessage, "%s\n", title);

  /* attach shared memory */
  shm_key = di_key();
  if(attach_shm()<0) exit(1);

	show_info(stdout);

  detach_shm();
  return(0);
}

/* output message */
errout(type,str,data1,data2,data3,data4)
		 int  type;
		 char *str;
		 long data1, data2, data3, data4;
{
	char fmt[256];
	char text[256];
	strcpy(fmt, str);
	strcat(fmt, "\n");
  sprintf(text, fmt, data1, data2, data3, data4);
	fprintf(stderr, text);
	writestr(stdout, text);
#if 0
  sprintf(text, str, data1, data2, data3, data4);
	errlog_msg(type, text);
#endif
}

/*
  Local Variables:
  tab-width: 2
  End:
*/
