/*
	router_message.c ... Dump message queue
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  24-JAN-1995 by A. Tamii (for Solaris 2.3)
  Created:  14-JUL-1997 by A. Tamii (for Solaris 2.3)
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdlib.h>
#ifdef OSF1
#include <strings.h>
#endif
#include <time.h>
#include <errno.h>
#include <stdio.h>

#include "router.h"

key_t     key;
int       msg_id = -1;
int       shm_id = -1;
char      *shm_buf;

rtr_msg_t snd_msg;

char *commands[] = {
	"   null", "connect", " discon", " refuse", " buffer",
	"release", " getbuf", "suspend", "       ", "       ",
	" status", "   term", "unknown"
};

main(argc, argv)
		 int    argc;
		 char   *argv[];
{
	int    i, res;
	int    *data;
	int    size;
	rtr_msg_t msg;
  int    stream;
	char   device[60];
	
	if(argc<2){
		fprintf(stderr,
						"%s ... Dump messages in the router message queue\n", argv[0]);
		fprintf(stderr, "  Usage: %s router\n", argv[0]);
    fprintf(stderr, "  router ... name of the router\n");
		exit(1);
	}

	fprintf(stderr, "Dump messages of the router '%s'-in\n", argv[1]);
	
	/* create key */
	key = get_key(argv[1], getuid());
	if(key==-1){
		fprintf(stderr, "Could not get key\n");
		exit(1);
	}
	
	msg_id = msgget(key, 0666);
	if(msg_id==-1){
		if(errno==ENOENT)
			fprintf(stderr, "Could not access to the message queue.\n");
		else
			perror("main - msgget()");
		exit(1);
	}
	
	fprintf(stderr,
					"      mtype     pid      cmd    arg1    arg2    arg3   name\n");
	for(i=1;;i++){
		res = msgrcv(msg_id, &msg, sizeof(rtr_msg_t), 0, IPC_NOWAIT);
		if(res==-1){
			if(errno==ENOMSG)
				break;
			if(errno==EINTR)
				continue;
			if(errno==EIDRM||errno==EINVAL){ /* message queue removed */
				fprintf(stderr, "Link to router was closed.\n");
				break;
			}
			fprintf(stderr, "Error in msgrcv(), id=%d.", errno);
			break;
		}
		if(msg.cmd>=10){
			if(msg.cmd==RTR_STATUS) msg.cmd = 10;
			else if(msg.cmd==RTR_TERMINATE) msg.cmd = 11;
			else msg.cmd = 12;
		}
		fprintf(stderr, "%3d)  %5d   %5d  %7s  %6d  %6d  %6d   %s\n",
					 i, msg.mtype, msg.pid, commands[msg.cmd],
						msg.arg[0], msg.arg[1], msg.arg[2], msg.name);
	}

	fprintf(stderr, "\nDump messages of the router '%s'-out\n", argv[1]);
	
	/* create key */
	key = get_key_out(argv[1], getuid());
	if(key==-1){
		fprintf(stderr, "Could not get key\n");
		exit(1);
	}
	
	msg_id = msgget(key, 0666);
	if(msg_id==-1){
		if(errno==ENOENT)
			fprintf(stderr, "Could not access to the message queue.\n");
		else
			perror("main - msgget()");
		exit(1);
	}
	
	fprintf(stderr,
					"      mtype     pid      cmd    arg1    arg2    arg3   name\n");
	for(i=1;;i++){
		res = msgrcv(msg_id, &msg, sizeof(rtr_msg_t), 0, IPC_NOWAIT);
		if(res==-1){
			if(errno==ENOMSG)
				break;
			if(errno==EINTR)
				continue;
			if(errno==EIDRM||errno==EINVAL){ /* message queue removed */
				fprintf(stderr, "Link to router was closed.\n");
				break;
			}
			fprintf(stderr, "Error in msgrcv(), id=%d.", errno);
			break;
		}
		if(msg.cmd>=10){
			if(msg.cmd==RTR_STATUS) msg.cmd = 10;
			else if(msg.cmd==RTR_TERMINATE) msg.cmd = 11;
			else msg.cmd = 12;
		}
		fprintf(stderr, "%3d)  %5d   %5d  %7s  %6d  %6d  %6d   %s\n",
					 i, msg.mtype, msg.pid, commands[msg.cmd],
						msg.arg[0], msg.arg[1], msg.arg[2], msg.name);
	}

}

