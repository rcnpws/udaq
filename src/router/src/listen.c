/*
	list.c ... Message Listener of Router
  Copyright (C) 1995, 1996  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  24-JAN-1995 by A. Tamii (for Solaris 2.3)
  Version 2.00 27-OCT-1996 by A. Tamii  # Separate Writer and support OSF1
  Version 3.00 14-JUL-1997 by A. Tamii  # Use two message queues for in/out
  Version 3.01 15-NOV-1997 by A. Tamii  # Show status when hang up
  Version 3.02 15-NOV-1997 by A. Tamii  # free msg (bug fix)
  Version 3.10 20-DEC-1999 by A. Tamii  # 
  Version 4.00 30-OCT-2000 by A. Tamii  # for AIX
  Version 4.10 08-NOV-2000 by A. Tamii  # bugfix for AIX
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#if 0
#include <synch.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

#include "router.h"
#include "router_priv.h"
#include "errlog.h"


enum{
	InKBytes = 0,
	InBytes,
	OutKBytes,
	OutBytes,
	InBlocks,
	OutBlocks,
	NumStat
};

static long  status[NumStat];

typedef struct rtr_msgbuf{
	struct rtr_msgbuf   *prev;
	struct rtr_msgbuf   *next;
	rtr_msg_t           msg;
} rtr_msgbuf_t;

extern rtr_proc_t *pl_head;

extern int          msg_count;
extern rtr_shmbuf_t *shm_used;
extern rtr_shmbuf_t *shm_free;

extern int   shmsize;

extern int   msg_id_in;
extern int   msg_id_out;
extern char     proc_name[32];

extern int max_num_msg;

#define num_msg_limit (max_num_msg)

static int end_flag;
static rtr_msgbuf_t mb_head_d;
static rtr_msgbuf_t *mb_head = &mb_head_d;

static rtr_msgbuf_t que_head_d;
static rtr_msgbuf_t *que_head = &que_head_d;
static int          que_count;

/* Initialyze Statistics */
void init_stat(){
	int   i;
	for(i=0; i<NumStat; i++)
		status[i] = 0;
}

/* Show Statistics */
void show_stat(){
	errout(ErrComment,
				 "Statistics:");
	errout(ErrComment,
				 "         In(KBytes)   In(Blks)   Out(KBytes)  Out(Blks)");
	errout(ErrComment, "     %13d %10d %13d %10d",
				 status[InKBytes], status[InBlocks], status[OutKBytes], status[OutBlocks]);
}

/* for alarm() SIGALRM */
static void nullfcn(){
}

/* show buffer status (for debug) */
void show_buf_cnt(){
	rtr_shmbuf_t  *shm;
	int           used, free;
	
	used=free=0;
	shm = shm_used;
	for(used=0; (shm=shm->next)!=shm_used; used++);
	shm = shm_free;
	for(free=0; (shm=shm->next)!=shm_free; free++);
	errout(ErrComment, "Used buf = %2d, Free buf = %2d.");
}

void show_buf(){
	rtr_shmbuf_t  *shm;
	int           i;
	
	errout(ErrComment, "");
	errout(ErrComment, "--- Show status of the router: %s ---", proc_name);
	errout(ErrComment, "");
	errout(ErrComment, "     shared memory   size (ref)");
	errout(ErrComment, "Used Buf:");
	shm = shm_used;
	for(i=0;  (shm=shm->next)!=shm_used; i++){
		errout(ErrComment, " %2d) %6x-%6x %6x (%3d)",
					 i, shm->buf, shm->buf+shm->size, shm->size, shm->count);
	}

	errout(ErrComment, "Free Buf:");
	shm = shm_free;
	for(i=0;  (shm=shm->next)!=shm_free; i++){
		errout(ErrComment, " %2d) %6x-%6x %6x (%3d)",
					 i, shm->buf, shm->buf+shm->size, shm->size, shm->count);
	}
	errout(ErrComment, "");
}

