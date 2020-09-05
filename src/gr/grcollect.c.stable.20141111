/*
 *
 *  grcollect.c 
 *     GR collector program 
 *   Ver0.1        2003/08/16    M.Uchida   
 *   Ver0.2        2003/08/26    M.Uchida   API for Tamidaq ver2.0
 *   Ver1.0        2003/08/31    M.Uchida   variable q-size for ring buf 
 *   Ver2.0        2009.05.04    M.Uchida   VMIVME7807
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#define _XOPEN_SOURCE 500
#include <signal.h>

#include "tcplib.h"
#include "ipclib.h"
#include "grlib.h"
#include "errlog.h"
#include "ring_buf.h"
#include "run_state.h"


#ifndef BLK_BUF_SIZE
#define BLK_BUF_SIZE 0x2000
#endif

#define GR_BYTE_SWAP 1
void grByteSwap(char *str);

#define SPARC_64BIT                /* SPARC 64bit mode                */
#define SCL_BUF_SIZE  132          /* Buffer size for VME scaler      */

char version[30]="GR COLLECTOR Ver 3.0";
#if 0
char version[30]="GR COLLECTOR Ver 2.0";
#endif
struct timespec t1,t2;             /* For nanosleep                   */
cmd_msgbuf_t mb;                   /* Command message que buffer      */
ring_buf_t *rb;                    /* ring buffer pointer             */
int max_que_num=64;                /* max que number for ring buffer  */
int local_flag = 0;                /* Local mode flag                 */
int quiet_flag;                    /* Quiet mode flag                 */

void *collect_data(void *arg);     /* Main collector routine          */
void *collect_cmd(void *arg);      /* Command handling routine        */
void *collect_test(void *arg);     /* Main collector routine(test)    */
void *collect_tcp(void *arg);      /* Data transfer routine(TCP/IP)   */
void *collect_tcp_serv(void *arg); /* Data transfer routine server    */

#if 0                              /* For remote logger (not used)    */
int lsockfd;
struct log_prop *lp;

int connfd;                        /* socket descripter for data tr   */
struct sockaddr_in  cliaddr;       /* Cliend address                  */
#endif


void usage(char *arg)
{
  fprintf(stderr,"Usage: %s [-h] [-d] [-t] [-l] [-s [time]sec] [-m [time]msec] [-u [time]usec] [-q que_num] [-Q] \n",arg);
  fprintf(stderr,"-h   : help\n");
  fprintf(stderr,"-t   : test mode\n");
  fprintf(stderr,"-l   : local mode\n");
  fprintf(stderr,"-d   : debug mode\n");
  fprintf(stderr,"-s   : interval[sec]\n");
  fprintf(stderr,"-m   : interval[msec]\n");
  fprintf(stderr,"-u   : interval[usec]\n");
  fprintf(stderr,"-q   : ring buffer size(default: 64)\n");
  fprintf(stderr,"-Q   : Quiet mode\n");
  exit(1);
}


void sighdl( arg )               /* Signal handler                 */
{
  printf("Interrupted. Signal ignored.\n");
}

