/*
 *
 *  grctrl.c 
 *     GR control program 
 *   Ver1.0        2003/08/26    M.Uchida   
 *
 *   Modified for Linx (conversion of run number by htonl);
 *   on 10-OCT-2011 by A. Tamii
 */

#include "tcplib.h"
#include "ipclib.h"
#include "grctrl.h"
#include "grlib.h"

const char version[30]="GR CONTROL Ver 1.0";

void usage(char *prog)
{
  fprintf(stderr,"Usage : %s <Command> <run number> <comment>\n",prog);
  fprintf(stderr,"start : Start DAQ\n");
  fprintf(stderr,"stop  : Stop DAQ\n");
  fprintf(stderr,"reset : Reset DAQ\n");
  exit(1);
}
  
int main(int argc, char **argv)
{
  
  int command=GR_CMD_START;
  char c,*str;
  int i;
  int sockfd;
  int dum;
  char *ctrl_server;
  
  /* Some variable for IPC */

  int mqid;                    /* Message que ID           */
  struct cmd_msgbuf mb;        /* Message que buffer       */


#if 0
  show_version(version);
#endif 

  if (argc < 2){
    usage(argv[0]);
  }

  /* Create Message que   */
#if 0
  mqid = Msgget((key_t) GR_SHM_KEY, SVMSG_MODE);
#endif

  /* Open TCP connection */  

  if((ctrl_server = (char *)malloc(sizeof(char)*100))==0){
    fprintf(stderr,"Can't allocate memory ctrl_server\n");
    exit(1);
  }



  if((ctrl_server = getenv("GR_CTRL_SERVER"))==NULL){
    sockfd = Tcp_connect(GR_CTRL_SERVER,SERV_PORT_CTRL);
  }else{
    printf("%s\n",ctrl_server);
    sockfd = Tcp_connect(ctrl_server,SERV_PORT_CTRL);
  }

  if(strcmp(argv[1],"start")==0){
    command=GR_CMD_START;
    for(i=0;i<MAX_MSG;i++){
      mb.comment[i] = 0;
    }

    mb.run = htonl(atoi(strtok(argv[2],",")));  /* run number */
#if 0
    str = strtok(NULL,",");
#else  /* changed by A. Tamii on 2008.03.04 */
    str = strtok(NULL,"\0");
#endif
    if(str==(char*)NULL){ 
       /* Added on 2013.4.19 by A. Tamii for treating no comment */
      strcpy(mb.comment,"");
    }else if(strlen(str) < MAX_MSG){
      strcpy(mb.comment,str);
    } else {

      fprintf(stderr,"Comment length is too long.\nMust be less than 256 words.\n");
      exit(1);
    }

#if 0
fprintf(stderr, "3\n");
    fprintf(stdout," Run number : %ld\n",ntohl(mb.run));
    fprintf(stdout," Comment    : %s\n",mb.comment); 
    fprintf(stdout," OK?(y/n)?  ");
    scanf("%c",&c);
    if( (c == 'y') || (c == 'Y')){
    } else {
      fprintf(stdout,"Please type y or Y\n");
      exit(0);
    }
#endif

  }else if(strcmp(argv[1],"stop")==0){
    command=GR_CMD_STOP;
  }else if(strcmp(argv[1],"reset")==0){
    command=GR_CMD_RESET;

  }else if(strcmp(argv[1],"init")==0){
    command=GR_CMD_INIT;
  }else if(strcmp(argv[1],"exit")==0){
    command=GR_CMD_EXIT;

  } else {
    fprintf(stderr,"Unexpected command\n");
    usage(argv[0]);
  }


  /* Send log message to blplog */


  /* Send command message to blpcollect */

  mb.command[0]=command;
#if 0
  if(send_cmd_msg(mqid,&mb)==-1){
    fprintf(stderr,"Message transfer failed!\n");
    exit(1);
  }


#else
  /*  printf("size = %d\n",sizeof(struct blp_msgbuf)); */
  Writen(sockfd,&mb,sizeof(struct cmd_msgbuf));
#endif

  return 0;
}

