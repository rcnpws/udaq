/*
	router.c ... Buffer Routing Program
  Copyright (C) 1995, 1996  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  24-JAN-1995 by A. Tamii (for Solaris 2.3)
  Version 2.00 27-OCT-1996 by A. Tamii  # Separate Writer and support OSF1
  Version 2.01 18-JUN-1997 by A. Tamii 
  Version 3.00 14-JUL-1997 by A. Tamii  # Use two message queues for in/out
  Version 3.01 15-NOV-1997 by A. Tamii  # Show status when hang up
  Version 3.02 15-NOV-1997 by A. Tamii  # free msg (bug fix)
  Version 3.10 20-DEC-1999 by A. Tamii  # bug fix (router freezing)
  Version 4.00 30-OCT-2000 by A. Tamii  # for AIX (bug fix in clear_shmbuf);
  Version 4.10 08-NOV-2000 by A. Tamii  # bugfix for AIX (in listen.c)
  Version 4.20 28-DEC-2017 by A. Tamii  # enlared the default shared memory size
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <fcntl.h>
#if 0
#include <synch.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

#include "router.h"
#include "router_priv.h"
#include "errlog.h"

static char title[] = "Router Ver 4.20 28-DEC-2017";

#if 0
static char title[] = "Router Ver 4.10 08-NOV-2000";
static char title[] = "Router Ver 4.00 30-OCT-2000";
static char title[] = "Router Ver 3.10 20-DEC-1999";
static char title[] = "Router Ver 3.02 15-NOV-1997";
static char title[] = "Router Ver 3.01 15-NOV-1997";
static char title[] = "Router Ver 3.00 14-JUL-1997";
static char title[] = "Router Ver 2.01 18-JUN-1997";
static char title[] = "Router Ver 2.0d 27-OCT-1996";
#endif

char     proc_name[32];
char     *filename;

rtr_proc_t   pl_head_d;
rtr_proc_t   *pl_head = &pl_head_d;

rtr_shmbuf_t shm_used_head;
rtr_shmbuf_t shm_free_head;

rtr_shmbuf_t *shm_used = (rtr_shmbuf_t*)NULL;
rtr_shmbuf_t *shm_free = (rtr_shmbuf_t*)NULL;

static key_t msg_key_in;   /* key to the message queue */
static key_t msg_key_out;  /* key to the message queue */
static key_t shm_key;      /* key to the shared memory */

int   shmsize    = RTR_DEF_SHMSIZE;
int   msg_count = 0;

int   shm_id = -1;
int   msg_id_in = -1;
int   msg_id_out = -1;

int max_num_msg = 0;