void show_pl(){
	rtr_proc_t    *pl;
	rtr_buf_t     *bl;
	int           i;
	char          tc[10], tl[10];
	
	errout(ErrComment, "Process List:");
	errout(ErrComment,
				 "     pid            name              nbuf(ref)    connect  last-access"),
	pl = pl_head;

	while((pl=pl->next)!=pl_head){
		strncpy(tc, &ctime(&pl->c_time)[11], 8);
		strncpy(tl, &ctime(&pl->l_time)[11], 8);
		tc[8]=tl[8]=0;
		errout(ErrComment, " %c %5d  %-25s %6d(%3d)   %8s  %8s",
					 pl->flag & O_WRONLY ? 'W' : pl->flag & O_SAMPLE ? 'S' : 'R',
					 pl->pid, pl->name, pl->cnt_total, pl->cnt, tc, tl);
	}
	errout(ErrComment, "");
}

void show_msg(){
	errout(ErrComment, "Message Queue:");
	errout(ErrComment, "  message count = %d/%d", msg_count, num_msg_limit);
	errout(ErrComment, "");
}

char *commands[] = {
	"   null", "connect", " discon", " refuse", " buffer",
	"release", " getbuf", "suspend", "unknwon", "unknwon"
};

char *str_cmd(cmd){
	if(0<cmd && cmd<10)
		return commands[cmd];
	switch(cmd){
	case RTR_STATUS:
		return " status";
	case RTR_TERMINATE:
		return "   term";
	default:
		return "unknwon";
	}
}

void show_pend(){
	rtr_msg_t     *msg;
	rtr_msgbuf_t  *mb;
	int           i, cmd;
	
	errout(ErrComment, "Pending Message: IN");
	errout(ErrComment,
					"      mtype     pid      cmd    arg1    arg2    arg3   name");
	for(i=1, mb=mb_head->next; mb!=mb_head; i++, mb=mb->next){
		msg = &mb->msg;
		errout(ErrComment, "%3d)  %5d   %5d  %7s  %6d  %6d  %6d   %s",
					 i, msg->mtype, msg->pid, str_cmd(msg->cmd),
						msg->arg[0], msg->arg[1], msg->arg[2], msg->name);
	}
	errout(ErrComment, "");

	errout(ErrComment, "Pending Message: OUT");
	errout(ErrComment,
					"      mtype     pid      cmd    arg1    arg2    arg3   name");
	for(i=1, mb=que_head->next; mb!=que_head; i++, mb=mb->next){
		msg = &mb->msg;
		errout(ErrComment, "%3d)  %5d   %5d  %7s  %6d  %6d  %6d   %s",
					 i, msg->mtype, msg->pid, str_cmd(msg->cmd),
						msg->arg[0], msg->arg[1], msg->arg[2], msg->name);
	}
	errout(ErrComment, "");
}

void show_status(){
  show_buf();
	show_pl();
	show_msg();
	show_pend();
	show_stat();
}

static int pl_remove_buf(pl, buf)
		 rtr_proc_t   *pl;
		 int          buf;
{
	int        i;
	rtr_buf_t  *bl;
	bl = &pl->bl;
	while((bl=bl->next)!=&pl->bl){
		if(bl->shm->buf==buf){
			remove_item(bl);
			free(bl);
			break;
		}
	}
	if(bl==&pl->bl){
		/* never come here */
		errout(ErrWarning, "Could not find buffer in pl->bl.");
		return(-1);
	}
	return 0;
}

static rtr_shmbuf_t *get_shm(size)
		 long  size;
{
	rtr_shmbuf_t *shm, *new;
	size = ((size+3)>>2)<<2;
	shm = shm_free;
	while((shm=shm->next)!=shm_free){
		if(shm->size == size){
			remove_item(shm);
			insert_item(shm_used, shm);
			return shm;
		}else if(shm->size > size){
			new = malloc(sizeof(rtr_shmbuf_t));
			if(new==(rtr_shmbuf_t*)NULL){
				errout(ErrSever, "Could not allocate shmbuf_t\n");
				return((rtr_shmbuf_t*)NULL);
			}
			new->buf   = shm->buf;
			new->size  = size;
			/* 08-NOV-2000 debug for AIX */
      new->count = 0;
			/* 08-NOV-2000 end debug for AIX */
			shm->buf  += size;
			shm->size -= size;
			insert_item(shm_used, new);
			return new;
		}
	}
	return((rtr_shmbuf_t*)NULL);
}

