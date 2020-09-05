/*
 *
 *  grcollect.c 
 *     GR collector program 
 *   Ver0.1        2003/08/16    M.Uchida   
 *   Ver0.2        2003/08/26    M.Uchida   API for Tamidaq ver2.0
 *   Ver1.0        2003/08/31    M.Uchida   variable q-size for ring buf 
 *
 */


#include <pthread.h>
#include "tcplib.h"
#include "ipclib.h"
#include "grlib.h"
#include "errlog.h"
#include "blpctrl.h"
#include "ring_buf.h"
#include "run_state.h"

#ifndef BLK_BUF_SIZE
#define BLK_BUF_SIZE 0x2000
#endif

#define SPARC_64BIT                /* SPARC 64bit mode                */
#define SCL_BUF_SIZE  132          /* Buffer size for VME scaler      */


const char version[30]="GR COLLECTOR Ver 1.0";
struct timespec t1,t2;             /* For nanosleep                   */
cmd_msgbuf_t mb;                   /* Command message que buffer      */
ring_buf_t *rb;                    /* ring buffer pointer             */
int max_que_num=64;                /* max que number for ring buffer  */

void *collect_data(void *arg);     /* Main collector routine         */
void *collect_cmd(void *arg);      /* Command handling routine       */
void *collect_test(void *arg);     /* Main collector routine(test)   */
void *collect_tcp(void *arg);      /* Data transfer routine(TCP/IP)  */
void *collect_tcp_serv(void *arg); /* Data transfer routine server   */

#if 0                              /* For remote logger (not used)   */
int lsockfd;
struct log_prop *lp;

int connfd;                        /* socket descripter for data tr  */
struct sockaddr_in  cliaddr;       /* Cliend address                 */
#endif


void usage(char *arg)
{
  fprintf(stderr,"Usage: %s [-h] [-d] [-t] [-s [time]sec] [-m [time]msec] [-u [time]usec] [-e loghost] \n",arg);
  fprintf(stderr,"-h   : help\n");
  fprintf(stderr,"-t   : test mode\n");
  fprintf(stderr,"-d   : debug mode\n");
  fprintf(stderr,"-s   : interval[sec]\n");
  fprintf(stderr,"-m   : interval[msec]\n");
  fprintf(stderr,"-u   : interval[usec]\n");
  fprintf(stderr,"-e   : error log host\n");
  exit(1);
}


void sighdl( arg )               /* Signal handler                 */
{
  printf("Interrupted. Signal ignored.\n");

#if 0
  rpv_dis(rpv);                  /* VETO ON                        */
  exit(1);
#endif

}


