/*
	router_lib.c ... Library Routine for Consumers
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  18-FEB-1995 by A. Tamii (for Solaris 2.3)
  Version 2.00 28-FEB-1998 by A. Tamii (cleanup when catch a signal of 1-15)
  Version 4.00 30-OCT-2000 by A. Tamii  # for AIX
*/

#ifdef  SunOS
#define KERNEL
#endif
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "router.h"

#define MNODESC    8        /* max number of connections to routers */

typedef struct rtr_desc{
	int       open;
	int       connect;
	key_t     key;
	key_t     key_in;
	key_t     key_out;
	int       msg_id_in;
	int       msg_id_out;
	int       shm_id;
  char      *shm_buf;
	int       fmode;
	rtr_msg_t msg_snd;
	rtr_msg_t msg_rcv;
} rtr_desc_t;

static rtr_desc_t *desc = (rtr_desc_t*)NULL;

/*************************************************************************/
/* FUNC: get_key                                                         */
/* DESC: create the key of the share memory and the message queue        */
/* ARG:  char  name;                                                     */
/*         name of the router                                            */
/*       uid_t uid;                                                      */
/*         user ID (obtained by getuid());                               */
/* RESL: key_t key;                                                      */
/*         key of the shared memory and the message queue (identical)    */
/*************************************************************************/
key_t get_key(name, uid)
		 char   *name;
		 uid_t  uid;
{
	key_t   id;
	char  str[10];
	strncpy(str, name, 4);
	str[4] = 0x00;
	strcat(str, "    ");
	id = (key_t)(*(int*)str + *(int*)"rtr " + (int)uid);
	return(id);
}

/* message queue key for output */
key_t get_key_out(name, uid)
		 char   *name;
		 uid_t  uid;
{
	key_t   id;
	id = get_key(name, uid) + *(int*)"out ";
	return(id);
}

/*************************************************************************/
/* FUNC: open_fd                                                         */
/* DESC: get file descripter                                             */
/* ARG:  none                                                            */
/* RESL: int  fd;                                                        */
/*         file descripter number                                        */
/*         returns -1 if error occured                                   */
/*************************************************************************/
static int open_fd(){
	int   i;
	if(desc==(rtr_desc_t*)NULL){
		desc = (rtr_desc_t*)malloc(sizeof(rtr_desc_t)*MNODESC);
		if(desc==(rtr_desc_t*)NULL){
			fprintf(stderr, "open_fd: Could not allocate memory for desc.\n");
			return(-1);
		}
		for(i=0; i<MNODESC; i++)
			desc[i].open = 0;
	}
	for(i=0; i<MNODESC; i++){
		if(desc[i].open==0){
			desc[i].open    = 1;
			desc[i].connect = 1;
			desc[i].key     = 0;
			desc[i].key_in  = 0;
			desc[i].key_out = 0;
			desc[i].msg_id_in   = -1;
			desc[i].msg_id_out  = -1;
			desc[i].shm_id  = -1;
			desc[i].shm_buf = (char*)-1;
			desc[i].fmode   = 0;
			return(i);
		}
	}
	fprintf(stderr, "open_fd: File descripter is full.\n");
	return(-1);
}

/*************************************************************************/
/* FUNC: check_fd                                                        */
/* DESC: check the validity of fd                                        */
/* ARG:  int  fd                                                         */
/*         file descripter number                                        */
/* RESL: int  result;                                                    */
/*         returns -1 if error occured, otherwise returns 0.             */
/*************************************************************************/
static int check_fd(fd)
{
	int   i;
	if(fd<0 || MNODESC<=fd ){
		return(-1);
	}
	if(desc==(rtr_desc_t*)NULL || desc[fd].open==0){
		return(-1);
	}
	return(0);
}