static int shm_free_insert(shm)
		 rtr_shmbuf_t  *shm;
{
	rtr_shmbuf_t  *s, *t;

	s = shm_free;
	while(1){
		s = s->next;
		if(s==shm_free){
			insert_item(s, shm);
			break;
		}
		if(shm->buf+shm->size <= s->buf){
			if(shm->buf+shm->size == s->buf){
				s->buf = shm->buf;
				s->size += shm->size;
				free(shm);
				break;
			}else{
				insert_item(s, shm);
				break;
			}
		}else if(shm->buf < s->buf+s->size){
			errout(ErrSever, "Shared memory buffer conflict;\n");
			errout(ErrSever, "buf=%d, size=%d and buf=%d, size=%d.\n",
						 s->buf, s->size, shm->buf, shm->size);
			free(shm);
			return(-1);
		}else if(shm->buf == s->buf+s->size){
			t = s->next;
			if(t!=shm_free){
				if(t->buf == shm->buf+shm->size){
					s->size += shm->size + t->size;
					remove_item(t);   /* Never do as remove_item(s->next) */
					free(t);
					free(shm);
					break;
				}else if(t->buf < shm->buf+shm->size){
					errout(ErrSever, "Shared memory buffer conflict;\n");
					errout(ErrSever, "buf=%d, size=%d and buf=%d, size=%d.\n",
								 t->buf, t->size, shm->buf, shm->size);
					free(shm);
					return(-1);
				}
			}
			s->size += shm->size;
			free(shm);
			break;
		}
	}
	return(0);
}

static rtr_buf_t *new_bl(shm)
		 rtr_shmbuf_t  *shm;
{
	rtr_buf_t *bl;
	bl = malloc(sizeof(rtr_buf_t));
	if(bl==(rtr_buf_t*)NULL){
		errout(ErrSever, "Could not allocate bl_t\n");
		return((rtr_buf_t*)NULL);
	}
	bl->shm = shm;
	return(bl);
}

static int que_msg(msg)
		 rtr_msg_t   *msg;
{
	rtr_msgbuf_t  *buf;
	buf = (rtr_msgbuf_t*)malloc(sizeof(rtr_msgbuf_t));
	if(buf==(rtr_msgbuf_t*)NULL){
		errout(ErrSever, "Could not allocate que\n");
		return(-1);
	}
	buf->msg = *msg;
	insert_item(que_head, buf);
	que_count++;
	return(0);
}

static int que_snd(){
	rtr_msgbuf_t  *msg;
	int           res;
	if(msg_count-que_count>=num_msg_limit)
		return(1);
	while((msg=que_head->next)!=que_head){
		while(1){
			res = msgsnd(msg_id_out, &msg->msg, sizeof(rtr_msg_t), IPC_NOWAIT);
			if(res==-1){
				if(errno==EINTR)
					continue;
				if(errno==EAGAIN){
					/* Queue is full */
					return(1);
				}
				errout(ErrWarning, "Erro in msgsnd, id=%d.\n", errno);
				return(-1);
			}
			break;
		}
		remove_item(msg);
		que_count--;
#if 1  /* 15-NOV-1997 */
		free(msg);
#endif
	}
	return(0);
}

static int send_msg(mtype, com, arg1, arg2, arg3)
		 int   mtype, com;
		 long  arg1, arg2, arg3;
{
  int        res;
	rtr_msg_t  msg;
	msg.mtype = mtype;
	msg.pid = RTR_MTYPE_ROUTER;
	msg.cmd = com;
	msg.arg[0] = arg1;
	msg.arg[1] = arg2;
	msg.arg[2] = arg3;
	strncpy(msg.name, proc_name, RTR_NAME_LEN);

	que_msg(&msg);
	que_snd();
	return(0);
}

static int check_max_buf(){
	rtr_proc_t  *pl;
	/* Check the reference counter of the 'must' consumers */
	pl = pl_head;
	while((pl=pl->next)!=pl_head){
		if(!(pl->flag&(O_WRONLY|O_SAMPLE)) && pl->cnt>=RTR_MAXSENDBUF)
			return(1);
	}
	return(0);
}
	
