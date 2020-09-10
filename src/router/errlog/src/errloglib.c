/*
	errloglib.c ... Library of Error Log
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  28-JUL-1995 by A. Tamii (for Solaris 2.3)
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "errlog.h"

static int msg_id  = -1;
static int msg_key = -1;
static err_msg_t msg;

int errlog_open(name)
		 char  *name;
{
	msg_key = ftok(ERRLOGKEY, getuid()&0xFF);
	if(msg_key==-1){
		fprintf(stderr, "Couldn't create key\n");
		perror("ftok");
		return(-1);
	}

	msg_id = msgget(msg_key, 0666);
	if(msg_id==-1){
		fprintf(stderr, "Couldn't connect to error log.\n");
		perror("msgget");
		return(-1);
	}
	msg.pid = getpid();
	msg.mtype = 1;
	strncpy(msg.pnam, name, PNAMLEN);
	return(0);
}

int errlog_close(){
	return(0);
}

int errlog_msg(errtype, message)
		 int     errtype;
		 char    *message;
{
	time_t  t;
  int     res;
	msg.errtype = errtype;
	msg.errid = 0;
	time(&t);
	msg.time = t;
	strncpy(msg.msg, message, EMSGLEN);
  while(1){
		res = msgsnd(msg_id, &msg, sizeof(err_msg_t), O_RDWR&IPC_NOWAIT);
		if(res==-1){
			if(errno==EINTR)
				continue;
			return(-1);
		}	
		break;
	}
	return(0);
}