int main(int argc, char **argv)
{
  long interval;                   /* Interval ( dummy data only)    */
  int  test_flag=0;                /* Option switch for test mode    */
  //char host[256];                  /* server host name               */
  char pname[256];                 /* process name                   */
  char ename[256]=LOG_SERVER;      /* error log host name (not used) */

  int listenfd,cconnfd;            /* Socket descripter              */
  int sockfd;                      /* Socket file desc               */
  socklen_t cclilen, caddrlen;     /* Command client/addr length     */
  struct sockaddr_in  ccliaddr;    /* Command client address         */

  pthread_t *thp,*thpp,*thppp;     /* Thread pointers                */
  pthread_attr_t detached_attr;    /* Thread attribute               */
  int ic;
  char c;

  show_version(version);

  /*
   *
   * Command argument
   *
   */

  strcpy(pname,argv[0]);


  while((ic = getopt(argc,argv,"tle:s:m:u:q:Qh")) != EOF){
    c = (char) ic;

    switch(c){

    case 't':  /* test mode on */
      test_flag = 1;
      t1.tv_sec  = 1;
      t1.tv_nsec = 0;
      break;

    case 'l':  /* local mode on */
      local_flag = 1;
      fprintf(stderr,"grcollect: set local mode.\n");
      break;
    case 'e':  /* Error logger host             */

      strcpy(ename,optarg);
      break;
    case 's':  /* interval in sec (test mode)   */

      interval = atoi(optarg);
      t1.tv_sec  = interval;
      t1.tv_nsec = 0;
      break;
    case 'm':  /* interval in msec (test mode)  */

      interval = atoi(optarg);
      t1.tv_sec  = interval/1000;
      t1.tv_nsec = (interval%1000)*1000000;
      break;
    case 'u': /* interval in usec (test mode)   */

      interval = atoi(optarg);
      t1.tv_sec  = interval/1000000;
      t1.tv_nsec = (interval%1000000)*1000;
      break;

    case 'q': /* max que number for ring buffer */

      max_que_num = atoi(optarg);
      break;
    case 'Q': /* Quiet mode on */

      quiet_flag = 1;
      break;
    case 'h': /* help: print usage              */
      usage(pname);
      break;
    default:     
      fprintf(stderr,"default\n");
      usage(pname);
      break;
    }
  }


  fprintf(stderr,"max que number:%d\n",max_que_num);

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


  rb = new_rb();             /* Create ring buffer            */

  sigset(SIGINT, sighdl);    /* Set signal (ignore sigint)    */

#if 0                        /* For remote logger (not used)  */

  if((lp = create_log_prop(pname,getenv("USER")))==NULL){
    	fprintf(stderr,"blpsave : Can't allocate memory\n");
	exit(1);
  }

  lsockfd = err_open(ename,lp);
  Send_err(lsockfd,"launched");

#endif

  /* Initialize thread attribution (detatched attr) */

  pthread_attr_init(&detached_attr);
  pthread_attr_setdetachstate(&detached_attr,PTHREAD_CREATE_DETACHED);

  thp = (pthread_t *)malloc(sizeof(pthread_t));




  if(test_flag == 1){
    fprintf(stderr,"Collector running in test mode \n");
    if(pthread_create(thp,&detached_attr,collect_test,(void *)sockfd) !=0) {
      fprintf(stderr,"Can't exec collect_test\n");
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



  /* Start TCP data transfer server */
  if((thppp = (pthread_t *)malloc(sizeof(pthread_t))) == NULL){
    fprintf(stderr,"Can't allocate memory\n");
    exit(1);
  }

  if( local_flag == 0 ){  /* Only remote setting, start collect_tcp_serv */
    if(pthread_create(thppp,&detached_attr,collect_tcp_serv,(void *)sockfd) !=0) {
      fprintf(stderr,"Can't exec daq collect\n");
      exit(1);
    }
    free(thppp);
  }


  /* Create TCP/IP interface to command control client */
  
  listenfd = Tcp_listen(NULL,SERV_PORT_CTRL,&caddrlen);



  /* Waiting  for control command from client */  
  for(;;){
    printf("****** Waiting for the control command ******\n");
  retry:
    cclilen = sizeof(ccliaddr);
    if((cconnfd = accept(listenfd,(SA *)&ccliaddr,&cclilen))<0){
      fprintf(stderr,"grcollect : can't connect cmd client\n");
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
  char dumStr[4];

#if 0
  fprintf(stderr,"enter collect cmd\n");
#endif

  /* Receive command message */

  printf("size = %d\n",sizeof(struct cmd_msgbuf));
  if(Readn((int)arg,&mb,sizeof(cmd_msgbuf_t)) == 0){
    fprintf(stderr,"grcollect: connection terminate by the other end!\n");
    close((int)arg);
    return(NULL);
  }


  if(GR_BYTE_SWAP == 1){
    dumStr[0] = (char) ((mb.run & 0xff000000) >> 24);
    dumStr[1] = (char) ((mb.run & 0xff0000) >> 16);
    dumStr[2] = (char) ((mb.run & 0xff00) >> 8);
    dumStr[3] = (char) (mb.run & 0xff);
    mb.run = (unsigned long) (dumStr[1]<<24) || (dumStr[0]<<16) ||
      (dumStr[3]<<8) || (dumStr[2]);
  }

  command = mb.command[0];

  if(quiet_flag==0){
    printf("get command from client\n");
    printf("command = %d \n",command);
  }


  if( GR_CMD_VALID(command) ){
    switch(command){
    case GR_CMD_START:

      pthread_mutex_lock(&run_state->lock);

      if(run_state->status == RUN_OFF){
	run_state->status=RUN_START;

	
	printf("mb.run %ld \n",mb.run);
	if(mb.run > 0){
	  fprintf(stderr,"\n ****** Run number:%lx ******\n",mb.run);
	} else {
	  fprintf(stderr,"\n ****** Test run ******\n");
	}

      } 

      fprintf(stderr," Run number : %ld\n",mb.run);
      fprintf(stderr," Comment    : %s\n",mb.comment); 


      pthread_mutex_unlock(&run_state->lock);

      break;

    case GR_CMD_STOP:    /*  RUN STOP  */

      pthread_mutex_lock(&run_state->lock);
      if((run_state->status ==RUN_ON) || (run_state->status == RUN_START)){
	run_state->status=RUN_STOP;
      }
      pthread_mutex_unlock(&run_state->lock);

      if(quiet_flag == 0){
	fprintf(stderr,"cmd: run stop\n");
      }
      /* Ack signal */
      break;
    case GR_CMD_EXIT:    /* RUN EXIT */

      fprintf(stderr,"\n ****** DAQ Exit ****** \n\n");

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

    case GR_CMD_RESET: /* RUN RESET  */

      fprintf(stderr,"\n ****** DAQ RESET ****** \n\n");

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

  unsigned long count;
  unsigned short buf[BLK_BUF_SIZE];
  //int hsm_word = 6426;
  
  printf("Enter collect test\n");

  for(;;){

    pthread_mutex_lock(&run_state->lock);
    switch(run_state->status){
    case RUN_START:
	run_state->status = RUN_ON;
	pthread_mutex_unlock(&run_state->lock);

      	/* Send Run Start block */


	send_run_start(-1,buf,BLK_BUF_SIZE,mb.comment);
	if(local_flag == 0){
	  put_rb_data(rb,buf);
	}

	fprintf(stdout,"\n ******  DAQ START  ****** \n\n");

#if 0
	fprintf(stderr,"DAQ start.\n");
#endif


      break;
    case RUN_ON:
      pthread_mutex_unlock(&run_state->lock);

#if 0
      read_c256(sbuf); /* Read CAMAC scaler */
#endif

      /* Dummy data */

#if 0      
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
#else
      buf[0] = 0xffff;
#endif

      if(local_flag==0){
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
	put_rb_data(rb,buf);
      }

#if 0
      bzero(buf,sizeof(unsigned short)*BLK_BUF_SIZE);  /* Set zero */
#endif
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


void grByteSwap(char *str){

  char tmp;
  int i;

  for(i=0;i<256;i+=2){
    tmp = str[2*i];
    str[2*i] = str[2*i+1];
    str[2*i+1] = tmp;

  }
}
  
  
  
  


