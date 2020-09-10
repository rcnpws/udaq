/*
	router_send.c ... Send Message to Router
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  24-JAN-1995 by A. Tamii (for Solaris 2.3)
	Ver 2.00  31-OCT-1995 by A. Tamii
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "router.h"

char title[] = "Router Send Ver 2.0d 31-OCT-1996";
char *filename;

key_t     msg_key;
int       msg_id = -1;
rtr_msg_t msg;

#define COM_NAME_LEN  20

struct{
	int     id;
	char    name[COM_NAME_LEN+1];
} cmd[] = {
	{ RTR_CONNECT,     "connect"    },
	{ RTR_DISCONNECT,  "disconnect" },
	{ RTR_STATUS,      "status"     },
	{ RTR_TERMINATE,   "terminate"  },
/* abbreviates */
	{ RTR_CONNECT,     "con"        },
	{ RTR_DISCONNECT,  "discon"     },
	{ RTR_STATUS,      "stat"       },
	{ RTR_TERMINATE,   "term"       },
	{ RTR_TERMINATE,   "stop"       },
	{ RTR_TERMINATE,   "end"        },
	{ RTR_TERMINATE,   "quit"       },
	{ RTR_TERMINATE,   "exit"       },
/* end of list */
	{ 0,   ""           }
};

void usage(){
	fprintf(stderr, "%s\n", title);
	fprintf(stderr, "%s ... send a command to a router\n", filename);
	fprintf(stderr, "Usage: %s router command\n", filename);
	fprintf(stderr, "  router     ... name of the target router\n");
	fprintf(stderr, "  command    ... terminate, status, etc.\n");
}

main(argc, argv)
		 int    argc;
		 char   *argv[];
{
	int    i, res;
	
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

	/* create key */
	msg_key = get_key(argv[0], getuid());
	if(msg_key==-1){
		fprintf(stderr, "main - ftok(): Cannot Ceate Key\n");
		exit(1);
	}
	
	msg_id = msgget(msg_key, 0666);
	if(msg_id==-1){
		if(errno==ENOENT)
			printf("Cannot link to router.\n");
		else
			perror("main - msgget()");
		exit(1);
	}
	
	msg.mtype = RTR_MTYPE_ROUTER;       /* Target is router */
	msg.pid = getpid();                 /* set my process id */
	strncpy(msg.name, filename, RTR_NAME_LEN);

	for(i=0;;i++){
		if(cmd[i].name[0]==0){
			fprintf(stderr, "Unknown Command '%s'\n", argv[2]);
			exit(1);
		}
		if(strcmp(cmd[i].name, argv[1]))
			continue;
		msg.cmd = cmd[i].id;
		break;
	}

	msgsnd(msg_id, &msg, sizeof(rtr_msg_t), 0);
}
