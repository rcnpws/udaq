
/*
 *
 * lr1190.c
 *  Utilities for LeCroy 1190 Dual Port Memory
 *
 *  2003/08/05   M.Uchida
 *  2009.05.04   M. Uchida   VMIVME7807
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>
#include "lr1190.h"

/*************************************************/
/*                                               */
/*  lrm_map                                      */
/*   Map the LRM on VME                          */
/*   Arguments:                                  */
/*      fd: file descript for vmedev             */
/*    addr: LRM base address                     */
/*                                               */
/*************************************************/

Lrm lrm_map(vme_bus_handle_t bus_handle)
{

  vme_master_handle_t window_handle;
  /*  uint8_t *ptr; */

  Lrm lrm;

  if(vme_master_window_create(bus_handle, &window_handle,
                               LRM_ADDR, LRM_MODE, LRM_SIZE,
			       0, NULL)) {
    perror("Error on creating a VME window");
    vme_term(bus_handle);
    exit(1);
  }

  lrm = (Lrm )vme_master_window_map(bus_handle, window_handle, 0);

  if(!lrm){
    perror("Error mapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    exit(1);
  }

  return lrm;
}


void lrm_nmap(vme_bus_handle_t bus_handle, long addr, Lrm lrm[], int num, int step)
{
  vme_master_handle_t window_handle;
  /*  uint8_t *ptr; */


  caddr_t alrm;
  int i;

  if(vme_master_window_create(bus_handle, &window_handle,
                               addr, LRM_MODE, LRM_SIZE*num,
			       0, NULL)) {
    perror("Error on creating a VME window");
    vme_term(bus_handle);
    exit(1);
  }


  alrm = (caddr_t )vme_master_window_map(bus_handle, window_handle, 0);
  printf("alrm 0x%x\n",alrm);
  if(!alrm){
    perror("Error mapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    exit(1);
  }

  for(i=0;i<num;i++){
    lrm[i] = (Lrm) alrm;
    alrm +=  step;
  }

}


/*************************************************/
/*                                               */
/*  lrm_init                                     */
/*   Init address pointer for LeCroy 1190        */
/*   arg: pointer of struct lrm                  */
/*                                               */
/*************************************************/

void lrm_init(Lrm lrm)
{
  int i;

  /* Set memory "zero" */
  for(i=0;i<LRM_LEN;i++){
    lrm->mem[i] = 0;
  }
  /* Set address pointer 0 */
  lrm->mem_ptr = 0;
}


/*************************************************/
/*                                               */
/*  lrm_reset                                    */
/*   Reset address pointer for LeCroy 1190       */
/*   arg: pointer of struct lrm                  */
/*                                               */
/*************************************************/

void lrm_reset(Lrm lrm)
{
  int i;
  unsigned short cur_ptr;
#if 0
  cur_ptr = ((lrm->mem_ptr)<<1);

  /* Set memory "zero" */
  if(cur_ptr > LRM_LEN){

    for(i=LRM_LEN;i>=0;i--){
      lrm->mem[i] = 0;
    }

  } else {
    for(i=cur_ptr;i>=0;i--){
      lrm->mem[i] = 0;
    }
  }
  /* Set address pointer 0 */
  lrm->mem_ptr = 0;
#else
  lrm->mem[0] = 0;
#endif
}


unsigned short lrm_get(Lrm lrm, unsigned short *buf,unsigned long max_size)
{
  
  int i;
  unsigned short word;

  word = lrm->mem_ptr;

  if( max_size > word ){

    for(i=0;i<word;i++){
      buf[i]=lrm->mem[i];
    }
    return word;

  } else {

    for(i=0;i<max_size;i++){
      buf[i]=lrm->mem[i];
    }
    return max_size;

  }

}


unsigned short lrm_get2(Lrm lrm, unsigned short *buf,unsigned long max_size)
{
  
  int i;
  unsigned short word;

#if 0
  word = ((lrm->mem_ptr)<<1);
#else
  word = lrm->mem_ptr;
#endif  
  if( max_size > word ){

    for(i=0;i<word;i++){
      buf[i]=lrm->mem[2*i];
    }
    return word;

  } else {

    for(i=0;i<max_size;i++){
      buf[i]=lrm->mem[2*i];
    }
    return max_size;

  }

}

#if 0

int main()
{
  int fd,i,cnt=0;     
  Lrm lrm;
  unsigned short data;

  if((fd = open(LRM_MODE, O_RDWR)) < 0){
    fprintf(stderr,"Can't open vme\n");
    exit(1);
  }

  lrm = lrm_map(fd,LRM_ADDR);

  /* Check LRM access */
  while(cnt<1000){

    lrm->daq_enable = LRM_VME_ENA;
    
    for(i=0;i<LRM_LEN;i++){
      lrm->mem[i] = i;
    }

    for(i=0;i<LRM_LEN;i++){
      data = lrm->mem[i];
    }
    lrm->daq_enable = LRM_FFP_ENA;
    cnt++;
  }

}
#endif