/*************************************************************************/
/* FUNC: close_fd                                                        */
/* DESC: release file descripter                                         */
/* ARG:  int  fd                                                         */
/*         file descripter number                                        */
/* RESL: int  result;                                                    */
/*         returns -1 if error occured, otherwise returns 0.             */
/*************************************************************************/
static int close_fd(fd)
{
	int   i;
	if(check_fd(fd)){
		fprintf(stderr, "close_fd: Illegal fd. (fd=%d)\n", fd);
		return(-1);
	}
	desc[fd].open = 0;
	return(0);
}

/*************************************************************************/
/* FUNC: get_shmbuf                                                      */
/* DESC: get shared memory buffer pointer                                */
/* ARG:  int  fd                                                         */
/*         file descripter number                                        */
/* RESL: void *shmbuf;                                                   */
/*         Pionter to the shared memory buffer.                          */
/*         Null pointer will be returned when error                      */
/*************************************************************************/
void *get_shmbuf(fd)
		 int   fd;
{
	if(check_fd(fd))
		return (void*)NULL;
	return (void*)desc[fd].shm_buf;
}

/*************************************************************************/
/* FUNC: router_disconnect                                               */
/* DESC: send disconnection message to router and detach shared memory.  */
/* ARG:  int  fd                                                         */
/*         file descripter number                                        */
/* RESL: none                                                            */
/*************************************************************************/
void router_disconnect(fd){
	if(check_fd(fd)){
		fprintf(stderr, "router_disconnect: Illegal fd. (fd=%d)\n", fd);
		return;
	}
	if(desc[fd].connect){
		desc[fd].msg_snd.mtype = RTR_MTYPE_ROUTER;
		desc[fd].msg_snd.pid = getpid();
		desc[fd].msg_snd.cmd = RTR_DISCONNECT;
		msgsnd(desc[fd].msg_id_out, &desc[fd].msg_snd, sizeof(rtr_msg_t), 0);
	}
	if(desc[fd].shm_buf!=(char*)-1){
		shmdt(desc[fd].shm_buf);
		desc[fd].shm_buf = (char*)-1;
	}
	close_fd(fd);
}

/*************************************************************************/
/* FUNC: static cleanup                                                  */
/* DESC: Internal signal handling routine. Send disconnection message    */
/*       to the router and exit.                                         */
/*       Get shared memory.                                              */
/* ARG:  none                                                            */
/* RESL: none                                                            */
/*************************************************************************/
static void cleanup(sig)
		 int  sig;
{
	int  i;
#if 0
	fprintf(stderr, "cleanup sig=%d\n", sig);
#endif
	for(i=0; i<MNODESC; i++)
		if(check_fd(i)==0)
			router_disconnect(i);
	exit(0);
}

