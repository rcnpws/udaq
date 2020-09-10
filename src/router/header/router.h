/*
	router.h ... Definitions for Router (Global Use)
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  24-JAN-1995 by A. Tamii (for Solaris 2.3)
  Version 2.0  20-JAN-1997 by A. Tamii (Change long to int for 64bit Deg Unix)
*/

#include <sys/ipc.h>
#include <sys/msg.h>

/*** constant definitions ***/
#ifndef TRUE
#define FALSE 0
#define TRUE  1
#endif

#ifndef O_SAMPLE
#define O_ALL    0x00              /* transfer all blocks */
#define O_SAMPLE 0x08              /* sampling connection */
						/* must be consistent with the value defined in rfmtypes.h */
#endif

#define ROUTER_DAQDATA_PORT  1126   /* default daqdata port */
#define RTR_NAME_LEN         31

/*** message buffer block ***/
typedef struct rtr_msg{
#ifdef OSF1
	mtyp_t   mtype;        /* mtype=target pid, if mtype==1, target=router */
#else
	long     mtype;        /* mtype=target pid, if mtype==1, target=router */
#endif
	long     pid;          /* sender process ID */
	long     cmd;          /* command to router or reply from router */
	long     arg[3];       /* optional arguments */
	char     name[RTR_NAME_LEN+1]; /* message sender/target name */
  long     reserved[18];
} rtr_msg_t;

/*** first information via TCP/IP ***/
typedef struct rtr_inet_info1{
	int      ver;          /* version = (1.0 or 2.0)* 256 */
	short    unit;         /* rfm unit  (0 for ver 2.0)*/
	short    stream;       /* rfm stream number (0 for ver 2.0)*/
  int      mode;         /* open mode */
	char     router[4];    /* name of the router (when stream=0) */
	char     user[16];     /* user name */
	char     program[16];  /* program name */
  char     uname[16];    /* user name on the host*/
} rtr_inet_info1_t;

typedef struct rtr_inet_info2{
	int      result;       /* result */
	int      blk_size;     /* 1 block size (fixed) */
  int      res1;         /* reserved */
	int      res2;         /* reserved */
} rtr_inet_info2_t;

typedef struct rtr_inet_info3{
	int      blk_count;    /* total count of the sent blocks */
	int      blk_size;     /* 1 block size (fixed) */
  int      res[14];      /* reserved */
} rtr_inet_info3_t;

/*** router messages ***/
#define RTR_CONNECT    1     /* Request Connection to Router */
#define RTR_DISCONNECT 2     /* Request Disconnection to Router */
#define RTR_REFUSE     3     /* Refuse connection */
#define RTR_BUFFER     4     /* Send Buffer Information to Consumer */
#define RTR_RELEASE    5     /* Release Buffer */
#define RTR_GETBUF     6     /* Request Buffer to Write */
#define RTR_SUSPEND    7     /* Suspend to Give Buffer */
#define RTR_STATUS     50    /* Show Status */
#define RTR_TERMINATE  100   /* Terminate Router Process */

#define ROUTER_VERSION     ((3<<8)|0)  /* Router version */
#define ROUTER_VERSION_MIN ((3<<8)|0)  /* Minimum client version */

/*** TCP/IP Message ***/
#define E_NOERR        0     /* No error */
#define E_VERSION      1     /* Version is too high */
#define E_INVAL        2     /* Invalid Data */
#define E_NODEV        3     /* No such router */
#define E_NOMEM        4     /* No more memory is available */

#define ROUTER_INET_VERSION_1  (1<<8)
#define ROUTER_INET_VERSION_2  (2<<8)

#define ROUTER_DAQDATA_SERVICE "daqdata"
#define ROUTER_DAQMSG_SERVICE  "daqmsg"

/*** message types ***/
#define RTR_MTYPE_ROUTER  1  /* target process = router */

#define remove_item(item) { \
	(item)->prev->next = (item)->next; \
	(item)->next->prev = (item)->prev; \
	}
#define insert_item(place,item) { \
	(item)->prev = (place)->prev; \
	(item)->next = (place); \
	(item)->prev->next = (item); \
	(item)->next->prev = (item); \
}

/*** library functions ***/
key_t get_key(char*, uid_t uid);
void *get_shmbuf(int);
int router_connect(char *router, uid_t uid, char *name, int mode);
void router_disconnect(int fd);
int router_get_buf(int fd, void *buf, long *size);
int router_request_buf(int fd, long size);
int router_release_buf(int fd, void *buf, long);