int main(int argc, char **argv)
{
  int i,j;
  long interval;                   /* Interval ( dummy data only)    */
  int  test_flag=0;                /* Option switch for test mode    */
  char host[256];                  /* server host name               */
  char pname[256];                 /* process name                   */
  char ename[256]=LOG_SERVER;      /* error log host name (not used) */

  int listenfd,cconnfd;            /* Socket descripter              */
  socklen_t cclilen, caddrlen;     /* Command client/addr length     */
  struct sockaddr_in  ccliaddr;    /* Command client address         */


#ifdef SPARC_64BIT
  int dum;
#else
  long dum;
#endif

  struct daq_prop *dp;

  pthread_t *thp,*thpp;
  pthread_attr_t detached_attr;

  int sockfd;      /* Socket file desc               */

  show_version(version);

  /*
   *
   * Command argument
   *
   */

  strcpy(pname,argv[0]);

  argc--;
  argv++;

  while( argc > 0){
    if(**argv != '-') break;
    if(strlen(*argv) == 2){
      switch((*argv)[1]){
      case 't':  /* debug mode on */

	test_flag = 1;

	t1.tv_sec  = 1;
	t1.tv_nsec = 0;

	break;
      case 'e':  /* Error logger host            */
	argc--; argv++;
	strcpy(ename,*argv);
	break;
      case 's':  /* interval in sec (debug mode) */
	argc--; argv++;
	interval = atoi(*argv);
	t1.tv_sec  = interval;
	t1.tv_nsec = 0;
	break;
      case 'm':  /* interval in msec (debug mode) */
	argc--;	argv++;
	interval = atoi(*argv);
	t1.tv_sec  = interval/1000;
	t1.tv_nsec = (interval%1000)*1000000;
	break;
      case 'u': /* interval in usec (debug mode) */
	argc--; argv++;
	interval = atoi(*argv);
	t1.tv_sec  = interval/1000000;
	t1.tv_nsec = (interval%1000000)*1000;
	break;
      case 'q': /* max que number for ring buffer */
	argc--; argv++;
	max_que_num = atoi(*argv);
	printf("max que number:%d\n",max_que_num);
	break;
      case 'h': /* help */
	usage(pname);
	break;
      default:
	printf("default\n");
	usage(pname);
	break;
      } 
    } else {
      usage(pname);
    }
    argc--; argv++;
  }

#if 0
  if(argc < 1){
    printf("no host\n");
    usage(pname);
  } else {
    strcpy(host,*argv);
  }
#endif

  /*
   *
   * Initialize thread instance
   *
   */

  run_state = (run_state_t *)malloc(sizeof(run_state_t));
  pthread_mutex_init(&run_state->lock,NULL);
  pthread_mutex_lock(&run_state->lock);
  run_state->status = RUN_OFF;
  pthread_mutex_unlock(&run_state->lock);


  /* Create ring buffer */

  rb = new_rb();

  /* Set process property */
#if 1
  if((dp = (struct daq_prop *)malloc(sizeof(struct daq_prop)))==NULL){
    fprintf(stderr,"Map error\n");
    exit(1);
  }
#else
  dp = (struct daq_prop *)Malloc(sizeof(struct daq_prop));
#endif

  strcpy(dp->name,pname);
  strcpy(dp->user,getenv("USER"));
  gethostname(dp->host,30);
  dp->type=DAQ_COLLECTOR;

  /* Set signal            */

  sigset(SIGINT, sighdl);

#if 0  /* For remote logger */
  if((lp = create_log_prop(pname,getenv("USER")))==NULL){
    	fprintf(stderr,"blpsave : Can't allocate memory\n");
	exit(1);
  }
#endif

#if 0   /* For remote logger */
  lsockfd = err_open(ename,lp);

  Send_err(lsockfd,"launched");
#endif

#if 0
  /* Create network interface  */

  sockfd = Tcp_connect(host,SERV_PORT_STR);


  Writen(sockfd, dp, sizeof(struct daq_prop));
  if(Readn(sockfd,&dum, sizeof(int))==0){
    err_quit("send_daq_prop: Waiting ack!\n");
  }
#endif


  /* Initialize thread attribution (detatched attr) */
  pthread_attr_init(&detached_attr);
  pthread_attr_setdetachstate(&detached_attr,PTHREAD_CREATE_DETACHED);

  thp = (pthread_t *)malloc(sizeof(pthread_t));

  if(test_flag == 1){
    fprintf(stdout,"Collector running in debug mode \n");
    if(pthread_create(thp,&detached_attr,collect_test,(void *)sockfd) !=0) {
      fprintf(stderr,"Can't exec daq collect\n");
      exit(1);
    }
    free(thp);
  } else {


    /* Start data readout from Memory modules */

    if(pthread_create(thp,&detached_attr,collect_data,(void *)sockfd) !=0) {
      fprintf(stderr,"Can't exec daq collect\n");
      exit(1);
    }
    free(thp);

  }

#if 1
  /* Start TCP data transfer server */

  if(pthread_create(thp,&detached_attr,collect_tcp_serv,(void *)sockfd) !=0) {
    fprintf(stderr,"Can't exec daq collect\n");
    exit(1);
  }
  free(thp);
#endif

  /* Create TCP/IP interface to command control client */
  
  listenfd = Tcp_listen(NULL,SERV_PORT_CTRL,&caddrlen);


  /* Waiting  for control command from client */  
  for(;;){
    printf("****** Waiting for the control command ******\n");
  retry:
    cclilen = sizeof(ccliaddr);
    if((cconnfd = accept(listenfd,(SA *)&ccliaddr,&cclilen))<0){
      fprintf(stderr,"blpcollect : can't connect cmd client\n");
      goto retry;
      
    }
#if 0
    printf("connection successful \n");
#endif
    
    if((thpp = (pthread_t *)malloc(sizeof(pthread_t))) == NULL){
      fprintf(stderr,"Can't allocate memory\n");
      exit(1);
    }
    
    if(pthread_create(thpp,&detached_attr,collect_cmd,(void *)cconnfd) !=0) {
      fprintf(stderr,"Can't exec daq collect\n");
      exit(1);
    }
    free(thpp);

  }

 
}

