#include "tcplib.h"
#include "blplib.h"
#include "errlog.h"

struct log_prop *create_log_prop(char *pname, char *uname)
{
  struct log_prop *lp;

  if((lp = (struct log_prop *)malloc(sizeof(struct log_prop))) == NULL){
    return(NULL);
  }
  
  strcpy(lp->name,pname);
  strcpy(lp->user,uname);
  gethostname(lp->host,30);
  sprintf(lp->comment,"%s started at %s by %s\n",lp->name,lp->host,lp->name);

  return lp;
}

void delete_log_prop(struct log_prop *lp)
{
  free(lp);
}


int err_open(char *ename,struct log_prop *prop)
{
  int fd;
#if 0
  fd = Tcp_connect(LOG_SERVER,SERV_PORT_LOG);
#else
  fd = Tcp_connect(ename,SERV_PORT_LOG);
#endif
  Send_err_msg(fd,prop);
  return fd;
}

void err_close(int fd)
{
  Close(fd);
}

int send_err_msg(int fd, struct log_prop *lp)
{
  int dum;

  Writen(fd, lp,sizeof(struct log_prop));

  if(Readn(fd,&dum, sizeof(int))==0){
    return -1;     /* NG   */
  } else {
    return 0;      /* OK   */
  }
}

void Send_err_msg(int fd, struct log_prop *lp)
{
  if(send_err_msg(fd,lp)== -1){
    fprintf(stderr," send error occured \n");
    exit(1);
  }
}

void rcv_err_msg(int fd, struct log_prop *lp)
{
  int dum;
  
  if(Readn(fd,lp,sizeof(struct log_prop))==0){
        err_quit("No daq proparty: Server terminated!\n");
  } else {
        Writen(fd, &dum, sizeof(int));
  }
}

int rcv_err(int fd, char *arg)
{
  int dum;
  size_t len = 256;
  char str[256];  
  
  if(Readn(fd,str,len)==0){
	return -1;
  } else {
	Writen(fd, &dum, sizeof(int));
  }
  strcpy(arg,str);
  return 0;
}

int send_err(int fd, char *arg)
{
  size_t len =256;
  char str[256];
  int dum;
  strcpy(str,arg);

  
  Writen( fd, str, len);

  if(Readn(fd,&dum, sizeof(int))==0){
    return -1;     /* NG   */
  } else {
    return 0;      /* OK   */
  }  
}

  
void Send_err(int fd, char *arg)
{
  if(send_err(fd,arg)== -1){
    fprintf(stderr," send error occured \n");
    exit(1);
  }
}
