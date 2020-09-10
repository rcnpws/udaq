/*
	router_tcp.c ... Send buffers from the router to the remove host by TCP/IP
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  18-FEB-1995 by A. Tamii (for Solaris 2.3)
  Version 1.0  18-FEB-1995 by A. Tamii
  Version 2.0  16-NOV-1996 by A. Tamii (for OSF1)
  Version 2.1  21-JAN-1997 by A. Tamii (Bug Fix)
  Version 2.3  06-JUL-1997 by A. Tamii (Catch SIGCLD for OSF1)
  Version 2.4  10-JUL-1997 by A. Tamii (use getuid for INET_VER1)
  Version 2.5  13-JUL-1997 by A. Tamii (change htons to htonl of info1.mode)
  Version 2.6  18-JUL-1997 by A. Tamii (accept signal SIGINT, SIGHUP, SIGQUIT)
  Version 2.7  18-MAY-2000 by A. Tamii
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <time.h>
#include <errno.h>

#include "router.h"
#include "errlog.h"

#define PNAMTCP "tcp_d"

char title[] = "Router TCP Ver 2.7 18-MAY-2000";

#if 0
char title[] = "Router TCP Ver 2.6 18-JUL-1997";
char title[] = "Router TCP Ver 2.5 13-JUL-1997";
char title[] = "Router TCP Ver 2.4 10-JUL-1997";
char title[] = "Router TCP Ver 2.3 06-JUL-1997";
char title[] = "Router TCP Ver 2.2 21-JAN-1997 (For e96)";
char title[] = "Router TCP Ver 2.6b - Big Buffer 23-APR-1999";
#endif
char *filename;
#if 1
/* Default */
int  def_blk_size[]={ 0x8000, 0x100000, 0x4000, 0x4000, 0x4000, 0x100000 };
#endif
#if 0
/* Big Buffer 23-APR-1999 */
int  def_blk_size[]={ 0x8000, 0x18000, 0x4000, 0x4000, 0x4000, 0x18000 };
#endif
#if 0
/* Old Settings */
int  def_blk_size[]={ 0x8000, 0x10000, 0x8000, 0x8000, 0x8000, 0x18000 };
int  def_blk_size[]={ 0x8000, 0x18000, 0x8000, 0x8000, 0x8000, 0x18000 };
#endif

long  blk_size;
char *router_names[]={ "", "BLDG", "GR  ", "LAS ", "BLP ", "BLDL" };
int num_router_names = 6;

struct sockaddr addr;
int addrlen;
int sd=-1;
int client_ver;    /* client version */
int p_pid=0;       /* parent process id */
int ld=-1;

#ifdef SunOS
static char errmsg[256];
char *strerror(n){
  perror(errmsg);
  return errmsg;
}
#endif


#ifdef OSF1
/* signal handler */
void sighdl_child(sig)
		 int  sig;
{
	int   status;
	wait(&status);
	signal(SIGCLD, sighdl_child);
}
#endif

void usage(){
	fprintf(stderr, "%s\n", title);
	fprintf(stderr, "  ... DAQ TCP/IP connection server\n", filename);
	fprintf(stderr, "Usage: %s \n", filename);
}

static void cleanup(){
  if(ld>=0) {
    shutdown(ld,2);
    close(ld);
	}
  if(sd>=0) {
    shutdown(sd,2);
    close(sd);
	}
	if(getpid()==p_pid)   /* only the parent process close the log */
		errlog_close();
}

static void quit(){
	cleanup();
	msgout(ErrMessage, "Terminate the process.");
	exit(0);
}

