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
    exit(1);
  }

  if (vme_master_window_create(bus_handle, &window_handle,
                               K2917_BASE, K2917_MODE, K2917_SIZE,
                               VME_CTL_PWEN, NULL)) {
    perror("Error creating the window");
    vme_term(bus_handle);
    exit(1);
  }

  phys_addr = (char *) vme_master_window_phys_addr(bus_handle, window_handle);

  mem = (unsigned short *) vme_master_window_map(bus_handle, window_handle, 0);
  if(!mem){
    perror("Error mapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    exit(1);
  }
  return mem;
}

void camClose(unsigned short * mem){

  if(vme_master_window_unmap(bus_handle, window_handle)){
    perror("Error unmapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    exit(1);
  }

  if(vme_master_window_release(bus_handle, window_handle)){
    perror("Error releasing the window");
    vme_term(bus_handle);
    exit(1);
  }

  if(vme_term(bus_handle)){
    perror("Error terminating");
    exit(1);
  }

  //  free(mem);
}

void camac(unsigned short *mem, 
           unsigned int N, 
           unsigned int F, 
           unsigned int A,
	   unsigned int C,  
           unsigned int* dat,
	   unsigned int* Q, unsigned int* X)
{
  unsigned short CSR;
  unsigned int data = 0;
  int i;
  unsigned int flag;
	   
  mem[K2917_CSR] &= ~K2917_CSR_GO;
  mem[K2917_CMA]  = 0;
  mem[K2917_CMR]  = (C<<8) + K2917_CMR_WS24;
  mem[K2917_CMR]  = (N<<9) + (A<<5) + F;
  mem[K2917_CMR]  = K2917_CMR_HALT;
  mem[K2917_CMA]  = 0;
  mem[K2917_CMA]  = 0;
  CSR = mem[K2917_CSR];
  switch (F & 0x18) {
  case 0x00: /* Read */
    CSR &= ~K2917_CSR_DIR; /* set Direction CAMAC => VME */
    mem[K2917_CSR] = CSR | K2917_CSR_GO;
    for (i = 0; i < MAX_RETRY; i++) {
      CSR = mem[K2917_CSR];
      if (CSR & K2917_CSR_RDY) break;
      if (CSR & K2917_CSR_ERR){
        exit(1);
      }
      usleep(RETRY_WAIT);
    } if (i == MAX_RETRY) {
        exit(1);
    }
    data = mem[K2917_DHR] & 0x00ff;
    data = mem[K2917_DLR] + (data<<16);
    *dat = data;
    //    printf("%d\n",*dat);
    mem[K2917_CSR] &= ~K2917_CSR_GO; /* <= This is required ! */
    for (i = 0; i < MAX_RETRY; i++) {
      if (mem[K2917_CSR] & K2917_CSR_DONE) break;
      usleep(RETRY_WAIT);
    } if (i == MAX_RETRY) {
        exit(1);;
    }
    fprintf(stdout,"0x%x(Hex) %d(Dec)\n", data,data);
    break;
  case 0x10: /* Write */
    CSR |=  K2917_CSR_DIR; /* set Direction VME => CAMAC */
    mem[K2917_CSR]  = CSR | K2917_CSR_GO;
    for (i = 0; i < MAX_RETRY; i++) {
      if (mem[K2917_CSR] & K2917_CSR_RDY) break;
      usleep(RETRY_WAIT);
      }
    if (i == MAX_RETRY) {
      fprintf(stderr,"MAX_RETRY1\n");	
      exit(1);
    }
    //    usleep(1000L);
    //    printf("%d\n",*dat);
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
        exit(1);
      }
      usleep(RETRY_WAIT);
    }
    if (i == MAX_RETRY) {
      fprintf(stderr,"MAX_RETRY2\n");
      exit(1);;
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
	exit(1);
      }
	
      usleep(RETRY_WAIT);
    }
    if (i == MAX_RETRY) {
      fprintf(stderr,"MAX_RETRY3\n");
      exit(1);
    }
    break;
  }

  flag = 0;
  if (CSR & K2917_CSR_NOX) flag += 1;
  if (CSR & K2917_CSR_NOQ) flag += 2;

  if(flag == 0){
    *Q = 1;
    *X = 1;
  } else if (flag == 1){
    *Q = 1;
    *X = 0;
  } else if (flag == 2){
    *Q = 0;
    *X = 1;
  } else if (flag == 3){
    *Q = 0;
    *X = 0;
  } else {
  }
  
}

void cSetClear(unsigned short *mem, unsigned int C)
{
  unsigned int* dat;
  unsigned int Q,X;
  
  *dat = 0;
  camac(mem, 30,  17, 0, C, dat,&Q,&X);
}