static int send_buf(shm, size)
		 rtr_shmbuf_t  *shm;
		 int           size;
{
	rtr_proc_t  *pl;
	rtr_buf_t   *bl;
	int    i, n, res;
	int    cnt;

	if(size>shm->size){
		errout(ErrSever, "Illegal size information (Size=%dKB). Reduce the size.",
					 size>>10);
		size = shm->size;
	}

#if 0   /* 14-JUL-1997 */
	/* check message counts in message queue */
	if(msg_count>=num_msg_limit){
		return(1);
	}
#endif

	/* Check the reference counter of the 'must' consumers */
	pl = pl_head;
	while((pl=pl->next)!=pl_head){
		if(!(pl->flag&(O_WRONLY|O_SAMPLE)) && pl->cnt>=RTR_MAXSENDBUF)
			return(2);
	}
			
	cnt = msg_count;
	pl = pl_head;
	while((pl=pl->next)!=pl_head){
		if(pl->flag&O_WRONLY) continue;
		if((pl->flag&O_SAMPLE) && pl->cnt>=RTR_MAXSAMPBUF){
      /* Give up to send buf (Sampling) */
			continue;
		}
		if(!(pl->flag&O_SAMPLE) && pl->cnt>=RTR_MAXSENDBUF){
			/* Nerver Come Here */
			errout(ErrSever, "Could not send buffer. The buffer may have lost.");
			return(-1);
		}
		
		bl = new_bl(shm);
		if(bl==(rtr_buf_t*)NULL)
			return(-1);

		if(send_msg(pl->pid, RTR_BUFFER, shm->buf, size, 0)){
			errout(ErrSever, "Could not send buffer. The buffer may have lost.");
			return(-1);
		}
		insert_item(&pl->bl, bl);
		pl->cnt++;
		pl->cnt_total++;
		pl->l_time = time((time_t*)NULL);
		shm->count++;
#if 0
		fprintf(stderr, "a) shm = %8x, shm->count = %d\n", shm, shm->count);
#endif
		msg_count++;
	}
#if TRUE
	if(cnt==msg_count)
		write(1,"*",1);    /* The buffer was ignored */
	else{
		write(1,".",1);    /* Sent at least to one consumer */
		/* Statistics */
		status[OutBlocks] ++;
		status[OutBytes] += size;
		n = status[OutBytes]/1024;
		status[OutKBytes] += n;
		status[OutBytes] -= n*1024;
	}
#endif
	return(0);
}
		 
static int release_buf(pid, buf, size)
		 int  pid;
		 long buf;
     long size;
{
	rtr_shmbuf_t *shm;
	rtr_proc_t   *pl;
	int      res;
	int      n;
	
	pl = pl_head;
	while((pl=pl->next)!=pl_head){
		if(pl->pid!=pid)
			continue;
		if(pl_remove_buf(pl, buf)==0){
			if(pl->cnt>0) pl->cnt--;
		}
		break;
	}

	shm = shm_used;
	while((shm=shm->next)!=shm_used){
		if(shm->buf==buf){
			if((pl->flag&O_WRONLY) && size>0){
				/* does not route size=0 buffer */
				/*--- Important: Send buffers before release the reference
					               by the writer itself;
											   i.e. send_buf() and shm->count-- ....       ---*/
				/* Statistics */
				status[InBlocks]++;
				status[InBytes] += size;
				n = status[InBytes]/1024;
				status[InKBytes] += n;
				status[InBytes] -= n*1024;

				if(res=send_buf(shm, size)){
					if(res<0)	return(-1);
					errout(ErrSever,
								 "Pending state occured.");
				}
			}
			shm->count--;
			if(shm->count<=0){
				shm->count = 0;
				remove_item(shm);
				shm_free_insert(shm);
			}
			break;
		}
	}
	
	if(shm==shm_used){
		errout(ErrWarning, "Could not find buffer in shm_used.");
		return(-1);
	}

	msg_count--;
	return 0;
}

int pl_release_buf_all(pid)
		 int      pid;
{
	int        i, buf;
	rtr_proc_t *pl;
  rtr_buf_t  *bl;
	pl = pl_head;
	while((pl=pl->next)!=pl_head){
		if(pl->pid==pid) break;
	}
	if(pl==pl_head){
		errout(ErrWarning, "Cound not find pid in pl.");
		return(-1);
	}

	bl = &pl->bl;
	while((bl=bl->next)!=&pl->bl){
		release_buf(pid, bl->shm->buf, -1);
#if 0
		errout(ErrDebug, "Release buf %d", bl->buf);
#endif
		bl = &pl->bl;  /* Never do as bl=bl->next (bl is already purged) */
	}
	return(0);
}

