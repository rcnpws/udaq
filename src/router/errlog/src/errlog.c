/*
	errlog.c ... Error Log
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  28-JUL-1995 by A. Tamii (for Solaris 2.3)
  Version 2.0   28-FEB-1998 by A. Tamii
  Version 4.00 30-OCT-2000 by A. Tamii  # for AIX
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "errlog.h"

static int msg_id  = -1;
static key_t msg_key = -1;
static int end_flag = FALSE;
static int log_fd = -1;

#define ERRLOGNAM  "errorlog"

static void get_key(){
	msg_key = ftok(ERRLOGKEY, getuid()&0xFF);
	if(msg_key==-1){
		fprintf(stderr, "Couldn't create key\n");
		perror("ftok");
	}
}

static int create_msg(){
	int   res;
	msg_id = msgget(msg_key, 0666|IPC_CREAT|IPC_EXCL);
	if(msg_id==-1){
		if(errno==EEXIST){
			fprintf(stderr, "The message queue is already exist.\n");
		}else{
			fprintf(stderr, "Error in create_msg()");
			perror("msgget");
		}
		return(-1);
	}
	return(0);
}

static int remove_msg()
{
	int    res;
	if(msg_id==-1){
		msg_id = msgget(msg_key, 0666);
		if(msg_id==-1){
			fprintf(stderr, "Error in remove_msg()");
			perror("msgget");
			return(-1);
		}
	}

	/* remove message queue */
	res = msgctl(msg_id, IPC_RMID, 0);
	if(res==-1){
		fprintf(stderr, "Couldn't remove message queue.");
		perror("msgctl");
		return(0);
	}
	msg_id = -1;
}

void show_msg(msg)
		 err_msg_t    *msg;
{
	char    str[256];
	char    timestr[256];
	char    text[256];
	time_t  t;
	char    bell[2] = {7, 0};
	switch(msg->errtype){
	case ErrFatal:      /* Fatal Error */
		str[0] = 'F';
		printf("%s",bell);
		break;
	case ErrSever:
		str[0] = 'E';     /* Sever Error */
		printf("%s",bell);
		break;
	case ErrWarning:
		str[0] = 'W';     /* Warning */
		break;
	case ErrMessage:
		str[0] = 'M';     /* Message */
		printf("%s",bell);
		break;
	case ErrComment:
		str[0] = 'C';     /* Comment */
		break;
	case ErrLog:
		str[0] = 'L';     /* Log */
		return;
	case ErrDebug:
		str[0] = 'D';     /* Debug */
		break;
	case ErrVoid:
		str[0] = 'V';     /* Void */
		return;
	default:
		str[0] = 'U';     /* Unknown */
		printf("%s",bell);
		break;
	}
	time(&t);
	strcpy(timestr, ctime(&t));
	strcpy(&str[1], &timestr[10]);
	str[10] = 0x00;
	strcat(str, " ");
	strncat(str, msg->pnam, 8);
	strcat(str, "          ");
	str[19] = 0x00;
	strcat(str," ... ");
	strncat(str, msg->msg,55);
	printf("%s\n", str);
#if 0
	sprintf(text, "%s\n", &str[1]);
	write(log_fd, text, strlen(text));
#endif
}
void log_msg(msg)
		 err_msg_t    *msg;
{
	char    str[256];
	char    timestr[256];
	char    text[256];
	time_t  t;
	switch(msg->errtype){
	case ErrFatal:      /* Fatal Error */
		str[0] = 'F';
		break;
	case ErrSever:
		str[0] = 'E';     /* Sever Error */
		break;
	case ErrWarning:
		str[0] = 'W';     /* Warning */
		break;
	case ErrMessage:
		str[0] = 'M';     /* Message */
		break;
	case ErrComment:
		str[0] = 'C';     /* Comment */
		break;
	case ErrLog:
		str[0] = 'L';     /* Log */
		break;
	case ErrDebug:
		str[0] = 'D';     /* Debug */
		return;
	case ErrVoid:
		str[0] = 'V';     /* Void */
		return;
	default:
		str[0] = 'U';     /* Unknown */
		break;
	}
	time(&t);
	strcpy(timestr, ctime(&t));
	strcpy(&str[1], &timestr[3]);
	str[17] = 0x00;
	sprintf(text, " %5d ",msg->pid);
	strcat(str, text);
	strncat(str, msg->pnam, 8);
	strncat(str, "        ", 8 );
	str[33] = 0x00;
	strcat(str, " ... ");
	strncat(str, msg->msg,55);
	if(log_fd!=-1){
		sprintf(text, "%s\n", str);
		write(log_fd, text, strlen(text));
	}
}

static void message(msg)
		 err_msg_t  *msg;
{
	show_msg(msg);
	log_msg(msg);
}

void errlog(){
  err_msg_t  msg;
	int         res;
	/* wait for the message from consumer */
  end_flag = FALSE;
	while(!end_flag){
		res = msgrcv(msg_id, &msg, sizeof(err_msg_t), 0, MSG_NOERROR);
		if(res==-1){
			if(errno==EINTR)
				continue;
			if(errno==EIDRM||errno==EINVAL) /* message queue removed */
				break;
			err_msg(ErrSever, "Error in msgrcv(), id=%d.", errno);
			break;
		}
		message(&msg);
	}
}

void test(){
	err_msg_t   msg;
	msg.errtype = ErrFatal;
	strcpy(msg.pnam,"Test");
	strncpy(msg.msg,"This is a test message.",EMSGLEN);
	show_msg(msg);
	log_msg(msg);
}

void end_task(){
	end_flag = 1;
}

/* output error */
err_msg(type, str, data1,data2,data3,data4)
  int  type;
  char *str;
	long data1, data2, data3, data4;
{
	err_msg_t  msg;
	time_t     t;
	char text[256];
	sprintf(text, str, data1, data2, data3, data4);
	msg.errtype = type;
	msg.pid = getpid();
	msg.mtype = 1;
	strncpy(msg.pnam, ERRLOGNAM, PNAMLEN);
	msg.errid = 0;
	time(&t);
	msg.time = t;
	strncpy(msg.msg, text, EMSGLEN);
	message(&msg);
}

int main(argc, argv)
		 int     argc;
		 char    *argv[];
{
	int   i;
	
	if(argc>2){
		printf("%s --- Error Logging Program\n", argv[0]);
		printf("  Usage:  %s [file_name] \n", argv[0]);
    printf("  file_name ... error log file name\n");
		exit(1);
	}

	if(argc>1){
		log_fd  = open(argv[1], O_WRONLY|O_CREAT|O_APPEND, 0666);
		if(log_fd==-1){
			fprintf(stderr, "Error on opening file: %s\n", argv[1]);
			perror("open");
			exit(1);
		}
		fprintf(stderr, "Output file: %s\n", argv[1]);
	}else{
		fprintf(stderr, "Output file: None\n");
	}

#if 0
	for(i=1; i<32; i++)
		signal(i, end_task);
#else
	for(i=1; i<15; i++)
		signal(i, end_task);
#endif

	err_msg(ErrMessage, "Launch error log.");

	if(argc>1)
		err_msg(ErrComment, "Error log file is '%s'.",argv[1]);
	else
		err_msg(ErrWarning, "No error log file is selected.");
	
	get_key();
	create_msg();
	errlog();
	err_msg(ErrMessage, "Terminate error log.");
  sleep(1);
	remove_msg();
}
