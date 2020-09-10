/*
	router_priv.h ... Definitions for Router (Private Use)
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  24-JAN-1995 by A. Tamii (for Solaris 2.3)
	Ver 2.00  31-OCT-1995 by A. Tamii
*/


/*** message queue ***/
#define RTR_MNOSENDMSG   1000   /* max number of messages in the queue */

/*** shared memory ***/
                               /* size of the shared memroy (default_)*/
//#define RTR_DEF_SHMSIZE    (0x4000*128)
#define RTR_DEF_SHMSIZE    (0x100000*64) // enlarged the size on 2017.12.28 (64MB)

#define RTR_MAXSAMPBUF    30   /* max number of buffers can be sent to sampling
																		 readers at once */
#define RTR_MAXSENDBUF   100   /* max number of buffers can be sent to must
																		 readers */
#define RTR_MAXWRITEBUF    5   /* max number of buffers can be sent to write
																		 writers */
#define RTR_ALARM_TIME     1   /* timeout of msgrcv in sec */
#define RTR_TMO           60   /* No response timeout (in sec) */

#define  WAITMSG           0   /* show message of pending */

/* shared memory buffer */
typedef struct rtr_shmbuf{
	struct rtr_shmbuf *next;
	struct rtr_shmbuf *prev;
	long   buf;        /* buffer position from the top of the shared memory */
	long   size;       /* buffer size */
	int    count;      /* number of reference count */
} rtr_shmbuf_t;

/*** buffer list ***/
typedef struct rtr_buf{
	struct rtr_buf *next;
	struct rtr_buf *prev;
	rtr_shmbuf_t   *shm;         /* pointer to the shared memory record */
} rtr_buf_t;

/*** process list ***/
typedef struct rtr_proc{
 	struct rtr_proc *next;       /* tag to the next data */
	struct rtr_proc *prev;       /* tag to the previous data */
	pid_t      pid;              /* process ID */
	int        flag;             /* O_RDONLY, O_WRONLY, O_SAMPLE */
	int        cnt;              /* number of buffers currently grabbing */
	int        cnt_total;        /* total num. of buffers sent to the process */
	time_t     c_time;           /* time when connected*/
	time_t     l_time;           /* time when most recently sent a buffer */
	rtr_buf_t  bl;               /* buffer list currently grabbing */
	char   name[RTR_NAME_LEN+1]; /* process name */
} rtr_proc_t;

/*** macro functions ***/
/* remove the item from the list */
#define remove_item(item) { \
	(item)->prev->next = (item)->next; \
	(item)->next->prev = (item)->prev; \
	}
/* insert the item before the place */
#define insert_item(place,item) { \
	(item)->prev = (place)->prev; \
	(item)->next = (place); \
	(item)->prev->next = (item); \
	(item)->next->prev = (item); \
}