static rtr_proc_t *new_pl(id, flag, name)
		 int   id, flag;
		 char  *name;
{
	rtr_proc_t *pl;
	pl = (rtr_proc_t*)malloc(sizeof(rtr_proc_t));
	if(pl==(rtr_proc_t*)NULL){
		errout(ErrSever, "Could not allocate pl_t\n");
		return((rtr_proc_t*)NULL);
	}
	pl->pid = id;
	pl->flag = flag;
	pl->cnt_total = 0;
	pl->cnt = 0;
	strncpy(pl->name, name, RTR_NAME_LEN);
	pl->bl.prev  = &pl->bl;
	pl->bl.next  = &pl->bl;
	insert_item(pl_head, pl);
	return(pl);
}

static int remove_pl(pid)
		 int  pid;
{
	rtr_proc_t  *pl;
	pl_release_buf_all(pid);
	pl = pl_head;
	while((pl=pl->next)!=pl_head){
		if(pl->pid!=pid)
			continue;
		remove_item(pl);
		free(pl);
		break;
	}
	if(pl==pl_head){
		errout(ErrWarning, "There is no link to the process %d.", pid);
		return(-1);
	}
	return(0);
}
	
static check_proc(){
	rtr_proc_t  *pl;
	int     i;
	time_t  t;
	
	t = time((time_t*)NULL);

	pl = pl_head;
#if 0
	while((pl=pl->next)!=pl_head){
		if(pl->cnt >= RTR_MAXSENDBUF &&
			   pl->l_time+RTR_TMO < t && !(pl->flag & O_WRONLY)){
			show_pl();
			errout((pl->flag & O_SAMPLE) ? ErrWarning : ErrSever,
						 "Force to remove connection to %s (Timeout).\n", pl->name);
			remove_pl(pl->pid);
			show_pl();
			pl = pl_head;         /* retry from the beginning */
		}
	}
#endif
}

static int get_buf(pid, size, shmp)
		 int   pid;
		 long  size;
		 rtr_shmbuf_t **shmp;
{
	rtr_proc_t   *pl;
	rtr_shmbuf_t *shm;
	rtr_buf_t    *bl;

	pl = pl_head;
	while((pl=pl->next)!=pl_head){
		if(pl->pid == pid) break;
	}
	if(pl==pl_head){
		errout(ErrWarning, "Illegal get buffer request from pid=%d.", pid);
		return(-2);
	}
	if(!(pl->flag & O_WRONLY)){
		errout(ErrWarning, "Illegal get buffer request from pid=%d.", pid);
		return(-1);
	}
	if(pl->cnt >= RTR_MAXWRITEBUF){
		/* pending request*/
		return(2);
	}
	size = ((size+3)>>2)<<2;
	if(size>shmsize){
		errout(ErrWarning, "Requested buffer size is too large. Size=%dKB",
					 size>>10);
		return(-1);
	}

	shm = get_shm(size);
	if(shm==(rtr_shmbuf_t*)NULL)
		return(2);

	shm->count++;
#if 0
	fprintf(stderr, "b) shm = %8x, shm->count = %d\n", shm, shm->count);
#endif
	bl = new_bl(shm);
	if(bl==(rtr_buf_t*)NULL){
		remove_item(shm);
		shm_free_insert(shm);
		return(-1);
	}

	insert_item(&pl->bl, bl);
	pl->cnt++;
	pl->cnt_total++;
	pl->l_time = time((time_t*)NULL);
	msg_count++;

	*shmp = shm;
	return(0);
}

