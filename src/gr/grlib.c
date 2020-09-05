#include "grlib.h"
#include <sys/types.h>
#include <time.h>
#define SEND_BLK_SUCCESS 1
#define SEND_BLK_FAIL    0

void show_version(char *ver)
{
  fprintf(stdout,"\n ***** %s *****\n",ver);
}

void pr_buf(unsigned short *buf,int i1, int i2,int l)
{
  int i,line;
  for(i=i1,line=0;i<i2;i++){    /* Sampling output                */
    if(line==0){
      printf("%4o: ",i);
    }
    line++;
    printf("%4x ",buf[i]);
    if(line==l){
      printf("\n");
      line=0;
    }
  }
  printf("\n");
}

void send_blk_buf(int sockfd,unsigned short *buf, int size)
{
  struct result res;

  for(;;){
    Writen(sockfd,buf, sizeof(unsigned short)*size);

    if(Readn(sockfd,&res,sizeof(struct result))==0){
      err_quit("send_blk_buffer: Server terminated!\n");
    }

    if(res.ack == SEND_BLK_SUCCESS){ /* Successful tranfer */
      break;
    } else {
      fprintf(stderr,"send_blk_buffer:Pending buffer transfer\n");
    }

  }
}


int ack_blk_buf(int sockfd, unsigned short *buf, int size)
{
  ssize_t n=0;
  struct result res;

  for(;;){
    if ( (n = Readn(sockfd, buf, sizeof(unsigned short)*size)) == 0)
      return -1;   /* connection closed by other end */

	n = n>>1;
	
    if(n == size){
      res.ack = SEND_BLK_SUCCESS;
      Writen(sockfd, &res, sizeof(struct result));
      break;
    } else {
      res.ack = SEND_BLK_FAIL;
      Writen(sockfd, &res, sizeof(struct result));
    }

  }

#if 0
  if(buf[2] == BLK_HEADER_START){
    n = -100;  /* Run start block */
  } else  if(buf[2] == BLK_HEADER_END){
    n = -200;  /* Run start block */
  }
#endif  

  return n;
}


#define DAQ_VERSION_MAJOR 4 /*  updated 2011.1.10 M.Uchida */
#define DAQ_VERSION_MINOR 0     

void send_run_start(int sockfd, unsigned short *buf, int size,char *comment)
{
  int i;
  int len;
  time_t tl;
  buf[0] = 0xffff;                /* Block Header ID */
#if 1  /* modified 2003/10/01 */
  buf[1] = 6;                     /* Block Header size (fix) */
#else
  buf[1] = 7;                     /* Block Header size (fix) */
#endif
  buf[2] = BLK_HEADER_START;      /* Run Start Block ID      */
  buf[3] = BLK_BUF_SIZE - buf[1]; /* Block size - Block Header Size */
  buf[4] = 0;                     /* Block Number           */
  buf[5] = 0;                     /* Number of Events       */
  buf[6] = 0;                     /* Reserved Word          */
  buf[7] = ((DAQ_VERSION_MAJOR<<8)|(DAQ_VERSION_MINOR));  /* Data Format Version */
  buf[8] = 0x0304;                /* Byte order */
  buf[9] = 0x0102;                /* Byte order */
  time(&tl);
  buf[10] = (unsigned short) (tl >> 8);
  buf[11] = (unsigned short) (tl & 0xff);
  
  len=MAX_MSG;                    
  for(i=0;i<len;i++){
    buf[i+3]=(unsigned short)comment[i];
  }
  for(i=len;i<size-2;i++){
    buf[i] = 0;
  }
  buf[i++] = 0xffef;
  buf[i] = 2;
  if( sockfd > 0){
    send_blk_buf(sockfd,buf,size);
  }
}

