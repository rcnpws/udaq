/* camlib.c ---- camac function library for K2917-K3922                    */
/*								           */
/*  Version 1.00        2008-08-11      by M.Uchida (For Linux2.6)         */
/*  Version 1.10        2008-11-23      by A. Tamii (cleaned-up)           */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "camlib.h"

/* CAMAC NAF generation macro */
#define NAF(n, a, f)    (((n) << 9) + ((a) << 5) + (f))

vme_bus_handle_t bus_handle;
vme_master_handle_t window_handle;
char *phys_addr;

struct K_REG *camOpen(void)
{
  struct K_REG *k2917;

  /*  Initialize K2917 */

  if(vme_init(&bus_handle)){
    perror("Error initializing the VMEbus");
    return NULL;
  }

  if (vme_master_window_create(bus_handle, &window_handle,
                               K2917_BASE, K2917_MODE, K2917_SIZE,
			       /*                               VME_CTL_PWEN, NULL)) {*/
			       0, NULL)) {
    perror("Error creating the window");
    vme_term(bus_handle);
    return NULL;
  }

  phys_addr = (char *) vme_master_window_phys_addr(bus_handle, window_handle);

  k2917 = (struct K_REG *) vme_master_window_map(bus_handle, window_handle, 0);
  if(!k2917){
    perror("Error mapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    return NULL;
  }
  return k2917;
}

int camClose(struct K_REG *k2917){

  if(vme_master_window_unmap(bus_handle, window_handle)){
    perror("Error unmapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    return -1;
  }

  if(vme_master_window_release(bus_handle, window_handle)){
    perror("Error releasing the window");
    vme_term(bus_handle);
    return -1;
  }

  if(vme_term(bus_handle)){
    perror("Error terminating");
    return -1;
  }
  return 0;
}

int CNAF(struct K_REG *k2917, 
           unsigned long C, 
           unsigned long N, 
           unsigned long A,
	   unsigned long F,  
           unsigned long* dat,
	   unsigned long* Q, unsigned long* X)
{
  int status;
  int cur_crate;
  unsigned short mode;
  unsigned short naf;
  int counter = 0;
  unsigned short camac_qx;
  
  status = 0;
  cur_crate = (int) C;
  mode = CC_BIT24;
  naf = NAF(N, A, F);

  k2917->lamc = (u_short)CC_INT_AUTO_CLEAR;
  k2917->donc = (u_short)CC_INT_AUTO_CLEAR;
  k2917->empc = (u_short)CC_INT_AUTO_CLEAR;
  k2917->aboc = (u_short)CC_INT_AUTO_CLEAR;

  k2917->cma = CC_CMA_INIT;                 /* Initialize memory pointer */
  k2917->cmr = mode | (cur_crate << 8);     /* Write command list */
  k2917->cmr = naf;
  k2917->cmr = CC_HALT;
  k2917->cma = CC_CMA_INIT;                 /* Reset memory pointer */

  switch(naf & 0x0018){
  case 0x0000:                              /* CAMAC read */
    k2917->csr &= (unsigned short)~CC_WRITE;
    k2917->csr |= (unsigned short) CC_GO;   /* Go! */
    while ((k2917->csr & ((unsigned short)CC_RDY|(unsigned short)CC_ERR)) == 0
	   && counter < CC_TIMEOUT_SINGLE){
      counter++;
    }

    if ((k2917->csr & (unsigned short)CC_RDY) != 0) {
      if ((mode & (unsigned short)CC_BIT16) == 0) {
	*dat = k2917->dhr & 0x00FF;
	*(dat + 1) = k2917->dlr;
      } else {
	*dat = k2917->dlr;
      }
    }
    break;

  case 0x0010:                              /* CAMAC write */
    k2917->csr |= (unsigned short)CC_WRITE;
    k2917->csr |= (unsigned short)CC_GO;    /* Go! */	    


    while ((k2917->csr & ((unsigned short)CC_RDY|(unsigned short)CC_ERR)) == 0
	   && (unsigned short)counter < (unsigned short)CC_TIMEOUT_SINGLE){
      (unsigned short)counter++;
    }

    if ((k2917->csr & (unsigned short)CC_RDY) != 0) {
      if ((mode & (unsigned short)CC_BIT16) == 0) {
	k2917->dhr = *dat;
	k2917->dlr = *(dat + 1);
      }
      else {
	k2917->dlr = *dat;
      }
    }
    break;

  default:                            /* NDT */
    k2917->csr |= (unsigned short)CC_GO;                     /* Go! */
    break;
  }

  while ((k2917->csr & ((unsigned short)CC_DONE | (unsigned short)CC_ERR)) == 0
	 && (unsigned short)counter < (unsigned short)CC_TIMEOUT_SINGLE){
    (unsigned short)counter++;
  }

  camac_qx = k2917->csr;
  *X =   ((camac_qx & CC_NOX) == 0) ? 1 : 0 ;
  *Q =   ((camac_qx & CC_NOQ) == 0) ? 1 : 0 ;

  if ((unsigned short)counter >= (unsigned short)CC_TIMEOUT_SINGLE) {
    status = (unsigned short)CC_STA_SINGLE_TIMEOUT;
  }

  return status;
}

int cSetClear(struct K_REG *k2917, unsigned long C)
{
  unsigned long dat;
  unsigned long Q,X;
  
  dat = 0;
  return CNAF(k2917, C, 30, 0, 17, &dat, &Q, &X);
}
