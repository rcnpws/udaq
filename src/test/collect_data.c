/*
 *
 * collect_data.c
 *
 *   Main collecter routine
 *
 *
 */

#include <vme/vme.h>
#include <vme/vme_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "rpv130.h"

#define BLK_BUF_SIZE 0x2000


int main(int argc, char **argv)
{
  vme_bus_handle_t bus_handle;
  vme_master_handle_t window_handle;

  unsigned short buf[BLK_BUF_SIZE];   /* Buffer for HSM                 */
  unsigned short sbuf[BLK_BUF_SIZE];  /* Scaler buffer                  */
  unsigned short *buf_ptr;            /* Start buffer pointer           */

  Rpv rpv;                            /* RPV130 module                  */

  int i,j,lrm_word;
  int count=0;
  int i_buf = 1,p_buf=1;
  int blk_cnt = 0;

  unsigned long ldata1,ldata2;
  unsigned long	pending_count = 0;
  int pending_flag = 0;

  int buf_loop=0;
  unsigned short dddat;

#if 1
  printf("enter test rpv130 \n");
#endif

  /* Initialize VME bus */
  
  if(vme_init(&bus_handle)){
    perror("Error initializing the VMEbus");
    return -1;
  }


  /* Initialize all modules  */

  fprintf(stderr,"\n Now mapping RPV130 module \n");

  /* Initialize RPV130              */
  if (vme_master_window_create(bus_handle, &window_handle,
                               RPV_ADDR, RPV_MODE, RPV_SIZE,
                               VME_CTL_PWEN, NULL)) {
    perror("Error creating the window");
    vme_term(bus_handle);
    return -1;
  }

  rpv = (struct rpv130 *) vme_master_window_map(bus_handle, window_handle, 0);

  if(!rpv){
    perror("Error mapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    return -1;
  }

  sleep(1);
#if 0
  printf(" latch1 :0x%x\n",rpv->latch1);
  printf(" latch2 :0x%x\n",rpv->latch2);
  printf(" RS-FF  :0x%x\n",rpv->ff & 0xff);
  printf(" Through:0x%x\n",rpv->thr);
  printf(" CSR1   :0x%x\n",rpv->csr1);
  printf(" CSR2   :0x%x\n",rpv->csr2);
#else
  dddat = rpv->ff & 0xff;
  printf(" RS-FF  :0x%x\n",rpv->ff );
  printf(" RS-FF  :0x%x\n",dddat );
  printf(" RS-FF  :0x%x\n",  rpv->ff & 0xff);
  fprintf(stdout, " RS-FF  :0x%x\n",rpv->ff & 0xff);
#endif
  return 0;
}
