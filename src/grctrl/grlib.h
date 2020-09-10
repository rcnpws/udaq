/*
 *
 *  grlib.h
 *  Library for GR system
 *
 *    2003/08/26  M.Uchida
 *
 *
 */
#include    <pthread.h>
#include    "tcplib.h"
#include	<sys/types.h>	/* basic system data types */
#include	<sys/time.h>	/* timeval{} for select() */
#include	<time.h>		/* timespec{} for pselect() */
#include	<errno.h>
#include	<fcntl.h>		/* for nonblocking */
#include	<limits.h>		/* PIPE_BUF */
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<unistd.h>
#include	<sys/wait.h>
#include    <sys/shm.h>


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
#define SCL_SHM_KEY   29293249
#define MAX_MSG       256

struct cmd_msgbuf {
  int type;               /* message type */
  char command[1];        /* command      */
  int run;                /* run number   */
  char comment[MAX_MSG];  /* comment      */
};

/*
 *
 * Definitions for socket interface
 *
 */

#define BLK_HEADER_START  0xf01
#define BLK_HEADER_END    0xf02
#define BLK_HEADER_RUN    0xf03
#define BLK_HEADER_BUF1   0xf04
#define BLK_HEADER_BUF2   0xf05
#define BLK_HEADER_BUF3A  0xf06
#define BLK_HEADER_BUF3B  0xf07
#define BLK_HEADER_UNDEF  0xf0f

#if 1    /* Original(TamiDAQ) Version(word) 8kWord(16kbyte) */
#define HSM_WORD 0x2000
#else
#define HSM_WORD 0xd000
#endif

#define MAX_BUFFER   (16)
#define BUF_MARGIN   (0x0)

typedef struct blk_buffer {
  unsigned short buf;
} *Blk_buffer;

struct result {
  unsigned short ack;
};

struct daq_prop {
  short type;
  short pid;
  short sample;                         /* Sampling rate for analyzer */
  char  name[50];
  char  user[20];
  char  ruser[20];
  char  host[30];
};

#if 0   /* Old version versin < 1.00 */
struct daq_state {
  unsigned short i_buf;                 /* Current buffer             */
  unsigned short i_bufn;                /* Next    buffer             */
  unsigned short p_num;                 /* Number of process          */
  unsigned short sem[MAX_BUFFER];       /* Semaphore                  */
  unsigned short ovf[MAX_BUFFER];       /* Over flow flag             */
  unsigned short ref[MAX_BUFFER];       /* Reference                  */
  unsigned short wp[MAX_BUFFER];        /* Write Pointer              */
  unsigned short cnt[MAX_BUFFER];       /* Reference counter          */
  long           blk_id[MAX_BUFFER];    /* Block ID                   */
};

struct daq_state { /* Old version < 1.02 */
  unsigned short i_buf;                 /* Current buffer index       */
  unsigned short run_state;             /* Current run state          */
  unsigned short rec_num;               /* Number of recorder         */
  unsigned short ana_num;               /* Number of analyzer         */
  unsigned short sem[MAX_BUFFER];       /* Semaphore                  */
  unsigned short ref[MAX_BUFFER];       /* Reference                  */
  long           blk_id[MAX_BUFFER];    /* Block ID                   */
};
#else

#define MAX_ANA_NUMBER 10
#define ANA_EMPTY       0
#define ANA_OCCUPIED    1

typedef struct 
{
  pthread_mutex_t state_lock;           /* lock the structure             */
  unsigned short run_state;             /* Current run state              */
  unsigned short recorder_num;	        /* Number of recorder             */
  unsigned short analyzer_num;		/* Number of analyzer             */
  unsigned short collector_num;         /* Number of collector            */
  unsigned short diagnostic_num;        /* Number of diagnostic           */
  int ana_index[MAX_ANA_NUMBER];        /* Analyzer index(empty,occupied) */
  int ana_sample[MAX_ANA_NUMBER];       /* Analyzer sampling reate        */
} daq_state_t;

daq_state_t *daq_state_ptr;             /* Pointer for daq state          */

typedef struct
{
  int sockfd;
  int sample;
} ana_arg_t;

#endif

#define RUN_START  1
#define RUN_STOP   0

typedef struct {
  pthread_mutex_t lock;
  int status;
} run_state_t;

run_state_t *run_state;


#if 0
#define MAX_BLK_ID     0x7fffffff       /* 2147483647(dec)   */
#else 
#define MAX_BLK_ID     0x20000            /* For Debug         */
#endif

#define BUF_NOT_USED    -1

#define DAQ_COLLECTOR    1
#define DAQ_DISKMGR      2
#define DAQ_RECORDER     2
#define DAQ_ANALYZER     3
#define DAQ_LOGGER       4
#define DAQ_DIAGNOSTIC   5
#define DAQ_FIN      -100

#define DTR_REQ            0x100
#define DTR_FIN            0x200
#define DTR_DAQ_REQ        0x300
#define DTR_DAQ_READY      0x400
#define DTR_DAQ_NOT_READY  0x500

/* For general use          */

void show_version(char *ver);
void pr_buf(unsigned short *buf,int i1, int i2,int l);

/* For block transfer       */

void send_blk_buf(int sockfd,unsigned short *buf,int size);
int  ack_blk_buf(int sockfd, unsigned short *buf, int size);
void send_run_start(int sockfd, unsigned short *buf, int size);
void send_run_end(int sockfd, unsigned short *buf, int size);

/* For server child process */

/* Version <= 1.02 */
#if 0
void daq_chld(int sockfd, int shmid,int shmid2);
void disk_chld(void *connfd, int shmid, int shmid2);
void ana_chld(int connfd, int shmid, int shmid2,short sample);
#endif
/* Version > 1.02 */
void *daq_chld(void *arg);
void *disk_chld(void *arg);
void *ana_chld(void *arg);
void *diag_chld(void *arg);
void *broadcast_chld(void *arg);
daq_state_t *new_ds();
void delete_ds(daq_state_t *);
int  increment_recorder(daq_state_t *dsp);
void decrement_recorder(daq_state_t *dsp);
int  increment_analyzer(daq_state_t *dsp);
void decrement_analyzer(daq_state_t *dsp);
int check_blk_id(long blk_id, int i_buf, daq_state_t *ds);


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



