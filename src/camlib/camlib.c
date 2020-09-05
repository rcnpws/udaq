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

#include "k2917.h"
#include "camlib.h"

#define MAX_RETRY   10
#define RETRY_WAIT  5L

vme_bus_handle_t bus_handle;
vme_master_handle_t window_handle;
char *phys_addr;

unsigned short *camOpen(void)
{
  unsigned short *mem;

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

  mem = (unsigned short *) vme_master_window_map(bus_handle, window_handle, 0);
  if(!mem){
    perror("Error mapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    return NULL;
  }
  return mem;
}

int camClose(unsigned short * mem){

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

  //  free(mem);

  return 0;
}

int CNAF(unsigned short *mem, 
           unsigned long C, 
           unsigned long N, 
           unsigned long A,
	   unsigned long F,  
           unsigned long* dat,
	   unsigned long* Q, unsigned long* X)
{
  unsigned short CSR;
  unsigned long data = 0;
  int i;
	   
  mem[K2917_CSR] &= ~K2917_CSR_GO;
  mem[K2917_CMA]  = 0;
  mem[K2917_CMR]  = (C<<8) + K2917_CMR_WS24;
  mem[K2917_CMR]  = (N<<9) + (A<<5) + F;
  mem[K2917_CMR]  = K2917_CMR_HALT;
  mem[K2917_CMA]  = 0;
  mem[K2917_CMA]  = 0;
  CSR = mem[K2917_CSR];

  *X = *Q = 0;  // default value

  switch (F & 0x18) {
  case 0x00: /* Read */
    CSR &= ~K2917_CSR_DIR; /* set Direction CAMAC => VME */
    mem[K2917_CSR] = CSR | K2917_CSR_GO;
    for (i = 0; i < MAX_RETRY; i++) {
      CSR = mem[K2917_CSR];
      if (CSR & K2917_CSR_RDY) break;
      if (CSR & K2917_CSR_ERR){
	fprintf(stderr, "K2917_CSR_ERR\n");
        return -1;
      }
      usleep(RETRY_WAIT);
    } if (i == MAX_RETRY) {
	fprintf(stderr, "MAX RETRY 1\n");
        return -1;
    }
    data = mem[K2917_DHR] & 0x00ff;
    data = mem[K2917_DLR] + (data<<16);
    *dat = data;
    mem[K2917_CSR] &= ~K2917_CSR_GO; /* <= This is required ! */
    for (i = 0; i < MAX_RETRY; i++) {
      if (mem[K2917_CSR] & K2917_CSR_DONE) break;
      usleep(RETRY_WAIT);
    } if (i == MAX_RETRY) {
      // ---- Ignore this error! -----
      //fprintf(stderr, "MAX RETRY 2\n");
      //return -1;
    }
    break;
  case 0x10: /* Write */
    CSR |=  K2917_CSR_DIR; /* set Direction VME => CAMAC */
    mem[K2917_CSR]  = CSR | K2917_CSR_GO;
    for (i = 0; i < MAX_RETRY; i++) {
      if (mem[K2917_CSR] & K2917_CSR_RDY) break;
      usleep(RETRY_WAIT);
      }
    if (i == MAX_RETRY) {
      fprintf(stderr,"MAX_RETRY 3\n");	
      return -1;
    }
    data = *dat;
    mem[K2917_DHR]  = (unsigned short)(data >> 16);
    mem[K2917_DLR]  = (unsigned short)(data & 0x0ffff);
    mem[K2917_CSR] &= ~K2917_CSR_GO; /* <= This is required ! */
    for (i = 0; i < MAX_RETRY; i++) {
      CSR = mem[K2917_CSR];
      if (CSR & K2917_CSR_DONE) {
	break;
      }
      if (CSR & K2917_CSR_ERR) {
	fprintf(stderr,"K2917_CSR_ERR1\n");	
        return -1;
      }
      usleep(RETRY_WAIT);
    }
    if (i == MAX_RETRY) {
      fprintf(stderr,"MAX_RETRY 4\n");
      return -1;
    }
    break;
  default:   /* Others */
    mem[K2917_CSR]  = CSR | K2917_CSR_GO;
    mem[K2917_CSR] &= ~K2917_CSR_GO;
    for (i = 0; i < MAX_RETRY; i++) {
      CSR = mem[K2917_CSR];
      if (CSR & K2917_CSR_DONE) {
	break;
      }
      if (CSR & K2917_CSR_ERR ) {
	
	fprintf(stderr,"K2917_CSR_ERR2\n");	
        return -1;
      }
	
      usleep(RETRY_WAIT);
    }
    if (i == MAX_RETRY) {
      fprintf(stderr,"MAX_RETRY 5\n");
      return -1;
    }
    break;
  }

  *X = (CSR & K2917_CSR_NOX) ? 0 : 1;
  *Q = (CSR & K2917_CSR_NOQ) ? 0 : 1;

  return 0;
}

int cSetClear(unsigned short *mem, unsigned long C)
{
  unsigned long dat;
  unsigned long Q,X;
  
  dat = 0;
  return CNAF(mem, C, 30, 0, 17, &dat, &Q, &X);
}
