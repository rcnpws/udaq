/*
 * rpvlib.c
 *   REPIC Interrupt & I/O register (Model RPV-130)
 *   Utility function (API,INIT,....)
 *                          2001/08/10   M.Uchida
 *
 */

#include <vme/vme.h>
#include <vme/vme_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "rpv130.h"

/* static unsigned short rpv_current_level = RPV_OREG_VETO | RPV_OREG_BUF1 | RPV_OREG_BUF2; */
static unsigned short rpv_current_level;


/*************************************************/
/*                                               */
/*  rpv130_map                                   */
/*   Map the RPV130 on VME                       */
/*   Arguments:                                  */
/*   vmedev: file descript for vme device        */
/*  vmeaddr: RPV130 base address                 */
/*                                               */
/*************************************************/

Rpv rpv_map(vme_bus_handle_t bus_handle)
{


  vme_master_handle_t window_handle;

  Rpv rpv;

  if (vme_master_window_create(bus_handle, &window_handle,
                               RPV_ADDR, RPV_MODE, RPV_SIZE,
                               VME_CTL_PWEN, NULL)) {
    perror("Error creating the window");
    vme_term(bus_handle);
    exit(1);
  }

  /*  phys_addr = (char *) vme_master_window_phys_addr(bus_handle, window_handle);
      fprintf(stdout,"VME master window addrress: 0x%lx\n",(unsigned long)phys_addr); */

  rpv = (struct rpv130 *) vme_master_window_map(bus_handle, window_handle, 0);
  if(!rpv){
    perror("Error mapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    exit(1);
  }

  return (struct rpv130 *)rpv;

}

void rpv_dump(Rpv rpv)
{
  printf(" latch1 :0x%x\n",rpv->latch1);
  printf(" latch2 :0x%x\n",rpv->latch2);
  printf(" RS-FF  :0x%x\n",rpv->ff);
  printf(" Through:0x%x\n",rpv->thr);
  printf(" CSR1   :0x%x\n",rpv->csr1);
  printf(" CSR2   :0x%x\n",rpv->csr2);
}
  
void rpv_init(Rpv rpv)
{
  unsigned short dat;
  
  rpv_current_level = RPV_OREG_VETO | RPV_OREG_BUF1 | RPV_OREG_BUF2 ;
  rpv_level(rpv,rpv_current_level);

  /* Clear busy signal and input register */
  /* test */

  /*  rpv_clr(rpv); */
  rpv->csr1=0x3;
  rpv->csr2=0x3;  

  /* Send init signal                     */

  dat = RPV_OREG_INIT;
  rpv_pulse(rpv,dat);

  rpv_level(rpv,rpv_current_level);

}

/* RPV130 VETO off                    */

void rpv_ena(Rpv rpv)
{

  rpv_current_level &= ~RPV_OREG_VETO; 
  rpv->level = rpv_current_level;
}

/* RPV130 VETO on                     */

void rpv_dis(Rpv rpv)
{

  rpv_current_level |= RPV_OREG_VETO; 
  rpv->level = rpv_current_level;
}


/* RPV130 disable buffer              */



void rpv_dis_buf(Rpv rpv,int i_buf)
{
  
  if(i_buf==1){
    rpv_current_level &= ~RPV_OREG_BUF1;
  }else if(i_buf==2){
    rpv_current_level &= ~RPV_OREG_BUF2;
  } else if(i_buf==3){
    rpv_current_level &= (~RPV_OREG_BUF1 & ~RPV_OREG_BUF2);
  } else {
    fprintf(stderr,"Undefined event! i_buf %d\n",i_buf);
    exit(1);
  }
  rpv->level = rpv_current_level; 
  
}


/* RPV130 enable buffer               */


void rpv_ena_buf(Rpv rpv,int i_buf)
{

 if(i_buf==1){
   rpv_current_level |= RPV_OREG_BUF1;
 }else if(i_buf==2){
   rpv_current_level |= RPV_OREG_BUF2;
 }else if(i_buf==3){
   rpv_current_level |= (RPV_OREG_BUF1 | RPV_OREG_BUF2);
 } else {
   fprintf(stderr,"Funny event!\n");
   exit(1);
 }

 rpv->level = rpv_current_level;

}

