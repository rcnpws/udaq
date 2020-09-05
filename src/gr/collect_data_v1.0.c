/*
 *
 * collect_data.c
 *
 *   Main collecter routine
 *
 *
 */

#include <stdio.h>
#include "hsm8170.h"
#include "hsmlib.h"
#include "lr1190.h"
#include "rpv130.h"
#include "lr1151.h"
#include "run_state.h"
#include "grlib.h"
#include "ring_buf.h"

#define VERBOSE 0                     /* If 1, verbose mode on          */

#define MAX_LRM_NUM 12                /* Number of LR1190 DPM           */
#define MAX_SCL_NUM 4                 /* Number of CAEN C256(Scaler)    */

#define MAX_DTR_INTERVAL 0xffffff

extern cmd_msgbuf_t mb;               /* Command message que buffer     */
extern ring_buf_t *rb;                /* ring buffer pointer            */
extern int local_flag;                /* Local mode flag                 */

void *collect_data(void *arg)
{
  int vmedev1,vmedev2;                /* File descripter                */
  unsigned long hsm_addr;             /* HSM base address               */
  unsigned long lrm_addr;             /* HSM base address               */
  unsigned short rpv_addr;            /* RPV130 base address            */
  unsigned long  scl_addr;            /* LR1151 base address            */
  unsigned short buf[BLK_BUF_SIZE];   /* Buffer for HSM                 */
  unsigned short sbuf[BLK_BUF_SIZE];  /* Scaler buffer                  */
  unsigned short *buf_ptr;            /* Start buffer pointer           */
  Lrm lrm[MAX_LRM_NUM];               /* HSM modules                    */
  Rpv rpv;                            /* RPV130 module                  */

  int i,j,lrm_word;
  int count=0;
  int i_buf = 1,p_buf=1;
  int blk_cnt = 0;

  unsigned long ldata1,ldata2;
  unsigned long	pending_count = 0;
  int pending_flag = 0;

  int buf_loop=0;

#if 1
  printf("enter collect data \n");
#endif


  /* Open VME device(A32D16)   for LRM      */
  vmedev1 = Open(LRM_MODE,O_RDWR,O_EXCL);

  /* Open VME device(A16D16)  for RPV130    */
  vmedev2 = Open(RPV_MODE,O_RDWR,O_EXCL);

  /* Initialize all modules  */

  fprintf(stdout,"\n Now mapping RPV130 module \n");

  /* Initialize RPV130              */

  rpv_addr = RPV_ADDR;
  rpv = rpv_map(vmedev2,rpv_addr);
  rpv_init(rpv);       

#if VERBOSE
  fprintf(stderr,"\n Load module base_addr=0x%x\n",rpv_addr);
  rpv_dump(rpv);       
#endif

  fprintf(stdout,"\n Now mapping HSM modules \n");

  lrm_nmap(vmedev1, LRM_ADDR, lrm, MAX_LRM_NUM, 0x100000);

  for(i=0;i<MAX_LRM_NUM;i++){

    lrm_reset(lrm[i]);

  }
  fprintf(stdout,"finish \n");


#if 1
  printf("\n DAQ Ready? Press Enter:");
  getchar();
#endif

  fprintf(stdout,"\n ****** Now collector is running ****** \n\n");

  /* Enable HSM             */

  for(i=0;i<MAX_LRM_NUM;i++){
#if 0
    printf("%d-th LRM addr = 0x%x\n",i,lrm[i]->mem_ptr);    
#endif
    lrm_ena(lrm[i]);
  }

  
  for(;;){

    /* Check the status of input register */
    /* 1: Buf1 overflow                   */
    /* 2: Buf2 overflow                   */
    /* 3: Buf1,2 overflow                 */

    pthread_mutex_lock(&run_state->lock);

    switch(run_state->status){
    case RUN_START:
	run_state->status = RUN_ON;
	pthread_mutex_unlock(&run_state->lock);

      	/* Send Run Start block */


	send_run_start(-1,buf,BLK_BUF_SIZE,mb.comment);

	if(local_flag == 0){
	  put_rb_data(rb,buf);
	}

#if 1
	pr_buf(buf,0,32,8);                 /* Sampling output  */
#endif

	bzero(buf,sizeof(unsigned short)*BLK_BUF_SIZE);  /* Set zero */

	fprintf(stdout,"start: reset lrm\n");
	for(i=0;i<MAX_LRM_NUM;i++){

	  /* Reset each LRM module */
#if 0
	  lrm_reset(lrm[i]);
#else
	  lrm[i]->mem[0] = 0;
#endif

	  /* Enable ECL DAQ */
	  lrm_ena(lrm[i]);
	}
	fprintf(stdout,"start: reset lrm end\n");

	fprintf(stdout,"\n ******  DAQ START  ****** \n\n");

#if 0
	Send_err(lsockfd,"DAQ start");
#else
	fprintf(stderr,"DAQ start\n");
#endif

	rpv_init(rpv);        /* INIT     */
	rpv_ena(rpv);         /* VETO off */

	break;
    case RUN_ON:

      pthread_mutex_unlock(&run_state->lock);


      if(HSM_BUF_OVF(i_buf,rpv)){ /* Buffer overflow    */

#if 1
	  fprintf(stdout,"buf ovf\n");
#endif

#if 0
	bzero(buf,sizeof(unsigned short)*BLK_BUF_SIZE);  /* Set zero */
#endif

	rpv_dis(rpv);                           /* VETO ON           */
	rpv_clr(rpv);                           /* Clear RPV130      */
	rpv_dis_buf(rpv,i_buf);                 /* Disable buffer    */

	/* Read CAMAC scaler */	
#if 0
	read_c256(sbuf); 
#endif

	rpv_ena(rpv);                          /* VETO off                           */

	if(local_flag == 0){
	  put_rb_data(rb,sbuf);                /* Write scaler data into ring buffer */
	}

	if(SINGLE_BUF_OVF(i_buf)){             /* Single buffer OVF                  */

#if 0
	  printf("single buf ovf ");
#endif
	  count++;


	  /* Read Buffer */

	  i_buf--;

#if 1
	  pr_buf(sbuf,0,32,8);                 /* Sampling output  */
#endif

	  for(i=i_buf;i<MAX_LRM_NUM;i += 2){
	  
	    if(lrm[i]->mem_ptr > 0){
	      lrm_dis(lrm[i]);                                  /* Disable HSM             */


#if 0
	      bzero(buf,sizeof(unsigned short)*BLK_BUF_SIZE);  /* Set zero                 */
#endif
	      lrm_word = lrm_get2(lrm[i],buf,BLK_BUF_SIZE);        /* Read LRM data            */
	      for( j=lrm_word; j<BLK_BUF_SIZE; j++){               /* Fill zero after LRM data */
		buf[j] = 0;
	      }
	      printf("%d-th mem word: 0x%x, mem_ptr: 0x%x\n",i,lrm_word, lrm[i]->mem_ptr);

#if 1
	      pr_buf(buf,0,32,8);                              /* Display buffer           */
#endif


	      if(local_flag == 0){
		put_rb_data(rb,buf);                          /* Write data into ring buffer */
	      }

	      lrm_reset(lrm[i]);                              /* Reset LRM             */
	      lrm_ena(lrm[i]);                                /* Enable LRM            */

	    } else {
	      lrm_reset(lrm[i]);                              /* Reset LRM             */
	    }

	  }
	  
	  p_buf = i_buf++;                     /* Set current buf index */

	  /* Modified 2003/08/05 by M.Uchida */
	  rpv_ena_buf(rpv,i_buf);              /* Enable buffer         */

	} else if(DOUBLE_BUF_OVF(i_buf)){       /* Buf1,2 OVF           */


#if 1
	  printf("double buf ovf\n");
#else
	  Send_err(lsockfd," double buf ovf!");
#endif


	  for(buf_loop=0; buf_loop < 2; buf_loop++){
	    p_buf = TOGGLE_BUF(p_buf);         /* Toggle buffer         */
	    count++;


	    for(i=p_buf;i<MAX_LRM_NUM;i+=2){
	      if(lrm[i]->mem_ptr > 0){

		lrm_dis(lrm[i]);                 /* Disable HSM      */  

#if 1
		printf("hsm_dis p_buf = %d \n",i);
#endif
		/* Get data        */ 
		lrm_word = lrm_get2(lrm[i],buf,BLK_BUF_SIZE);
		for( j=lrm_word; j<BLK_BUF_SIZE; j++){               /* Fill zero after LRM data */
		  buf[j] = 0;
		}
		printf("%d-th mem word: 0x%x, mem_ptr: 0x%x\n",i,lrm_word, lrm[i]->mem_ptr);


#if 1
		pr_buf(buf,0,32,8);                 /* Sampling output  */
#endif

		if(local_flag == 0){
		  put_rb_data(rb,buf);            /* Write data into ring buffer */
		}

		lrm_reset(lrm[i]);               /* Initialize LRM   */
		lrm_ena(lrm[i]);                 /* Enable LRM       */

		printf("buf status: %d count=0x%x hsm_word=%d\n",i,count,lrm_word);

	      }
	    }

	    rpv_ena_buf(rpv,p_buf+1);          /* Enable buffer    */

	  }

	} else {
	  fprintf(stderr,"collect_data: Unexpected Index\n");
	}

	pending_count = 0;
	if(pending_flag == 1){
#if 0
	  Send_err(lsockfd," exit pending state!");
#else
	  fprintf(stderr,"Exit pending state.\n");
#endif
	  pending_flag = 0;
	}

      } else {
#if 0
	getchar();
#else
	if(pending_count == MAX_DTR_INTERVAL){
	  if(pending_flag == 0){
#if 0
	    Send_err(lsockfd,"pending buffer change!");
#else
	    fprintf(stderr,"pending buffer change\n");
#endif
	    pending_flag = 1;
	  }
	  pending_count = 0;
	} else {
	  pending_count += 1;
	}
#endif
      }

      break;

    case RUN_STOP:
      run_state->status = RUN_OFF;
      pthread_mutex_unlock(&run_state->lock);

      rpv_dis(rpv);                          /* VETO on         */

      count++;

#if 0
      read_c256(sbuf); /* Read CAMAC scaler */
#endif

      if(local_flag == 0){
	put_rb_data(rb,sbuf);            /* Write data into ring buffer */
      }

#if 0
      p_buf = TOGGLE_BUF(p_buf);         /* Toggle buffer         */
      printf("hsm_dis p_buf = %d \n",p_buf);

      for(i=p_buf;i<MAX_LRM_NUM;i+=2){
	if(lrm[i]->mem_ptr > 0){
#if 0
	  printf("disable mem\n ");
#endif
	  lrm_dis(lrm[i]);                /* Disable LRM      */  
#if 0
	  printf("get data\n ");
#endif
	  lrm_word = lrm_get2(lrm[i],buf,BLK_BUF_SIZE);

	  for( j=lrm_word; j<BLK_BUF_SIZE; j++){               /* Fill zero after LRM data */
	    buf[j] = 0;
	  }
	  printf("%d-th mem word: 0x%x, mem_ptr: 0x%x\n",i,lrm_word, lrm[i]->mem_ptr);

	  lrm_reset(lrm[i]);          /* Reset LRM        */
	  lrm_ena(lrm[i]);            /* Enable LRM       */

	  if(local_flag == 0){
	    put_rb_data(rb,buf);            /* Write data into ring buffer */
	  }
	  bzero(buf,sizeof(unsigned short)*BLK_BUF_SIZE);  /* Set zero */
	}

      }
#endif

      sleep(1);
	  
      /* Send Run End block */

      send_run_end(-1,buf,BLK_BUF_SIZE);
      if(local_flag == 0){
	put_rb_data(rb,buf);
      }

      fprintf(stdout,"\n ******  DAQ STOP  ****** \n");

#if 0
      Send_err(lsockfd,"DAQ stop");
#else
      fprintf(stderr,"DAQ stop.\n");
#endif

      count   = 0;                      
      blk_cnt = 0;
      i_buf   = 1;
      p_buf   = 1;
      pending_count = 0;

      break;
    case RUN_OFF:
      pthread_mutex_unlock(&run_state->lock);
      usleep(1000L);
      break;
    default:
      fprintf(stderr,"collect_data: Undefined status!\n");
      break;
    }


  }

  return(NULL);
  
}
