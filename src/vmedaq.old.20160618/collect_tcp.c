#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "router.h"
#include "tcplib.h"

int connfd;                     /* socket descripter for data tr  */
struct sockaddr_in  cliaddr;    /* Cliend address                 */                    

/*** error type ***/
#define ErrFatal    100
#define ErrSever     20
#define ErrWarning   10
#define ErrMessage    8
#define ErrComment    7
#define ErrLog        6
#define ErrDebug      3
#define ErrVoid       0

void encode_scl_blk(unsigned short *buf, unsigned short cnt);

void *collect_tcp(void *arg)
{
  rtr_inet_info1_t info1;
  rtr_inet_info2_t info2;
  rtr_inet_info3_t info3;
  struct sockaddr_in *in_p;
  struct hostent *hp;
  char name[256];
  char str[256];
  char *ptr;
  int count, n;
  int i;
  /*  int blk_size = (BLK_BUF_SIZE<<1); */
  int blk_size;
  int size = (BLK_BUF_SIZE<<1);
  char *zero = (char *)NULL;
  int client_ver;
  int reply;
  int end_flag;
  unsigned short block_scaler_count = 0;
  unsigned short buf[BLK_BUF_SIZE];

  fprintf(stderr,"collect_tcp start\n");

  /* get client host name */
  in_p = (struct sockaddr_in *)&cliaddr;
  hp = gethostbyaddr((char *)&in_p->sin_addr,sizeof(struct in_addr),AF_INET);
  if(hp != NULL && *hp->h_aliases != (char *)NULL){
    strcpy(name,*hp->h_aliases);
  } else {
    strcpy(name, (char *)(long)inet_ntoa(in_p->sin_addr));
  }

  fprintf(stderr,"Connection request from %s.\n", name);

  /* Request from client */
  count = 0;
  ptr = (char *) &info1;
  while( count < sizeof(info1)){
    if((n = read(connfd, ptr, sizeof(info1)-count)) < 0){ /* wait for the reply */
      fprintf(stderr, "error in read(): %s", strerror(errno));
      return(NULL);
    }
    if(n == 0){
      fprintf(stderr, "Connection was closed by the client on %s", name);
      return(NULL);
    }
    count += n;
    ptr = &ptr[n];
  }
  
  /* Check the version */

  client_ver = ntohl(info1.ver);
  if(client_ver > ROUTER_INET_VERSION_2){
    info2.result = htonl(E_VERSION);
    write(connfd, &info2, sizeof(info2));
    fprintf(stderr, "Client version is too high.");
    return(NULL);
  }

  fprintf(stderr,"client_ver %x\n",client_ver);

  client_ver = client_ver < ROUTER_INET_VERSION_2 ?
    ROUTER_INET_VERSION_1 : ROUTER_INET_VERSION_2;

  if((ptr = (char *)strchr(name,'.')) != (char *)NULL){
    *ptr = 0x00;
  }
  strncpy(str, (char *)info1.user,sizeof(info1.user));
  strcat(str,"@");
  strcat(str,name);
  strcat(str,":");
  strncat(str,(char *) info1.program, sizeof(info1.program));

  info2.result = htonl(E_NOERR);
  info2.blk_size = client_ver == ROUTER_INET_VERSION_1 ?
    htonl(blk_size) : 0;
  info2.res1 = info2.res2 = 0;
  
  /* Send reply */

  if(write(connfd, &info2, sizeof(info2))<0){
    fprintf(stderr, "Error in write(): %s", strerror(errno));
    return(NULL);
  }

  /* Main routine */

  fprintf(stderr,"main routine \n");
  
  end_flag = 0;
  for(i=0; !end_flag; i++){

    /* Get data from ring buffer */
    fprintf(stderr,"get data from rb\n");
    get_rb_data(rb,buf);

    if((buf[0]&0x8000) == 0){ /* If scaler block, must be encode */
      encode_scl_blk(buf,block_scaler_count++);
    } else if((buf[0] == 0xffff) && (buf[2] == 0x0f02)){ /* If run end block, reset scaler count */
      block_scaler_count = 0;
    }
    fprintf(stderr,"send data\n");
    switch(client_ver){
    case ROUTER_INET_VERSION_2:
      /* Send data via TCP/IP (variable length) */

      fprintf(stderr, "ver_2\n");

      fprintf(stderr,"blk count=%d\n",i);

      info3.blk_count = htonl(i);

      fprintf(stderr,"size=%d\n",size);

      info3.blk_size = htonl(size);

      if(write(connfd, &info3, sizeof(info3))<0){
	fprintf(stderr,"Error inwrite(): %s\n",strerror(errno));
      }
      if(size > 0){
	if(write(connfd, buf, size)<0){
	  fprintf(stderr,"Error in write(): %s\n",strerror(errno));
	}
      }

      for(count = 0; count < sizeof(reply); count+=n){
	n = read(connfd, &((char *)&reply)[count],sizeof(reply)-count);
	if(n<0){
	  fprintf(stderr,"Error in read():%s\n",strerror(errno));
	  break;
	}
      }
      reply = ntohl(reply);
      fprintf(stderr,"reply=%d\n",reply);
      if(reply != 0){
	fprintf(stderr, "Error reply rom the client(%d).\n",reply);
      }
      break;
    default:
      fprintf(stderr,"Never come here\n");
      end_flag = 1;
      break;
    }
  }

  if(zero != (char *)NULL){
    free(zero);
    zero = (char *)NULL;
  }

  printf("collect_tcp exit\n");
  
  return(NULL);
}


