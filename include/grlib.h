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

#define GR_CMD_START 0x01
#define GR_CMD_STOP  0x02
#define GR_CMD_RESET 0x03
#define GR_CMD_INIT  0x04
#define GR_CMD_EXIT  0x05
#define GR_CMD_ACK   0x0f
#define GR_CMD_VALID(a) ((a>0) && (a < 6))

#define GR_SHM_KEY   20010819
#define GR_MSG_TYPE1 100
#define GR_MSG_TYPE2 101
#define DAQ_SHM_KEY   19750120
#define MAX_MSG       256

#if 0
typedef struct cmd_msgbuf {
  int  type;              /* message type */
  char command[1];        /* command      */
  char runString[256];    /* run number   */
  unsigned long  run;     /* run number   */
  char comment[MAX_MSG];  /* comment      */
} cmd_msgbuf_t;
#else 
typedef struct cmd_msgbuf {
  int  type;              /* message type */
  char command[1];        /* command      */
  unsigned long  run;     /* run number   */
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
#define BLK_HEADER_UNDEF   0xf0f

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

void encode_run_start(unsigned short *buf, int size, cmd_msgbuf_t mb);
void encode_run_end(unsigned short *buf, int size);

/* For block transfer       */

void send_blk_buf(int sockfd,unsigned short *buf,int size);
int  ack_blk_buf(int sockfd, unsigned short *buf, int size);
void send_run_start(int sockfd, unsigned short *buf, int size,char *comment);
void send_run_end(int sockfd, unsigned short *buf, int size);

/* For command transfer */

int send_cmd_msg(int mqid,struct cmd_msgbuf *mb);

/*
 *  Block header
 */

typedef struct blk_header {
#if 0
  unsigned short h_id=0xffff;         /* Header ID    */
  unsigned short h_size=7;            /* Header size  */
#endif
  unsigned short h_id;                /* Header ID    */
  unsigned short h_size;              /* Header size  */
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


#define	SERV_PORT_CTRL "9879"	   /* TCP  client-servers for logging */
/* #define CTRL_SERVER      "frc03.rcnp.osaka-u.ac.jp" */
#define CTRL_SERVER      "vmelas.rcnp.osaka-u.ac.jp" 