static int message(msg)
		 rtr_msg_t  *msg;
{
	rtr_proc_t *pl;
	int       res;
	int       i;
	rtr_msg_t tmp_msg;
	rtr_shmbuf_t *shm;
	rtr_msgbuf_t *mb, *mbt;
	
	switch(msg->cmd){
	case RTR_CONNECT:
		errout(ErrMessage, "Connection request from '%s'.", msg->name);
		pl = pl_head;
		while((pl=pl->next)!=pl_head){
			if(pl->pid == msg->pid){
				errout(ErrWarning, "Process ID confiliction.");
				errout(ErrSever, "Connection request from %s was ignored.", msg->name);
				break;
			}
		}
		if(pl!=pl_head) break;
		if(msg->arg[0] != O_RDONLY && msg->arg[0]!=O_WRONLY
			 && msg->arg[0]!=O_SAMPLE){
			errout(ErrWarning, "Illegal connection flag %d.", msg->arg[0]);
			send_msg(msg->pid, RTR_REFUSE, 0, 0, 0);
			break;
		}
		if(msg->arg[1] < ROUTER_VERSION_MIN) {
			errout(ErrWarning, "Client Version is too low (%d.%d).",
						 msg->arg[1]>>8, msg->arg[1]&0x0F);
			send_msg(msg->pid, RTR_REFUSE, 0, 0, 0);
			break;
		}
		if(msg->arg[1] > ROUTER_VERSION) {
			errout(ErrWarning, "Client Version is too high (%d.%d).",
						 msg->arg[1]>>8, msg->arg[1]&0x0F);
			send_msg(msg->pid, RTR_REFUSE, 0, 0, 0);
			break;
		}
		pl = new_pl(msg->pid, msg->arg[0], msg->name);
		if(pl==(rtr_proc_t*)NULL){
			errout(ErrSever, "Cound not get new pl.");
			errout(ErrSever, "Connection request from %s was ignored.", msg->name);
			send_msg(msg->pid, RTR_REFUSE, 0, 0, 0);
			break;
		}
		pl->c_time = time((time_t*)NULL);
		pl->l_time = time((time_t*)NULL);
		send_msg(msg->pid, RTR_CONNECT, 0, 0, 0);
		switch(msg->arg[0]){
		case O_RDONLY:
			errout(ErrMessage, "Create Read Connection to '%s'.", msg->name );
			break;
		case O_SAMPLE:
			errout(ErrMessage, "Create Sampling Connection to %s.", msg->name );
			break;
		case O_WRONLY:
			errout(ErrMessage, "Create Write Connection to '%s'.", msg->name );
			break;
		}
		break;
	case RTR_DISCONNECT:
		errout(ErrMessage, "Disconnection request from '%s'.", msg->name);
		/* remove messages which send to the process */
		for(mb=que_head->next; mb!=que_head; mb=mb->next){
			if(mb->msg.mtype==msg->pid){
				release_buf(msg->pid, mb->msg.arg[0], -1);
				mbt = mb;
				mb = mb->prev;
				remove_item(mbt);
#if 1  /* 15-NOV-1997 */
				free(mbt);
#endif
			}
		}

    /* 20-DEC-1999 corrected from msg_id_in to msg_id_out*/
		while(TRUE){
#ifndef OSF1
			res = msgrcv(msg_id_out, &tmp_msg, sizeof(rtr_msg_t),
									 msg->pid, MSG_NOERROR|IPC_NOWAIT);
#else
			res = msgrcv(msg_id_out, &tmp_msg, sizeof(rtr_msg_t)-16,
									 msg->pid, MSG_NOERROR|IPC_NOWAIT);
#endif
			if(res==-1) break;
			if(tmp_msg.cmd==RTR_BUFFER){
				release_buf(msg->pid, tmp_msg.arg[0], -1);
			}
		}
		
		while(TRUE){
#ifndef OSF1
			res = msgrcv(msg_id_in, &tmp_msg, sizeof(rtr_msg_t),
									 msg->pid, MSG_NOERROR|IPC_NOWAIT);
#else
			res = msgrcv(msg_id_in, &tmp_msg, sizeof(rtr_msg_t)-16,
									 msg->pid, MSG_NOERROR|IPC_NOWAIT);
#endif
			if(res==-1) break;
			if(tmp_msg.cmd==RTR_BUFFER){
				release_buf(msg->pid, tmp_msg.arg[0], -1);
			}
		}

		if(remove_pl(msg->pid)==0)
			errout(ErrMessage, "Remove Connection to '%s'.", msg->name);
		break;
	case RTR_RELEASE:
		pl = pl_head;
		while((pl=pl->next)!=pl_head)
			if(pl->pid==msg->pid) break;
		if(pl==pl_head)
			errout(ErrWarning, "No connection to the process `%s`(%d).",
						 msg->name, msg->pid);
		if(pl->flag&O_WRONLY){
			if(check_max_buf())
				return(1);
		}
		if(release_buf(msg->pid, msg->arg[0], msg->arg[1]))
			errout(ErrWarning, "Illegal release buffer request from `%s`.",
								msg->name);
		break;
	case RTR_GETBUF:
		switch(get_buf(msg->pid, msg->arg[0], &shm)){
		case -1:  /* Error */
			send_msg(msg->pid, RTR_REFUSE, 0, 0, 0);
			break;
		case 2:   /* Pending */
			return(1);
		case 0:
			send_msg(msg->pid, RTR_BUFFER, shm->buf, shm->size, 0);
			break;
		case -2:   /* Error (Not Reply) */
		default:
			break;
		}
		break;
	case RTR_STATUS:
		show_status();
		break;
	case RTR_TERMINATE:
		errout(ErrMessage, "Router termination request from '%s'.", msg->name);
		end_flag = 1;
		return(-1);
		break;
	default:
		errout(ErrWarning, "Unkown message from '%s'.", msg->name);
		break;
	}
	return(0);
}