void *collect_tcp_serv(void *arg){

  int listenfd;
  socklen_t clilen, addrlen;
  struct servent *serv_p;
  struct sockaddr_in name;
  pthread_t *thpp;
  pthread_attr_t detached_attr;
  
  if(htonl(0x1234) == 0x1234) {
  } else {
    fprintf(stderr,"word swap\n");
  }
  
    
  pthread_attr_init(&detached_attr);
  pthread_attr_setdetachstate(&detached_attr,PTHREAD_CREATE_DETACHED);

    /* Create TCP/IP interface to blpctrl */

  if((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
    fprintf(stderr,"Error in socket():%s.\n",strerror(errno));
    fprintf(stderr,"Terminate the process.\n");
    exit(1);
  }

  if((serv_p = getservbyname(ROUTER_DAQDATA_SERVICE, "tcp")) == NULL){
    fprintf(stderr,"Could not find service '%s'. Use the default port.\n",ROUTER_DAQDATA_SERVICE);
    name.sin_port = htons(ROUTER_DAQDATA_PORT);
  } else {
    name.sin_port = htons(serv_p->s_port);
  }

  name.sin_family = AF_INET;
  name.sin_addr.s_addr = INADDR_ANY;
  fprintf(stdout,"Waiting for connection at port %d\n",ntohs(name.sin_port));

  if(bind(listenfd, (struct sockaddr *)&name, sizeof(name))<0){
    close(listenfd);
    fprintf(stderr,"Error in bind():%s\n",strerror(errno));
    fprintf(stderr,"Terminate the processs.\n");
    exit(1);
  }

  if(listen(listenfd,5)<0){
    close(listenfd);
    fprintf(stderr,"Error in listen():%s\n",strerror(errno));
    fprintf(stderr,"Terminate the processs.\n");
    exit(1);
  }
	  
  

  for(;;){
    printf("****** Waiting for the data request ******\n");
  retry:
    clilen = sizeof(cliaddr);
    if((connfd = accept(listenfd,(SA *)&cliaddr,&clilen))<0){
      fprintf(stderr,"grcollect : can't connect tcp client\n");
      goto retry;
      
    }
#if 1
    printf("connection successful \n");
#endif
    
    if((thpp = (pthread_t *)malloc(sizeof(pthread_t))) == NULL){
      fprintf(stderr,"Can't allocate memory\n");
      exit(1);
    }
    
    if(pthread_create(thpp,&detached_attr,collect_tcp,(void *)connfd) !=0) {
      fprintf(stderr,"Can't exec daq collect tcp(%s)\n",strerror(errno));
      exit(1);
    }
    free(thpp);

  }

}

#define SCL_BLK_HEADER_OFFSET 18
#define BLOCK_HEADER_BLOCK_NUM 4

unsigned short scaler_header[SCL_BLK_HEADER_OFFSET] = {
  0xffff, 0x0006, 0x0e00, 0x0094, 0x0000, 0x0001, /* Block header */
  0xffdf, /*... Event Header ID                                   */
  0x0006, /* ... Event Header Size                                */
  0x0001, /* ... Event ID (Block End Event)                       */
  0x008c, /* ... Event Size                                       */
  0x014b, /*... Event Number                                      */
  0x0001, /*... Number of Fields in This Event                    */

  0xffcf, /*... Field Header ID                                   */
  0x0004, /*... Field Header Size                                 */ 
  0x0000, /*... Field ID                                          */
  0x0088, /*... Field Size                                        */

  0x2001, /*... Input Register Region Header (Region Size=1)      */
  0x8000  /*... Input Register Data (Block End Event)             */
};

unsigned short scaler_trailer[4] = {
  0xf001, 0xcb2e, 0xffef, 0x0002
};

void encode_scl_blk(unsigned short *buf, unsigned short cnt)
{
  unsigned short bbuf[BLK_BUF_SIZE];
  unsigned short *ptr;
  unsigned short sum = 0;
  int i;

  scaler_header[BLOCK_HEADER_BLOCK_NUM] = cnt;
  
  for(i=0;i<132;i++){
    bbuf[i] = buf[i];
    sum += buf[i];
  }

  /* Add block header for scaler block  */
  for(i=0;i<SCL_BLK_HEADER_OFFSET;i++){
    buf[i] = scaler_header[i];
  }

  /* Add scaler data                    */
  ptr = &buf[i];
  for(i=0;i<132;i++){
    *ptr++ = bbuf[i];
  }

  /* Add block trailer for scaler block */
  scaler_trailer[1] = sum;
  for(i=0;i<4;i++){
    *ptr++ = scaler_trailer[i];
  }
}