main(argc, argv)
		 int    argc;
		 char   *argv[];
{
  void send_data();
	struct sockaddr_in name;
  struct servent *serv_p;
	int i;
	
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

	/* Open Error Log */
	errlog_open(PNAMTCP);
	msgout(ErrMessage, "%s", title);
  p_pid = getpid();

	/* make a socket */
	if((ld=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0){
		msgout(ErrSever, "Error in socket(): %s", strerror(errno));
		msgout(ErrSever, "Terminate the process.");
		exit(1);
	}
	
  /* Get service port */
  if((serv_p=getservbyname(ROUTER_DAQDATA_SERVICE, "tcp"))==NULL){
    msgout(ErrWarning, 
					 "Could not find service '%s'. Use the default port.",
					 ROUTER_DAQDATA_SERVICE);
		name.sin_port = htons(ROUTER_DAQDATA_PORT);
	}else{
		name.sin_port = htons(serv_p->s_port);
	}
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = INADDR_ANY;
	msgout(ErrComment, "Waiting for connections at port %d.",
				 ntohs(name.sin_port));
	
	/* Bind */
  if(bind(ld, (struct sockaddr*)&name, sizeof(name))<0){
		close(ld); ld = -1;
		msgout(ErrSever, "Error in bind(): %s", strerror(errno));
		msgout(ErrSever, "Terminate the process.");
		exit(1);
	}
	
  /* Listen */
	if(listen(ld, 5)<0){
		close(ld); ld = -1;
		msgout(ErrSever, "Error in listen(): %s", strerror(errno));
		msgout(ErrSever, "Terminate the process.");
		exit(1);
	}
	

  /* Ignore SIGCLD */	
#ifdef OSF1
	signal(SIGCLD, sighdl_child);
#else
  signal(SIGCLD, SIG_IGN);
#endif
	
  /* add signal hooks */
	signal(SIGINT,  quit);
	signal(SIGQUIT, quit);
	signal(SIGHUP,  quit);

	/* Accept Loop */
	while(1){
		addrlen = sizeof(addr);
		if((sd=accept(ld, &addr, (socklen_t *)&addrlen))<0){
			if(errno==EINTR){
				/* tought to be a SIGCLD signal from a child process */
				continue;
			}
			close(ld); ld = -1;
			msgout(ErrSever, "Error in accept(): %s", strerror(errno));
			msgout(ErrSever, "Terminate the process.");
			exit(1);
		}
		if(fork()==0){
			/* Child Process */
			close(ld); ld = -1;
			send_data(sd);
			close(sd); sd = -1;
			exit(0);
		}
		/* Parent Process */
		close(sd); sd = -1;
	}
	
	msgout(ErrSever, "Terminate the process.");

  /* End */
  cleanup();
	exit(0);
}

void send_data()
{
  rtr_inet_info1_t  info1;
	rtr_inet_info2_t  info2;
	rtr_inet_info3_t  info3;
  struct hostent  *hp;
  struct sockaddr_in *in_p;
  int   count, n;
	long  size;
  char  *ptr, *buf;
	char  *zero=(char*)NULL;
  char  name[256];
  char  str[256];
  int   i, blksize;
  int   reply;
	int   fd_read;
	struct passwd *pwd;
	int   end_flag;
	
	/* get client host name */
  in_p = (struct sockaddr_in*)&addr;
	hp = gethostbyaddr((char*)&in_p->sin_addr,
			 sizeof(struct in_addr), AF_INET);
  if(hp!=NULL && *hp->h_aliases!=(char*)NULL)
		strcpy(name, *hp->h_aliases);
	else
		strcpy(name, (char*)inet_ntoa(in_p->sin_addr));

	msgout(ErrMessage, "Connection request from %s", name);


  /* read request from client */
  count = 0;
  ptr  = (char*)&info1;
	while(count<sizeof(info1)){
		if((n=read(sd, ptr, sizeof(info1)-count))<0){  /* wait for the reply */
			msgout(ErrSever, "Error in read(): %s", strerror(errno));
			return;
		}
	 if(n==0){
			msgout(ErrMessage, "Connection was closed by the client on %s", name);
			return;
		}
		count += n;
    ptr = &ptr[n];
	}

  /* check the version */
	client_ver = ntohl(info1.ver);
	if(client_ver > ROUTER_INET_VERSION_2){
		info2.result = htonl(E_VERSION);
    write(sd, &info2, sizeof(info2));
		msgout(ErrSever, "Client version is too high.");
		return;
	}
	client_ver = client_ver < ROUTER_INET_VERSION_2 ?
		ROUTER_INET_VERSION_1 : ROUTER_INET_VERSION_2;
  
	if('0' <= *name && *name <= '9'){
		/* IP Address */
	}else{
		/* Host Name */
		if((ptr=(char*)strchr(name, '.'))!=(char*)NULL)   /* get the host name */
			*ptr = 0x00;
	}
	strncpy(str, (char*)info1.user, sizeof(info1.user));
	strcat(str, "@");
  strcat(str, name);
	strcat(str, ":");
	strncat(str, (char*)info1.program, sizeof(info1.program));

	/* make connection to router */
	switch(client_ver){
	case ROUTER_INET_VERSION_1:
		if(ntohs(info1.unit)==0 &&
			 0<ntohs(info1.stream) || ntohs(info1.stream)<num_router_names){
			strncpy(info1.router, router_names[ntohs(info1.stream)], 
							sizeof(info1.router));
			blk_size = def_blk_size[ntohs(info1.stream)];
		}else{
		  info2.result = htonl(E_NODEV);
      write(sd, &info2, sizeof(info2));
		  msgout(ErrSever, 
						 "Could not connect to the router (unit=%d, stream=%d).",
						 ntohs(info1.unit), ntohs(info1.stream));
			return;
		}
		zero = (char*)malloc(blk_size);
		if(zero == (char*)NULL){
		  info2.result = htonl(E_NOMEM);
      write(sd, &info2, sizeof(info2));
		  msgout(ErrSever, "Could not allocate memory.");
			return;
		}else{
			for(i=0; i<blk_size; i++)
				zero[i]=0x00;
		}
		info1.router[sizeof(info1.router)-1] = 0x00;
		fd_read=router_connect(info1.router, getuid(), str, ntohl(info1.mode));
		if(fd_read<0){
		  info2.result = htonl(E_NODEV);
      write(sd, &info2, sizeof(info2));
		  msgout(ErrSever, "Could not connect to the router '%s'.",
					 info1.router);
			return;
		}
		break;
	case ROUTER_INET_VERSION_2:
		info1.uname[sizeof(info1.uname)-1] = 0x00;
#if 0
		info1.router[sizeof(info1.router)-1] = 0x00;
#endif
		pwd = getpwnam(info1.uname);
		if(pwd == (struct passwd*)NULL ||
		   (fd_read=router_connect(info1.router, pwd->pw_uid, 
															 str, ntohl(info1.mode)))<0){
		  info2.result = htonl(E_NODEV);
      write(sd, &info2, sizeof(info2));
		  msgout(ErrSever, "Could not connect to the router '%s'.",
					 info1.router);
			return;
		}
		break;
	default:
		msgout(ErrSever, "Never come here.");
		break;
	}
	
	info2.result = htonl(E_NOERR);
	info2.blk_size = client_ver == ROUTER_INET_VERSION_1 ? 
		htonl(blk_size) : 0;
	info2.res1 = info2.res2 = 0;

	/* send reply */
	if(write(sd, &info2, sizeof(info2))<0){
		msgout(ErrSever, "Error in write(): %s", strerror(errno));
		return;
	}

	end_flag = 0;
	for(i=0; !end_flag;i++){
		if(router_get_buf(fd_read, &buf, &size)<0){
			msgout(ErrSever, "Error in router_get_buf(). errno=%d", errno);
			break;
		}
		switch(client_ver){
		case ROUTER_INET_VERSION_1:
			/* write data via TCP/IP. Always send blk_size Bytes */
			if(size>(blk_size-0)){
				msgout(ErrWarning, "Data size is too large\n");
				size = blk_size;
			}
			if(write(sd, buf, size)<0)     
				msgout(ErrSever, "Error in write(): %s", strerror(errno));
			if(write(sd, zero, blk_size-size)<0)
				msgout(ErrSever, "Error in write(): %s", strerror(errno));
#if 1
			for(count=0; count<sizeof(reply); count+=n){
				n = read(sd, &((char*)&reply)[count], sizeof(reply)-count);
				if(n<0){  /* wait for the reply */
					msgout(ErrSever, "Error in read(): %s", strerror(errno));
					break;
				}
			}
#else
			if(read(sd, &reply, sizeof(reply))<0)  /* wait for the reply */
				msgout(ErrSever, "Error in read(): %s", strerror(errno));
#endif
			reply = ntohl(reply);
			if(reply!=0)
				msgout(ErrSever, "Error reply from the client(%d).", reply);
			break;
		case ROUTER_INET_VERSION_2:
			/* send data via TCP/IP (variable length) */
			info3.blk_count = htonl(i);
			info3.blk_size  = htonl(size);
#if 1
			if(size<=0 || size>0x4000){
				fprintf(stderr, "router_tcp: bufsize error: bufsize = %xH\n", size);
			}
#endif
			if(write(sd, &info3, sizeof(info3))<0)     
				msgout(ErrSever, "Error in write(): %s", strerror(errno));
			if(size>0){
				if(write(sd, buf, size)<0)
					msgout(ErrSever, "Error in write(): %s", strerror(errno));
			}
#if 1
			for(count=0; count<sizeof(reply); count+=n){
				n = read(sd, &((char*)&reply)[count], sizeof(reply)-count);
				if(n<0){  /* wait for the reply */
					msgout(ErrSever, "Error in read(): %s", strerror(errno));
					break;
				}
			}
#else
			if(read(sd, &reply, sizeof(reply))<0)  /* wait for acknowledge */
				msgout(ErrSever, "Error in read(): %s", strerror(errno));
#endif
			reply = ntohl(reply);
			if(reply!=0){
				msgout(ErrSever, "Error reply from the client(%d).", reply);
        router_disconnect(fd_read);
				return;
			}
			break;
		default:
			msgout(ErrSever, "Never come here.");
			end_flag = 1;
			break;
		}
		if(router_release_buf(fd_read, buf, size)<0){
			msgout(ErrSever, "Error in router_get_buf(). errno=%d", errno);
			break;
		}
	}

	router_disconnect(fd_read);
	if(zero!=(char*)NULL){
		free(zero);
		zero = (char*)NULL;
	}
  return;
}

msgout(type,str,data1,data2,data3,data4)
		 int  type;
		 char *str;
		 long data1, data2, data3, data4;
{
	char text[256];
  sprintf(text, str, data1, data2, data3, data4);
	fprintf(stderr, "%s\n",text);
	errlog_msg(type, text);
}

/*
  Local Variables:
  tab-width: 2
  End:
*/