int camac2(unsigned int N,  unsigned int F, unsigned int A,
	    unsigned int C, unsigned int* dat,
	    unsigned int* Q, unsigned int* X)
	    
{
  int i;
  vme_bus_handle_t bus_handle1;
  vme_master_handle_t window_handle1;
  char *phys_addr1;
  unsigned int  flag;
  unsigned int  data=0;
  unsigned short *mem, CSR;
  
  /*  Initialize VME */

  if(vme_init(&bus_handle1)){
    perror("Error initializing the VMEbus");
    exit(1);
  }

  if (vme_master_window_create(bus_handle1, &window_handle1,
                               K2917_BASE, K2917_MODE, K2917_SIZE,
                               VME_CTL_PWEN, NULL)) {
    perror("Error creating the window");
    vme_term(bus_handle);
    exit(1);
  }

  phys_addr1 = (char *)
    vme_master_window_phys_addr(bus_handle1, window_handle1);

  mem = (unsigned short *)
    vme_master_window_map(bus_handle1, window_handle1, 0);
  if(!mem){
    perror("Error mapping the window");
    vme_master_window_release(bus_handle1, window_handle1);
    vme_term(bus_handle);
    exit(1);
  }

  mem[K2917_CSR] &= ~K2917_CSR_GO;
  mem[K2917_CMA]  = 0;
  mem[K2917_CMR]  = (C<<8) + K2917_CMR_WS24;
  mem[K2917_CMR]  = (N<<9) + (A<<5) + F;
  mem[K2917_CMR]  = K2917_CMR_HALT;
  mem[K2917_CMA]  = 0;
  mem[K2917_CMA]  = 0;
  CSR = mem[K2917_CSR];
  switch (F & 0x18) {
  case 0x00: /* Read */
    CSR &= ~K2917_CSR_DIR; /* set Direction CAMAC => VME */
    mem[K2917_CSR] = CSR | K2917_CSR_GO;
    for (i = 0; i < MAX_RETRY; i++) {
      CSR = mem[K2917_CSR];
      if (CSR & K2917_CSR_RDY) break;
      if (CSR & K2917_CSR_ERR){
        goto Error;
      }
      usleep(RETRY_WAIT);
    }

    if (i == MAX_RETRY) {
        goto Error;
    }
    data = mem[K2917_DHR] & 0x00ff;
    data = mem[K2917_DLR] + (data<<16);
    *dat = data;
    mem[K2917_CSR] &= ~K2917_CSR_GO; /* <= This is required ! */
    for (i = 0; i < MAX_RETRY; i++) {
      if (mem[K2917_CSR] & K2917_CSR_DONE) break;
      usleep(RETRY_WAIT);
    }

    if (i == MAX_RETRY) {
        goto Error;
    }
    /*    fprintf(stdout,"0x%x(Hex) %d(Dec)\n", data,data); */
    break;

  case 0x10: /* Write */
    CSR |=  K2917_CSR_DIR; /* set Direction VME => CAMAC */
    mem[K2917_CSR]  = CSR | K2917_CSR_GO;
    for (i = 0; i < MAX_RETRY; i++) {
      if (mem[K2917_CSR] & K2917_CSR_RDY) break;
      usleep(RETRY_WAIT);
      } if (i == MAX_RETRY) {
	goto Error;
    }
    data = *dat;
    mem[K2917_DHR]  = (unsigned short)(data >> 16);
    mem[K2917_DLR]  = (unsigned short)(data & 0x0ffff);
    mem[K2917_CSR] &= ~K2917_CSR_GO; /* <= This is required ! */
    for (i = 0; i < MAX_RETRY; i++) {
      CSR = mem[K2917_CSR];
      if (CSR & K2917_CSR_DONE) break;
      if (CSR & K2917_CSR_ERR) {
        goto Error;
      }
      usleep(RETRY_WAIT);
    } if (i == MAX_RETRY) {
        goto Error;
      }
    break;
  default:   /* Others */
    mem[K2917_CSR]  = CSR | K2917_CSR_GO;
    mem[K2917_CSR] &= ~K2917_CSR_GO;
    for (i = 0; i < MAX_RETRY; i++) {
      CSR = mem[K2917_CSR];
      if (CSR & K2917_CSR_DONE) break;
      if (CSR & K2917_CSR_ERR ) goto Error;
      usleep(RETRY_WAIT);
    } if (i == MAX_RETRY) goto Error;
    break;
  }

  free(mem);
  
  if(vme_master_window_unmap(bus_handle1, window_handle1)){
    perror("Error unmapping the window");
    vme_master_window_release(bus_handle1, window_handle1);
    vme_term(bus_handle1);
    exit(1);
  }

  if(vme_master_window_release(bus_handle1, window_handle1)){
    perror("Error releasing the window");
    vme_term(bus_handle1);
    exit(1);
  }

  if(vme_term(bus_handle1)){
    perror("Error terminating");
    exit(1);
  }

  flag = 0;
  if (CSR & K2917_CSR_NOX) flag += 1;
  if (CSR & K2917_CSR_NOQ) flag += 2;

  if(flag == 0){
    *Q = 1;
    *X = 1;
  } else if (flag == 1){
    *Q = 1;
    *X = 0;
  } else if (flag == 2){
    *Q = 0;
    *X = 1;
  } else if (flag == 3){
    *Q = 0;
    *X = 0;
  } else {
  }

  return 0;
  
 Error:
  printf("Enter error\n");
  if(vme_master_window_unmap(bus_handle1, window_handle1)){
    perror("Error unmapping the window");
    vme_master_window_release(bus_handle1, window_handle1);
    vme_term(bus_handle1);

  }

  if(vme_master_window_release(bus_handle1, window_handle1)){
    perror("Error releasing the window");
    vme_term(bus_handle1);
  }

  if(vme_term(bus_handle1)){
    perror("Error terminating");
  }
  
  exit(4);


}

