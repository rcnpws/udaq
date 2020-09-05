/*
 *
 * blplib.h
 *  Library for BLP system
 *
 *    2001/10/03  M.Uchida
 *
 *
 */
#include        "tcplib.h"
#include	<sys/time.h>	/* timeval{} for select() */
#include	<limits.h>	/* PIPE_BUF */
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<unistd.h>
#include        <sys/shm.h>


/* Some definition for IPC */

#define BLP_CMD_START 0x01
#define BLP_CMD_STOP  0x02
#define BLP_CMD_RESET 0x03
#define BLP_CMD_INIT  0x04
#define BLP_CMD_EXIT  0x05
#define BLP_CMD_ACK   0x0f
#define BLP_CMD_VALID(a) ((a>0) && (a < 6))

#define BLP_SHM_KEY   20010819
#define BLP_MSG_TYPE1 100
#define BLP_MSG_TYPE2 101
#define DAQ_SHM_KEY   19750120
#define MAX_MSG      256 

#if 0
struct cmd_msgbuf {
  long type;              /* message type */
  char command[1];        /* command      */
  long run;               /* run number   */
  char comment[MAX_MSG];  /* comment      */
};
#else   
typedef struct cmd_msgbuf {
  int  type;              /* message type */
  char command[1];        /* command      */
  int  run;               /* run number   */
  char comment[MAX_MSG];  /* comment      */
} cmd_msgbuf_t;
#endif

/*
 *
 * Definitions for socket interface
 *
 */

#define BLK_HEADER_START   0xf01
#define BLK_HEADER_END     0xf02
#define BLK_HEADER_RUN     0xf03
#define BLK_HEADER_BUF1    0xf04
#define BLK_HEADER_BUF2    0xf05
#define BLK_HEADER_BUF3A   0xf06
#define BLK_HEADER_BUF3B   0xf07
#define BLK_HEADER_EVENT   0xf0e
#define BLK_HEADER_UNDEF   0xf0f

#define BLK_HEADER_SIZE   0x000c              /* Block header size  */
#define BLK_TRAILER_SIZE  0x0002              /* Block trailer size */

#ifndef BLK_BUF_SIZE
#define BLK_BUF_SIZE (0x2000-BLK_HEADER_SIZE) /* Block data size    */
#endif

#if 0     /* Original(TamiDAQ) Version(word) */
#define HSM_WORD  0x2000
#endif

#if 1
#define MAX_BUFFER   (8)
#define BUF_MARGIN   (0)
#endif

typedef struct blk_buffer {
  unsigned short buf;
} *Blk_buffer;

struct result {
  unsigned short ack;
};

struct daq_prop {
  short type;
  short pid;
  short sample;
  char name[50];
  char user[20];
  char ruser[20];
  char host[30];

};

struct daq_state {
  unsigned short i_buf;               /* Current buffer    */
  unsigned short p_num;               /* Number of process */
  unsigned short sem[MAX_BUFFER];    /* Semaphore         */
  unsigned short ovf[MAX_BUFFER];    /* Over flow flag    */
  unsigned short ref[MAX_BUFFER];    /* Reference         */
  unsigned short wp[MAX_BUFFER];     /* Write Pointer     */
  unsigned short cnt[MAX_BUFFER];    /* Reference counter */
};

/* 
 *
 * data structure for event block
 * 2004/02/10 M.Uchida
 *
 */

typedef struct event_block {
  unsigned short header_id;   
  unsigned short header_size; 
  unsigned short header_type; 
  unsigned short block_size;
  unsigned short block_number;
  unsigned short number_of_eve;
#if 1  /* Modified by M. Uchida 2005/07/13 */
  unsigned short reserved_word;
#else
  unsigned short dum;
#endif
  unsigned short format;
  unsigned short order[2];
  unsigned short time[2];
  unsigned short buf[BLK_BUF_SIZE];
} *event_block_t;

#define DAQ_COLLECTOR  1
#define DAQ_DISKMGR    2
#define DAQ_ANALYZER   3
#define DAQ_LOGGER     4

#define DAQ_FIN      -100

#define HSM_BUF_OVF(a,b) (((a)=((b)->ff&0xff)) != 0)
#define SINGLE_BUF_OVF(a) ( (a) == 1 ) || ( (a) == 2 )
#define DOUBLE_BUF_OVF(a) ( (a) == 3 )
#define W_SHIFT(a)  (( (a) >> 16)&0xffff)
#define TOGGLE_BUF(a)  ((a) ^ 0x1)

/* For general use          */

void show_version(char *ver);
void pr_buf(unsigned short *buf,int i1, int i2,int l);

/* For block transfer       */
#if 1   /* Modified by M.Uchida 2005/08/03 */
void encode_run_start(unsigned short *buf, cmd_msgbuf_t mb);
void encode_run_end(unsigned short *buf);
#else
void encode_run_start(unsigned short *buf, int size, cmd_msgbuf_t mb);
void encode_run_end(unsigned short *buf, int size);
#endif

void send_blk_buf(int sockfd,unsigned short *buf,int size);
int  ack_blk_buf(int sockfd, unsigned short *buf, int size);
void send_run_start(int sockfd, unsigned short *buf, int size,char *comment);
void send_run_end(int sockfd, unsigned short *buf, int size);
struct event_block  *init_event_block(void );

/* For command transfer */

int send_cmd_msg(int mqid,struct cmd_msgbuf *mb);

/* For server child process */

void daq_chld(int sockfd, int shmid,int shmid2);
void disk_chld(int connfd, int shmid, int shmid2);
void ana_chld(int connfd, int shmid, int shmid2);

/*  Not Used 2002/01/20    M.Uchida

int  create_inet(struct sockaddr_in *servaddr, char *host);
void daq_connect(int fd, struct sockaddr_in *addr,int type);
int ack_blk_buffer(int fd, unsigned short *buf, unsigned short size);
int ack_blk_trailer(int fd, struct blk_trailer *bt);
int ack_blk_header(int fd, struct blk_header *bh);
void send_blk_trailer(int sockfd);
void send_blk_header(int sockfd, unsigned short bid, unsigned short bsize, unsigned short bnum, unsigned short evnum);
void send_blk_buffer(int sockfd,unsigned short *buf, int size);
void send_msg_buffer(int sockfd,struct blp_msgbuf *mb);
int ack_msg_buffer(int fd, struct blp_msgbuf *mb);

*/

/*
 *  Block header
 */

typedef struct blk_header {
#if 0
  unsigned short h_id=0xffff;   /* Header ID    */
  unsigned short h_size=7;      /* Header size  */
#endif
  unsigned short h_id;   /* Header ID    */
  unsigned short h_size;      /* Header size  */
  unsigned short b_id;                /* Block ID     */
  unsigned short b_size;              /* Block size   */
  unsigned short b_num;               /* Block number */
  unsigned short ev_num;              /* Event number */
  unsigned short ev_flag;             /* Event flag   */
} *Blk_header;


typedef struct blk_trailer {
#if 0
  unsigned short h_id=0xffef;  /* Header ID    */
  unsigned short h_size=2;     /* Header size  */
#else
  unsigned short h_id;  /* Header ID    */
  unsigned short h_size;     /* Header size  */
#endif

} *Blk_trailer;


#if 0
#define SCL_BUF_SIZE  132        /* Buffer size for VME scaler     */
#else
#define SCL_BUF_SIZE  264        /* Buffer size for VME scaler     */
#endif

