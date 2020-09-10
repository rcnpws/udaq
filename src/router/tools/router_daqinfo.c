/*
	router_scaler_shm.c ... Write scaler data on shared memory
  Copyright (C) 2000  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  09-JUL-1995 by A. Tamii
  Ver 1.0   20-MAY-2000 by A. Tamii
  Ver 1.1   28-MAY-2000 by A. Tamii
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
#include "mtformat.h"
#include "assign_j11.h"
#include "errlog.h"
#include "daqinfo.h"

#define NSCLRS     6
#define NCHS      16

#define PNAMSCLRSHM "sclrshm"

#define BLKSIZE   0x4000

char title[] = "Router DAQ Info Ver 1.1 28-MAY-2000";
#if 0
char title[] = "Router DAQ Info Ver 1.0 20-MAY-2000";
#endif
char *filename;

int *scaler = (int*)NULL;
int *sum = (int*)NULL;
double *rate = (double*)NULL;
int num_scalers = 6;
int fd_out;
int data_flag=0;
int swap=0;

int shm_id;
int shm_key;

daq_info_t  *info = (daq_info_t*)NULL;

void writestr(fd, str)
		 int  fd;
		 char *str;
{
	write(fd, str, strlen(str));
}

void show_scalers(fd)
		 int  fd;
{
	int    i, j, k;
  char   str[256];
	if(fd<=0)
		return;
	writestr(fd, "-----------------------------------------------------------------------------\n");
  sprintf(str, "RUN(%5d)   BLK(%8d)\n", info->run, info->nblkrun);
	writestr(fd, str);
  sprintf(str, "'%s'\n", info->comment);
	writestr(fd, str);
	for(j=0; j<NCHS; j++){
		sprintf(str, "%2d   ", j);
		writestr(fd, str);
		for(i=0; i<NSCLRS; i++){
			sprintf(str, "%11d ", scaler[i*NCHS+j]);
			writestr(fd, str);
		}
		writestr(fd, "\n");
	}
	writestr("-----------------------------------------------------------------------------\n");
}


void do_clear_scalers(){
	int    i, j, k;
  k = 0;
	for(i=0; i<NSCLRS; i++){
		for(j=0; j<NCHS; j++){
			scaler[k] = 0;
			rate[k] = 0.0;
		}
		k++;
	}
  info->ndatarun = 0;
  info->nblkrun = 0;
}

void clear_scalers(){
  //	if(data_flag && !clear_scalers){
	if(data_flag){
		data_flag = 0;
		show_scalers(1);
		show_scalers(fd_out);
	}
	do_clear_scalers();
	errout(ErrComment,"Scalers are cleared.");
}

void get_scalers(buf, size)
		 unsigned short *buf;
		 int            size;
{
	unsigned short *ptr;
	unsigned short *p2;
	unsigned short *end;
  BlkHeaderPtr  blk;
	EvtHeaderPtr  evt;
	FldHeaderPtr  fld;
  int           scl;
  unsigned short data;
	int           i, k, rgn, len;
	int           value;
	
	blk = (BlkHeaderPtr)buf;
	end = &buf[size/2];
	ptr = &buf[blk->headerSize];
  p2 = &ptr[blk->blockSize];
	if(p2<end) end=p2;
	
	scl = 0;
	for(;ptr<end;ptr++){
		/* search for an event header */
		if(*ptr!=EvtHeaderID)
			continue;
		if(&ptr[S_EH_MIN]>end)
			break;
		evt = (EvtHeaderPtr)ptr;
		if(evt->headerSize<S_EH_MIN || S_EH_MAX<evt->headerSize)
			continue;
		if(evt->eventID!=BlockEndEvent)
			continue;
		fld = (FldHeaderPtr)&ptr[evt->headerSize];
		ptr = (unsigned short*)fld;
		if(&ptr[S_FH_MIN]>end)
			break;
		if(fld->headerID!=FldHeaderID)
			continue;
		if(fld->headerSize<S_FH_MIN || S_FH_MAX<fld->headerSize)
			continue;
		ptr = &ptr[fld->headerSize];
    p2 = &ptr[fld->fieldSize];
		if(p2>end) p2=end;
		while(ptr<p2){
			data = *ptr++;
			rgn = data&ModuleIDMask;
			len = data&DataLengthMask;
			if(rgn!=ID_Scaler){
				ptr += len;
				continue;
			}
			len >>= 1;
			if(len>NCHS) len = NCHS;
      k = scl*NCHS;
			for(i=0; i<len; i++){
				if(ptr>=p2)
					break;
				value = *ptr++;
				value += *ptr++ <<16;
				scaler[k++] += value;
			}
		scl++;
		}
	}
}

