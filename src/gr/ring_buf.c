/* BeginSourceFile fifo.c */

#include "ring_buf.h"
#include "blplib.h"

extern int max_que_num;  /* max que number for ring buffer */
static int blk_size = BLK_BUF_SIZE;

/*
 * new_cb() creates and initializes a new circular buffer.
 */

ring_buf_t *
new_rb()
{
	ring_buf_t *cbp;
	int i;


	fprintf(stdout,"blk_size = %x \n",blk_size);
	cbp = (ring_buf_t *) malloc(sizeof (ring_buf_t));
	cbp->shm_buf = (unsigned short *)malloc(sizeof(unsigned short)*max_que_num*blk_size);

	for(i=0;i<max_que_num;i++){
	  cbp->shm_hdl[i] = cbp->shm_buf+(blk_size)*i;
	}


	if (cbp == NULL)
		return (NULL);
	pthread_mutex_init(&cbp->buf_lock, NULL);
	pthread_cond_init(&cbp->notfull, NULL);
	pthread_cond_init(&cbp->notempty, NULL);
	cbp->start_idx = 0;
	cbp->num_full = 0;
	return (cbp);
}

/*
 * delete_cb() frees a circular buffer.
 */

void
delete_rb(ring_buf_t *cbp)
{
	pthread_mutex_destroy(&cbp->buf_lock);
	pthread_cond_destroy(&cbp->notfull);
	pthread_cond_destroy(&cbp->notempty);
	free(cbp->shm_buf);
	free(cbp);
}


/*
 * put_cb_data() puts new data on the queue.
 * If the queue is full, it waits until there is room.
 */

void
put_rb_data(ring_buf_t *cbp, unsigned short *data)
{
  int i;
  
	pthread_mutex_lock(&cbp->buf_lock);
	/* wait while the buffer is full */
	while (cbp->num_full == max_que_num){
		pthread_cond_wait(&cbp->notfull, &cbp->buf_lock);
		printf("full of que \n");
	}
	for(i=0;i<blk_size;i++){
	  cbp->shm_hdl[(cbp->start_idx + cbp->num_full) % max_que_num][i] = data[i];
	}
	
	cbp->num_full += 1;
	/* let a waiting reader know there's data */
	/* exercise: can cond_signal be moved after unlock? see text */
	pthread_cond_signal(&cbp->notempty);
	pthread_mutex_unlock(&cbp->buf_lock);
}

/*
 * get_cb_data() gets the oldest data in the circular buffer.
 * If there is none, it waits until new data appears.
 */

void *
get_rb_data(ring_buf_t *cbp,unsigned short *data)
{
	void *dum;
	int i;
	
	pthread_mutex_lock(&cbp->buf_lock);
	/* wait while there's nothing in the buffer */
	while (cbp->num_full == 0)
		pthread_cond_wait(&cbp->notempty, &cbp->buf_lock);

	for(i=0;i<blk_size;i++){
	  data[i] = cbp->shm_hdl[cbp->start_idx][i];
	}

	cbp->start_idx = (cbp->start_idx + 1) % max_que_num;
	cbp->num_full -= 1;
	/* let a waiting writer know there's room */
	/* exercise: can cond_signal be moved after unlock? see text */
	pthread_cond_signal(&cbp->notfull);
	pthread_mutex_unlock(&cbp->buf_lock);
	return (dum);
}
/* EndSourceFile */