// camacW
// 16 bit write
// 
void camacW(unsigned short *mem, unsigned int N,  unsigned int F,
	   unsigned int A,  unsigned int C, unsigned short* dat,
	   unsigned int* Q, unsigned int* X)
{
  unsigned short CSR;
  unsigned short data = 0;
  int i;
  unsigned int flag;
	   
  mem[K2917_CSR] &= ~K2917_CSR_GO;
  mem[K2917_CMA]  = 0;
  mem[K2917_CMR]  = (C<<8) + K2917_CMR_WS16;
  mem[K2917_CMR]  = (N<<9) + (A<<5) + F;
  mem[K2917_CMR]  = K2917_CMR_HALT;
  mem[K2917_CMA]  = 0;
  mem[K2917_CMA]  = 0;
  CSR = mem[K2917_CSR];
  switch (F & 0x18) {
  case 0x00: /* Read */
    CSR &= ~K2917_CSR_DIR; /* set Direction CAMAC => VME */
    mem[K2917_CSR] = CSR | K2917_CSR_GO;
    for (i = 0; i < MAX_RETRY; i++) {
      CSR = mem[K2917_CSR];
      if (CSR & K2917_CSR_RDY) break;
      if (CSR & K2917_CSR_ERR){
        exit(1);
      }
      usleep(RETRY_WAIT);
    } if (i == MAX_RETRY) {
        exit(1);
    }

    data = mem[K2917_DLR];
    *dat = data;

    mem[K2917_CSR] &= ~K2917_CSR_GO; /* <= This is required ! */
    for (i = 0; i < MAX_RETRY; i++) {
      if (mem[K2917_CSR] & K2917_CSR_DONE) break;
      usleep(RETRY_WAIT);
    } if (i == MAX_RETRY) {
        exit(1);;
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
      fprintf(stderr,"MAX_RETRY1\n");	
      exit(1);
    }

    data = *dat;
    mem[K2917_DLR]  = (unsigned short)(data & 0xffff);

    mem[K2917_CSR] &= ~K2917_CSR_GO; /* <= This is required ! */
    for (i = 0; i < MAX_RETRY; i++) {
      CSR = mem[K2917_CSR];
      if (CSR & K2917_CSR_DONE) {
	break;
      }
      if (CSR & K2917_CSR_ERR) {
	fprintf(stderr,"K2917_CSR_ERR1\n");	
        exit(1);
      }
      usleep(RETRY_WAIT);
    }
    if (i == MAX_RETRY) {
      fprintf(stderr,"MAX_RETRY2\n");
      exit(1);;
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
	exit(1);
      }
	
      usleep(RETRY_WAIT);
    }
    if (i == MAX_RETRY) {
      fprintf(stderr,"MAX_RETRY3\n");
      exit(1);
    }
    break;
  }

  flag = 0;
  if (CSR & K2917_CSR_NOX) flag += 1;
  if (CSR & K2917_CSR_NOQ) flag += 2;

  if(flag == 0){
    *Q = 1;
    *X = 1;
  } else if (flag == 1){
    *Q = 1;
    *X = 0;
  } else if (flag == 2){
    *Q = 0;
    *X = 1;
  } else if (flag == 3){
    *Q = 0;
    *X = 0;
  } else {
  }
  
}