void get_sum(){
	int i, j, k;
	int s[NCHS];
  k = 0;
	for(i=0; i<NCHS; i++)
		s[i] = 0;
	for(i=0; i<NSCLRS; i++)
		for(j=0; j<NCHS; j++)
			s[j] += scaler[k++];
	for(i=0; i<NCHS; i++)
		sum[i] = s[i];
}

void read_buf(buf, size)
		 unsigned short *buf;
		 int            size;
{
	static int out_count=0;
	int    id;
	int    cnt;
	char   str[256];
	id = assign_j11(buf, size, &cnt, str);
	if(id&BLK_UNKNOWN){
#if 0
		errout(ErrVoid,"Unknown Block Format");
#endif
		return;
	}
	if(id&BLK_START){
    if(swap)
      swab(str, info->comment, DI_MAXCOMLEN);
    else
  		strncpy((char*)info->comment, str, DI_MAXCOMLEN);
    info->comment[MaxComLen+1] = 0X00;
		info->comlen = strlen(info->comment);
    info->run = cnt;
		info->nblkrun = 0;
		errout(ErrComment, "Run Start Block ... Run #%d, %s",
					 info->run, info->comment);

		clear_scalers();
	}
	if(id&BLK_END){
		errout(ErrComment, "Run End Block ... Run #%d, %s",
					 cnt, str);
    info->run = cnt;
    if(swap)
      swab(str, info->comment, DI_MAXCOMLEN);
    else
  		strncpy((char*)info->comment, str, DI_MAXCOMLEN);
    info->comment[DI_MAXCOMLEN+1] = 0X00;
		info->comlen = strlen(info->comment);

		show_scalers(1);
		show_scalers(fd_out);
		data_flag = 0;
#if 0
		errout(ErrComment, "Total %d Data Blocks", info->nblkrun);
#endif
	}
	if(id&BLK_MIDDLE){
		errout(ErrComment, "Run Middle Block");
	}
	if(id&(BLK_DATA|BLK_SCALER)){
#if 0
		if(id&BLK_DATA)
#endif
		info->nblk++;
		info->nblkrun++;
		get_scalers(buf, size);
		get_sum();
		show_scalers(1);
		data_flag = 1;
	}
}

static int clear_shm(){
  int i;
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
  info->vermajor = DI_VER_MAJOR;
  info->verminor = DI_VER_MINOR;
  strcpy(info->namexp,"");
  info->run = 0;
  strcpy(info->comment,"");
  info->comlen = 0;
  info->nmodes = NSCLRS;
  info->nchs   = NCHS;
  info->nsclrs = NSCLRS*NCHS;
  if(info->nsclrs > DI_MAXNSCLRS){
    info->nmodes = 0;
    info->nchs = 0;
    info->nsclrs = 0;
    errout(ErrSever, "Not enough shm space for scalers.");
    return(-1);
	}
  info->ndata = 0;
  info->ndatarun = 0;
  info->nblk = 0;
  info->nblkrun = 0;
  scaler = &info->scaler[0];
  sum    = &info->sum[0];
  rate   = &info->rate[0];
  for(i=0; i<info->nsclrs; i++){
    scaler[i] = 0;
	  rate[i] = 0.0;
	}
	return(0);
}