/*************************************************************************/
/* FUNC: router_connect                                                  */
/* DESC: Make a connection to the router. Initialize internal variables. */
/*       Get shared memory.                                              */
/*       This function set signal handling routines for the unexpected   */
/*       program end.                                                    */
/* ARG:  char  *router;                                                  */
/*         name of the router (4 characters)                             */
/*       uid_t uid;                                                      */
/*         user ID (obtained by getuid());                               */
/*       char  *name;                                                    */
/*         user program name                                             */
/*       int   mode;                                                     */
/*         open mode = NULL, IPC_NOWAIT, RTR_BLOCK, RTR_SAMPLE           */
/* RESL: >=0 ... file descripter                                         */
/*       -1 .... failure (errno = error code)                            */
/*************************************************************************/
int router_connect(router, uid, name, mode)
		 char   *router;
		 uid_t  uid;
		 char   *name;
		 int    mode;
{
	int       i, res, fd;
	
	fd = open_fd();
	if(fd<0)
		return(-1);
	
	switch(mode){
	case O_ALL:
	case O_SAMPLE:
	case O_WRONLY:
		break;
	default:
		fprintf(stderr, "router_connect: Illegal mode\n");
		router_disconnect(fd);
		errno=-10;
		return(-1);
	}

  desc[fd].fmode = mode;
  	
	/* create key */
	desc[fd].key      = get_key(router, uid);
	desc[fd].key_in   = get_key_out(router, uid);   /* client:in = router:out */
	desc[fd].key_out  = get_key(router, uid);

	desc[fd].msg_id_in = msgget(desc[fd].key_in, 0666);
	if(desc[fd].msg_id_in==-1){
		fprintf(stderr, "router_connect: Could not get msg_id\n");
		router_disconnect(fd);
		return(-1);
	}
	
	desc[fd].msg_id_out = msgget(desc[fd].key_out, 0666);
	if(desc[fd].msg_id_out==-1){
		fprintf(stderr, "router_connect: Could not get msg_id\n");
		router_disconnect(fd);
		return(-1);
	}
	
	desc[fd].msg_snd.mtype = RTR_MTYPE_ROUTER;    /* Target is router */
  desc[fd].msg_snd.pid  = getpid();             /* set my process id */
	desc[fd].msg_snd.cmd = RTR_CONNECT;           /* command */
	strncpy(desc[fd].msg_snd.name, name, RTR_NAME_LEN);
	desc[fd].msg_snd.name[RTR_NAME_LEN] = 0x00;
	desc[fd].msg_snd.arg[0] = desc[fd].fmode;
	desc[fd].msg_snd.arg[1] = ROUTER_VERSION;
	desc[fd].msg_snd.arg[2] = 0;
	
	/* send message */
	if(msgsnd(desc[fd].msg_id_out,
						&desc[fd].msg_snd, sizeof(rtr_msg_t), 0)<0){
		fprintf(stderr, "router_connect: Could not send messages.\n");
		router_disconnect(fd);
		return(-1);
	}
	
	/* wait for reply */
	while(1){
		res = msgrcv(desc[fd].msg_id_in, &desc[fd].msg_rcv,
								 sizeof(rtr_msg_t), getpid(), 0);
		if(res==-1){
			if(errno==EINTR)
				continue;
			else{
				fprintf(stderr, "router_connect: Could not get a reply.\n");
				router_disconnect(fd);
				return(-1);
			}
		}
		break;
	}

	if(desc[fd].msg_rcv.cmd!=RTR_CONNECT){
		fprintf(stderr, "router_connect: Could not connect to router.\n");
		router_disconnect(fd);
		errno = -11;
		return(-1);
	}
	
	desc[fd].connect = 1;   /* connected to the router */

	desc[fd].shm_id = shmget(desc[fd].key, 0, 0666);
	if(desc[fd].shm_id==-1){
		fprintf(stderr, "router_connect: Could not get shm_id.\n");
		router_disconnect(fd);
		errno = -12;
		return(-1);
	}

	if(mode & O_WRONLY)
		desc[fd].shm_buf = (char*)shmat(desc[fd].shm_id, 0, SHM_W|SHM_R);
	else
		desc[fd].shm_buf = (char*)shmat(desc[fd].shm_id, 0, SHM_RDONLY);
	if(desc[fd].shm_buf==(char*)-1){
		fprintf(stderr, "router_connect: Could not attach to the share memory.\n");
		router_disconnect(fd);
		errno = -13;
		return(-1);
	}

	for(i=1; i<16; i++)
		signal(i, cleanup);
	
	return(fd);
}

/*************************************************************************/
/* FUNC: router_request_buf                                              */
/* DESC: request buffer from router                                      */
/* ARG:  int    fd;                                                      */
/*          file descripter number                                       */
/*       long   size;                                                    */
/*          request size of the buffer                                   */
/* RESL:  0 .... success                                                 */
/*       -1 .... failure (errno = error code)                            */
/*************************************************************************/
int router_request_buf(fd, size)
		 int   fd;
		 long  size;
{
	int     res;
	if(check_fd(fd)){
		fprintf(stderr, "router_request_buf: Illegal fd. (fd=%d)\n", fd);
		errno = -10;
		return(-1);
	}
  while(1){
		desc[fd].msg_snd.mtype = RTR_MTYPE_ROUTER;
		desc[fd].msg_snd.pid = getpid();
		desc[fd].msg_snd.cmd = RTR_GETBUF;
		desc[fd].msg_snd.arg[0]  = size;
		desc[fd].msg_snd.arg[1]  = 0;
		desc[fd].msg_snd.arg[2]  = 0;
		if(msgsnd(desc[fd].msg_id_out,
							&desc[fd].msg_snd, sizeof(rtr_msg_t), 0)<0){
			fprintf(stderr, "router_request_buf: Could not send messages.\n");
			return(-1);
		}
		if(res==-1){
			if(errno==EINTR)
				continue;
			fprintf(stderr, "router_request_buf: Could not send messages.\n");
			return(-1);
		}
		break;
	}
	return(0);
}