void send_run_end(int sockfd, unsigned short *buf, int size)
{
  int i;
  buf[0] = 0xffff;

#if 1  /* modified 2003/10/01 */
  buf[1] = 6;                     /* Block Header size (fix) */
#else
  buf[1] = 7;                     /* Block Header size (fix) */
#endif
  buf[2] = BLK_HEADER_END;
  for(i=3;i<size-2;i++){
    buf[i] = 0;
  }
  buf[i++] = 0xffef;
  buf[i] = 2;
  if( sockfd > 0){
    send_blk_buf(sockfd,buf,size);
  }
}

static cmd_msgbuf_t cmb;

void encode_run_start(unsigned short *buf, int size, cmd_msgbuf_t mb)
{
  int i,i_max;
  int len;
  time_t tl;
  buf[0] = 0xffff;                /* Block Header ID */
#if 1  /* modified 2003/10/01 */
  buf[1] = 6;                     /* Block Header size (fix) */
#else
  buf[1] = 7;                     /* Block Header size (fix) */
#endif
  buf[2] = BLK_HEADER_START;      /* Run Start Block ID      */
  buf[3] = BLK_BUF_SIZE - buf[1]; /* Block size - Block Header Size */
  buf[4] = 0;                     /* Block Number           */
  buf[5] = 0;                     /* Number of Events       */
  buf[6] = 0;                     /* Reserved Word          */
  buf[7] = ((DAQ_VERSION_MAJOR<<8)|(DAQ_VERSION_MINOR));  /* Data Format Version */

  buf[8] = 0x0304;                /* Byte order */
  buf[9] = 0x0102;                /* Byte order */
  time(&tl);
  buf[10] = (unsigned short) (tl >> 16);
  buf[11] = (unsigned short) (tl & 0xffff);

  fprintf(stderr,"Time stamp = %s\n",ctime(&tl));

  memcpy(&cmb,&mb,sizeof(struct cmd_msgbuf));

  
  len=MAX_MSG;                    
  buf[12] = mb.run;
  for(i=0;i<len;i++){
    if(mb.comment[i] == 0){
      i_max = i+13;
      break;
    } else {
      buf[i+13]=(unsigned short)mb.comment[i];
    }
  }

  for(i=i_max;i<size-2;i++){   /* Set zero */
    buf[i] = 0;
  }

  buf[i++] = 0xffef;
  buf[i] = 2;
}
  
void encode_run_end(unsigned short *buf, int size)
{
  int i,i_max;
  int len;
  time_t tl;

  buf[0] = 0xffff;                /* Block Header ID */
#if 1  /* modified 2003/10/01 */
  buf[1] = 6;                     /* Block Header size (fix) */
#else
  buf[1] = 7;                     /* Block Header size (fix) */
#endif
  buf[2] = BLK_HEADER_END;        /* Run End Block ID      */
  buf[3] = BLK_BUF_SIZE - buf[1]; /* Block size - Block Header Size */
  buf[4] = 0;                     /* Block Number           */
  buf[5] = 0;                     /* Number of Events       */
  buf[6] = 0;                     /* Reserved Word          */
  buf[7] = ((DAQ_VERSION_MAJOR<<8)|(DAQ_VERSION_MINOR));  /* Data Format Version */
  buf[8] = 0x0304;                /* Byte order */
  buf[9] = 0x0102;                /* Byte order */
  time(&tl);
  buf[10] = (unsigned short) (tl >> 16);
  buf[11] = (unsigned short) (tl & 0xffff);


  buf[12] = cmb.run;

  len=MAX_MSG;                    

  for(i=0;i<len;i++){
    if(cmb.comment[i] == 0){
      i_max = i+13;
      break;
    } else {
      buf[i+13]=(unsigned short)cmb.comment[i];
    }
  }

  printf("imax:%d \n",i_max);


  for(i=i_max;i<size-2;i++){   /* Set zero */
    buf[i] = 0;
  }

  buf[i++] = 0xffef;
  buf[i] = 2;

  printf("test2\n");
}