static int create_shm(){
  int fd;
  fd = open(DI_FNAM,O_CREAT);
  close(fd);
  if(DI_SHMSIZE<sizeof(daq_info_t)){
		errout(ErrSever, "The shared memory size must larger than (%d).",
					 sizeof(daq_info_t));
		return(-1);
	}
	shm_id= shmget(shm_key, DI_SHMSIZE, 0666|IPC_CREAT|IPC_EXCL);
	if(shm_id==-1){
		if(errno==EEXIST){
			errout(ErrSever, "The shared memory (Ke=%x) is already exist.", shm_key);
			errout(ErrSever,
							 "Please use -r option to remove the old shared memory.");
		} else
			errout(ErrSever, "Error in shmget(). errno=%d", errno);
		return(-1);
	}	
	errout(ErrComment, "Create shared memory (Key=%x).", (int)shm_key);
  if(clear_shm()<0) return(-1);
	return(0);
}

static int remove_shm()
{
	int res;
  shm_id = shmget(shm_key, 0, 0666);
  if(shm_id>=0){
  	res = shmctl(shm_id, IPC_RMID, NULL);
  	if(res==-1){
  		errout(ErrWarning, "Cannot remove shared memory (Key=%x), (Error=%d).",
						 shm_key, errno );
			return(-1);
		}
		else
			errout(ErrComment, "Remove shared memory (Key=%x).", shm_key);
	}
  return(0);
}

void usage(){
	fprintf(stderr, "%s\n", title);
	fprintf(stderr, "%s ... show scalers in buffers from a router\n", filename);
	fprintf(stderr, "Usage: %s router [file_name]\n",
					filename);
	fprintf(stderr, "  router     ... name of the target router\n");
	fprintf(stderr, "  file_name  ... file name to save scaler data\n");
}

main(argc, argv)
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

  swap = (ntohl(0x01020304)!=0x01020304);

	while(argc>0){
		if(**argv!='-')break;
		if(strlen(*argv)==2){
			switch((*argv)[1]){
			case 'h':
			case 'H':
				usage();
				exit(0);
			case 'r':
        rflag = 1;
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
	errlog_open(PNAMSCLRSHM);

  /* show title */
	errout(ErrMessage, "%s\n", title);
	if(strlen(argv[0])>4) argv[0][4] = 0x00;
	errout(ErrComment, "Connect to router '%s'.\n", argv[0]);

	/* Make Connection to Router */
	fd_read = router_connect(argv[0], getuid(), filename, 0);
  if(fd_read<0){
		perror("router_connect");
		errout(ErrSever, "Terminate Recorder Process.");
		exit(1);
	}
	
	/* Open File to Save */
	fd_out = 0;
	if(argc>=2){
		fd_out  = open(argv[1], O_WRONLY|O_CREAT|O_APPEND, 0666);
		if(fd_out==-1){
			errout(ErrSever, "Error opening file: %s\n", argv[1]);
			perror("open");
			router_disconnect(fd_read);
			exit(1);
		}
		errout(ErrComment, "Output file: %s\n", argv[1]);
	}

  errout(ErrMessage, "Output file=%s", fd_out==0 ? "NONE" : argv[1]);
	
  /* create shared memory */
  shm_key = di_key();
  if(rflag){
		if(remove_shm()<0) exit(1);
	}
  if(create_shm()<0) exit(1);

	clear_scalers();
	while(TRUE){
		res = router_get_buf(fd_read, &buf, &size);
		if(res){
			perror("router_get_buf");
			break;
		}
    if(swap){
      bufs = (char*)malloc((size+0x4000)&0xFFFFFFF0);
      if(bufs==(char*)NULL){
				errout(ErrSever, "Could not allocate memory.");
				break;
			}
      swab(buf, bufs, size);
  		read_buf(bufs, size); 
			free(bufs);
		}else{
  		read_buf(buf, size);
		}
		res = router_release_buf(fd_read, buf, size);
		if(res){
			perror("router_release_buf");
			break;
		}
#if 0
    info->ndata+=size;
    info->ndatarun+=size;
#else
    info->ndata+=BLKSIZE;
    info->ndatarun+=BLKSIZE;
#endif
	}
	router_disconnect(fd_read);
	errlog_close();
	close(fd_out);
  remove_shm();
	errout(ErrMessage, "Terminate scaler program.");
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
	if(fd_out>0)
		writestr(fd_out, text);
  sprintf(text, str, data1, data2, data3, data4);
	errlog_msg(type, text);
}