void errout(type, msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8)
		 int     type;
		 char    *msg;
		 long    arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8;
{
	char text[256];
	sprintf(text, msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
#if 1
	errlog_msg(type, text);
#endif
	fprintf(stderr, "%s\n", text);
}

static int init_shm(){
	int      i;
	rtr_shmbuf_t  *shm;
  int      res;
	
	/* initialize shared memory */
	shm_used = &shm_used_head;
	shm_free = &shm_free_head;
	shm_used->next = shm_used;
	shm_used->prev = shm_used;
	shm_free->next = shm_free;
	shm_free->prev = shm_free;
	
	/* initialize free shared memory (all) */
	shm = malloc(sizeof(rtr_shmbuf_t));
	if(shm==(rtr_shmbuf_t*)NULL){
		errout(ErrFatal, "The shared memory is already exist.");
		return(-1);
	}
	shm->buf  = 0;
	shm->size = shmsize;
	shm->count=0;
	insert_item(shm_free, shm);

	return(0);
}

static int init_pl(){
	int      i;
	rtr_proc_t  *pl;
  int      res;
	
	/* initialize process list */
	pl_head = &pl_head_d;
	pl_head->next = pl_head;
	pl_head->prev = pl_head;
	
	return(0);
}

static int create_shm(){
	shm_id= shmget(shm_key, shmsize, 0666|IPC_CREAT|IPC_EXCL);
	if(shm_id==-1){
		if(errno==EEXIST){
			errout(ErrSever, "The shared memory is already exist.");
			errout(ErrSever,
							 "Please use -r option to remove the old shared memory.");
		} else
			errout(ErrSever, "Error in shmget(). errno=%d", errno);
		return(-1);
	}	
	errout(ErrComment, "Create shared memory (Key=%d).", (int)shm_key);
	return(0);
}

static int create_msg(){
	int   res;
	struct msqid_ds  msg_info;
	msg_id_in = msgget(msg_key_in, 0666|IPC_CREAT|IPC_EXCL);
	if(msg_id_in==-1){
		if(errno==EEXIST){
			errout(ErrSever, "The message queue is already exist.");
			errout(ErrSever,
							 "Please use -r option to remove the old message queue.");
		} else
			errout(ErrSever, "Error in msgget(). errno=%d", errno);
		return(-1);
	}
	errout(ErrComment, "Create message queue (Key=%d).", (int)msg_key_in);

	msg_id_out = msgget(msg_key_out, 0666|IPC_CREAT|IPC_EXCL);
	if(msg_id_in==-1){
		if(errno==EEXIST){
			errout(ErrSever, "The message queue is already exist.");
			errout(ErrSever,
							 "Please use -r option to remove the old message queue.");
		} else
			errout(ErrSever, "Error in msgget(). errno=%d", errno);
		return(-1);
	}
	errout(ErrComment, "Create message queue (Key=%d).", (int)msg_key_out);

#if FALSE  /* only the super user can change msg_qbytes */
	max_num_msg = RTR_MNOSENDMSG;
	msg_info.msg_qbytes = sizeof(rtr_msg_t)*max_num_msg;
	res = msgctl(msg_id, IPC_SET, &msg_info);
	if(res){
		errout(ErrSever, "Error in msgctl(IPC_SET) ... %d.", res);
		return(-1);
	}
#elif FALSE
	res = msgctl(msg_id, IPC_STAT, &msg_info);
	if(res){
		errout(ErrSever, "Error in msgctl(IPC_STAT) ... %d.", res);
		return(-1);
	}
	max_num_msg = msg_info.msg_qbytes/sizeof(rtr_msg_t)/2;
#else
	max_num_msg = RTR_MNOSENDMSG;
#endif
	msg_count = 0;
	errout(ErrComment, "Max number of messages = %d (Key=%d).",
				 max_num_msg);
	
	return(0);
}

static void remove_msg()
{
	int    res;
  /* ---- for msg_in ---- */
	if(msg_id_in==-1){
		msg_id_in = msgget(msg_key_in, 0666);
		if(msg_id_in==-1){
		  perror("remove_msg-shmget");
			return;
		}
	}
	/* remove message queue */
	res = msgctl(msg_id_in, IPC_RMID, 0);
	if(res==-1){
		errout(ErrWarning, "Cannot remove message queue, (err=%d).", errno );
		return;
	}
	else
		errout(ErrComment, "Remove message queue.");
	msg_id_in = -1;

  /* ---- for msg_out ---- */
	if(msg_id_out==-1){
		msg_id_out = msgget(msg_key_out, 0666);
		if(msg_id_out==-1){
		  perror("remove_msg-shmget");
			return;
		}
	}
	/* remove message queue */
	res = msgctl(msg_id_out, IPC_RMID, 0);
	if(res==-1){
		errout(ErrWarning, "Cannot remove message queue, (err=%d).", errno );
		return;
	}
	else
		errout(ErrComment, "Remove message queue.");
	msg_id_out = -1;
}

static void clear_shmbuf(){
	rtr_shmbuf_t  *shm, *tmp;
	shm = shm_used->next;
	while(shm!=shm_used){
		tmp = shm;
    shm = shm->next;
		remove_item(tmp);
		free(tmp);
	}
	shm = shm_free->next;
	while(shm!=shm_free){
		tmp = shm;
    shm = shm->next;
		remove_item(tmp);
		free(tmp);
	}
}

static void remove_shm()
{
	int           res;
	if(shm_id==-1){
		shm_id = shmget(shm_key, 0, 0666);
		if(shm_id==-1){
			perror("remove_shm-shmget");
			return;
		}
	}else{
		clear_shmbuf();
	}
	res = shmctl(shm_id, IPC_RMID, NULL);
	if(res==-1)
		errout(ErrWarning, "Cannot remove shared memory, (err=%d).", errno );
	else
		errout(ErrComment, "Remove shared memory.");
	shm_id = -1;
}

static void cleanup(){
	remove_msg();
	remove_shm();
	errlog_close();
}

static void quit(){
#if 0  /* 15-NOV-1997 */
	show_status();
#endif
	cleanup();
	errout(ErrMessage, "Terminate the process \"router\".");
	exit(0);
}

void usage(){
	fprintf(stderr, "%s\n", title);
	fprintf(stderr, "Buffer Routing Program\n");
	fprintf(stderr,
				"  Usage:  %s [-r] [-b shm_buf_size ] stream_num\n", filename);
  fprintf(stderr,
				"  Option: -r ... remove the old shared memory and message queue\n");
  fprintf(stderr,
				"          -b ... select shared memory buffer size in KB\n");
	fprintf(stderr,
				"                 Default = %d KB (%d MB)\n",
		RTR_DEF_SHMSIZE/1024, RTR_DEF_SHMSIZE/1024/1024);
}

int main(argc, argv)
		 int    argc;
		 char   *argv[];
{
	void router(char*);
	void listen(void);
	
	int      i, res;
	int      rflag, endFlag;
	char     num[20];
	char     *ptr;
	
	filename = *argv++;
	argc--;
	
	/* check the arguments */
	rflag = FALSE;
	endFlag = 0;
	while(argc>0 && argv[0][0]=='-'){
		if(strlen(argv[0])!=2){
			fprintf(stderr, "Unknown flag '%s'.\n",argv[0]);
			usage();
			exit(1);
		}
		switch(argv[0][1]){
		case 'b':
			if(argc<=1){
				fprintf(stderr, "Share memory size is not specified after '-b'.\n");
				usage();
				exit(1);
			}
			shmsize = atoi(argv[1])*0x400;
			if(shmsize<=0){
				fprintf(stderr, "Too small buffer size %d\n", shmsize/0x400);
				exit(1);
			}
			argv ++;
			argc --;
			break;
		case 'r':
			rflag = 1;
				break;
		default:
			fprintf(stderr, "Unknown flag '%s'.\n",argv[0]);
			usage();
			exit(1);
		}
		argc--;
		argv++;
	}
	
	if(argc<=0){
		usage();
		exit(1);
	}

	/* starting up message */
	if(strlen(*argv)>4) (*argv)[4] = 0x00;
  sprintf(proc_name,"%s-%s","rtr",*argv);

  /* add signal hooks */
	signal(SIGINT,  quit);
	signal(SIGQUIT, quit);
	signal(SIGHUP,  quit);

  /* open error log */
	if(errlog_open(proc_name)){
		/* avoid error */
		errout(ErrWarning, "Avoid the above error.");
	}

	errout(ErrMessage, "%s", title);
	errout(ErrMessage, "Launch Router %s", *argv);
	errout(ErrComment,
				 "Shared memory size = %dKB.", shmsize>>10);
	
	/* create key */
	shm_key = msg_key_in = get_key(*argv, getuid());
  msg_key_out = get_key_out(*argv, getuid());

	/* remove the old message queue or shared memory */
	if(rflag){
		remove_shm();
		remove_msg();
	}

	/* create shared memory */
	if(create_shm()){
		errout(ErrMessage, "End of process.");
		exit(1);
	}

	/* initialize statistics */
  init_stat();

	/* initialize shared memory */
  if(init_shm()){
		errout(ErrMessage, "End of process.");
		remove_shm();
		exit(1);
	}

	/* initialize process list */
  if(init_pl()){
		errout(ErrMessage, "End of process.");
		remove_shm();
		exit(1);
	}

	if(create_msg()){
		errout(ErrMessage, "End of process.");
		remove_shm();
		exit(1);
	}

	/* do main task*/
	listen();
	
	/* show status */
#if 0  /* 15-NOV-1997 */
	show_status();
#endif

  /* clean up */
	cleanup();

	errout(ErrComment, "End of the process \"router\".");
}