void *collect_cmd(void *arg)
{

  int command;

#if 0
  fprintf(stderr,"enter collect cmd\n");
#endif

  /* Receive command message */

  if(Readn((int)arg,&mb,sizeof(cmd_msgbuf_t)) == 0){
    fprintf(stderr,"blpcollect: connection terminate by the other end!\n");
    close((int)arg);
    return(NULL);
  }

#if 1
  printf("get command from client\n");
#endif
  
  command = mb.command[0];

#if 1
  printf("command = %d \n",command);
#endif

  if( BLP_CMD_VALID(command) ){
    switch(command){
    case BLP_CMD_START:

      pthread_mutex_lock(&run_state->lock);

      if(run_state->status == RUN_OFF){
	run_state->status=RUN_START;

	if(mb.run > 0){
	  fprintf(stdout,"\n ****** Run number:%ld ******\n",mb.run);
	} else {
	  fprintf(stdout,"\n ****** Test run ******\n");
	}

      } 

      fprintf(stdout," Run number : %ld\n",mb.run);
      fprintf(stdout," Comment    : %s\n",mb.comment); 

      pthread_mutex_unlock(&run_state->lock);

      break;

    case BLP_CMD_STOP:    /*  RUN STOP  */

      pthread_mutex_lock(&run_state->lock);
      if((run_state->status ==RUN_ON) || (run_state->status == RUN_START)){
	run_state->status=RUN_STOP;
      }
      pthread_mutex_unlock(&run_state->lock);

      fprintf(stdout,"cmd: run stop\n");
      /* Ack signal */
      break;
    case BLP_CMD_EXIT:    /* RUN EXIT */

      fprintf(stdout,"\n ****** DAQ Exit ****** \n\n");

	/* Ack signal */

      usleep(1000);
	
      close((int)arg);

#if 0
      Send_err(lsockfd,"exit");
      err_close(lsockfd);
#else
      fprintf(stderr,"DAQ Exit.\n");
#endif

      exit(0);
      break;

    case BLP_CMD_RESET: /* RUN RESET  */

      fprintf(stdout,"\n ****** DAQ RESET ****** \n\n");

	/* Ack signal */

      break;
    default:
      fprintf(stderr,"msgrcv:Undefined message \n");

      /* Ack signal */

	break;
    }
  }

  close((int)arg);

  return(NULL);
}
  

void *collect_test(void *arg)
{

  int i,j;
  unsigned long ldata1,ldata2;
  unsigned long count;
  unsigned short buf[BLK_BUF_SIZE];
  unsigned short sbuf[SCL_BUF_SIZE];
  int hsm_word = 6426;
  
  printf("enter collect test\n");

  for(;;){
    /* Check the status of input register */
    /* 1: Buf1 overflow                   */
    /* 2: Buf2 overflow                   */
    /* 3: Buf1,2 overflow                 */

    pthread_mutex_lock(&run_state->lock);
    switch(run_state->status){
    case RUN_START:
	run_state->status = RUN_ON;
	pthread_mutex_unlock(&run_state->lock);

      	/* Send Run Start block */

#if 1
	send_run_start(-1,buf,BLK_BUF_SIZE,mb.comment);
	put_rb_data(rb,buf);
#else
	send_run_start((int)arg,buf,BLK_BUF_SIZE,mb.comment);
#endif


	fprintf(stdout,"\n ******  DAQ START  ****** \n\n");

#if 0
	Send_err(lsockfd,"DAQ start");
#else
	fprintf(stderr,"DAQ start.\n");
#endif

      break;
    case RUN_ON:
      pthread_mutex_unlock(&run_state->lock);

      read_c256(sbuf); /* Read CAMAC scaler */

      /* Dummy data */
      
      for(i=0,j=1;i<hsm_word;i++){
	if(i%17 == 0){
	  buf[i] = (0x8000 | j);
	  if(j==3){
	    j=1;
	  }else{
	    j++;
	  }
	} else {
	  buf[i] = 0x1234;
	}
      }

      memcpy((buf+hsm_word),sbuf,sizeof(unsigned short)*SCL_BUF_SIZE);
#if 1
      put_rb_data(rb,buf);
#else
      send_blk_buf((int)arg,buf,BLK_BUF_SIZE);
#endif
      bzero(buf,sizeof(unsigned short)*BLK_BUF_SIZE);  /* Set zero */
      count++;

      if(nanosleep(&t1,&t2) != 0){ 
	fprintf(stderr,"Nanosleep: Error occured!\n");
	exit(1);
      }
      
      break;
    case RUN_STOP:
      run_state->status = RUN_OFF;
      pthread_mutex_unlock(&run_state->lock);

      /* Send Run End block */
#if 1
      send_run_end(-1,buf,BLK_BUF_SIZE);
      put_rb_data(rb,buf);
#else
      send_run_end((int)arg,buf,BLK_BUF_SIZE);
#endif
      fprintf(stdout,"\n ******  DAQ STOP  ****** \n");

#if 0
      Send_err(lsockfd,"DAQ stop");
#else
      fprintf(stderr,"DAQ stop.\n");
#endif
      count = 0;
	
      break;
    case RUN_OFF:

      pthread_mutex_unlock(&run_state->lock);
      usleep(1000L);

      break;
    default:

      fprintf(stderr,"collect_test: Undefined status!\n");

      break;
    }
  }

  return(NULL);

}



