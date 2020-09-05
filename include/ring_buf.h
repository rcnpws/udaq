#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#define QSIZE	128                      /* number of pointers in the queue */
typedef struct {
	pthread_mutex_t buf_lock;	 /* lock the structure              */
	int start_idx;			 /* start of valid data             */
	int num_full;			 /* # of full locations             */
	pthread_cond_t notfull;		 /* full -> not full condition      */
	pthread_cond_t notempty;	 /* empty -> notempty condition     */
	unsigned short *shm_hdl[QSIZE];	 /* Circular buffer of pointers     */
	unsigned short *shm_buf;	 /* Circular buffer                 */
} ring_buf_t;


ring_buf_t *new_rb();
void delete_rb(ring_buf_t *cbp);
void put_rb_data(ring_buf_t *cbp, unsigned short *data);
void *get_rb_data(ring_buf_t *cbp, unsigned short *data);

#define RING_BUF_MODE 1                  /* If 1, Ring buf mode on.        */