/*************************************************************************/
/* FUNC: router_get_buf                                                  */
/* DESC: get buffer from router                                          */
/* ARG:  int    fd;                                                      */
/*          file descripter number                                       */
/*       void  *buf;                                                     */
/*          pointer to return the buffer pointer                         */
/*       int   *size;                                                    */
/*          pointer to return the buffer size                            */
/* RESL:  0 .... success                                                 */
/*       -1 .... failure (errno = error code)                            */
/*************************************************************************/
int router_get_buf(fd, buf, size)
		 int  fd;
		 void *buf;
		 long *size;
{
	int     res;
	if(check_fd(fd)){
		fprintf(stderr, "router_get_buf: Illegal fd. (fd=%d)\n", fd);
		errno = -10;
		return(-1);
	}
  while(1){
		res = msgrcv(desc[fd].msg_id_in, &desc[fd].msg_rcv, sizeof(rtr_msg_t),
								 getpid(), desc[fd].fmode & IPC_NOWAIT);
		if(res==-1){
			if(errno==EINTR)
				continue;
			fprintf(stderr, "router_get_buf: Could not get messages.\n");
			return(-1);
		}
		switch(desc[fd].msg_rcv.cmd){
		case RTR_BUFFER:
			*(char**)buf = (char*)&desc[fd].shm_buf[desc[fd].msg_rcv.arg[0]];
			*size = desc[fd].msg_rcv.arg[1];
			break;
		default: /* unknown message from router ... ignore */
			fprintf(stderr,
							"router_get_buf: Unknown message from router ... ignore.\n");
			continue;
			break;
		}	
		break;
	}
	return(0);
}

/*************************************************************************/
/* FUNC: router_release_buf                                              */
/* DESC: release buffer to router                                        */
/* ARG:  int    fd;                                                      */
/*          file descripter number                                       */
/*       void  *buf;                                                     */
/*          buffer pointer                                               */
/*       long   size;                                                    */
/*          size of the data (used only when fmode==O_WRONLY)            */
/* RESL:  0 .... success                                                 */
/*       -1 .... failure (errno = error code)                            */
/*************************************************************************/
int router_release_buf(fd, buf, size)
		 int  fd;
		 void *buf;
		 long size;
{
	int       res;
	if(check_fd(fd)){
		fprintf(stderr, "router_release_buf: Illegal fd. (fd=%d)\n", fd);
		errno = -10;
		return(-1);
	}
	desc[fd].msg_snd.mtype = RTR_MTYPE_ROUTER;
	desc[fd].msg_snd.pid = getpid();
	desc[fd].msg_snd.cmd = RTR_RELEASE;
	desc[fd].msg_snd.arg[0] = (long)buf-(long)desc[fd].shm_buf;
	desc[fd].msg_snd.arg[1]  = size;
	desc[fd].msg_snd.arg[2]  = 0;
  while(1){
		res = msgsnd(desc[fd].msg_id_out, &desc[fd].msg_snd,
								 sizeof(rtr_msg_t), desc[fd].fmode&IPC_NOWAIT);
		if(res==-1){
			if(errno==EINTR)
				continue;
			fprintf(stderr,	"router_release_buf: Could not send the message.\n");
			return(-1);
		}	
		break;
	}
	return(0);
}
	