rtr_msg_t  msg2;
int do_listen()
{
	rtr_msg_t     msg;
	int           res;
	rtr_msgbuf_t  *mb, *new;

#if 0
	fprintf(stderr, "(%d,%d)", msg_count, msg_count-que_count);
	fprintf(stderr, "\nmsg_count = %d\n", msg_count);
	show_buf();
	show_pend();
#endif

	que_snd();

	/* wait for the message from consumers */
	mb = mb_head->next;
	while(1){
		check_proc();
		if(mb!=mb_head){         /* do pending message */
			msg = mb->msg;
			res = message(&msg);   /* do the message */
			switch(res){
			case 0:                /* Normal Result */
				remove_item(mb);
				free(mb);
				return(0);
			case 1:                /* Pending */
				mb = mb->next;
				break;
			default:               /* End of Building */			
				return(1);
			}
			continue;
		}else{                   /* get and do message */
#if 0
			signal(SIGALRM, nullfcn);
			alarm(RTR_ALARM_TIME);
#endif
			while(1){
#ifndef OSF1
				res = msgrcv(msg_id_in, &msg, sizeof(rtr_msg_t), 1, MSG_NOERROR);
#else				
#if 1
				res = msgrcv(msg_id_in, &msg, sizeof(rtr_msg_t)-16, 1,
										 IPC_NOWAIT|MSG_NOERROR);
#else
				res = msgrcv(msg_id_in, &msg, sizeof(rtr_msg_t)-16, 1,
										 MSG_NOERROR);
#endif
#endif
				/* I don't know why -16 is needed. I'm suspecting msgrcv function */
        /* If -16 is not used, memory violation occures in some conditions */
				if(res==-1){
					if(errno==EINTR)
						continue;
					if(errno==ENOMSG){
#ifdef SunOS
#else	
						usleep(1000);
#endif
						que_snd();
						continue;
					}
					if(errno==EIDRM||errno==EINVAL) /* message queue removed */
						return(-1);
					errout(ErrSever, "Error in msgrcv(), id=%d.", errno);
					return(-1);
				}
				break;
			}
#if 0
			alarm(0);
#endif
			if(res==-1){
				if(errno==EINTR)
					return(0);
				if(errno==EIDRM||errno==EINVAL) /* message queue removed */
					return(-1);
				errout(ErrSever, "Error in msgrcv(), id=%d.", errno);
				return(-1);
			}
			msg.name[RTR_NAME_LEN] = 0x00;

			res = message(&msg);   /* do the message */
			switch(res){
			case 0:                /* Normal Result */
				return(0);
			case 1:                /* Pending */
				new = (rtr_msgbuf_t*)malloc(sizeof(rtr_msgbuf_t));
				if(new==(rtr_msgbuf_t*)NULL){
					errout(ErrSever, "Could not get msg_buf.");
					break;
				}
				new->msg = msg;
				insert_item(mb_head, new);
				break;
			default:               /* End of Building */			
				return(1);
			}
			continue;
		}
	}

	return(0);
}

void listen(){
	mb_head = &mb_head_d;
	mb_head->prev = mb_head;
	mb_head->next = mb_head;
	que_head = &que_head_d;
	que_head->prev = que_head;
	que_head->next = que_head;
	que_count = 0;
	
	end_flag = 0;
	while(!end_flag){
#if 0
		show_buf();
#endif
		if(do_listen(0))
			break;
	}
}
