/*
 *
 *  nfa.c
 *  Simple nfa function for K2917-K3922
 *  2008.08.11   M. Uchida
 *
 */


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

#define MAX_RETRY   50   
#define RETRY_WAIT  5L

int main(int argc, char *argv[])
{
  unsigned int  C, N, A, F, Q, X;
  unsigned int  data=0;
  unsigned int  flag;
  int i;
  unsigned short *mem, CSR;
  vme_bus_handle_t bus_handle;
  vme_master_handle_t window_handle;
  /*  uint8_t *ptr; */
  char *phys_addr;
  char c;


  if (argc < 5) {printf("Usage: %s n f a [data] c\n", argv[0]); exit(4);}

  if(argc == 5){
    if ((C = atoi(argv[4])) < 0 || C >  7 ||
	(N = atoi(argv[1])) < 1 ||(N > 24 && N != 30)||
	(A = atoi(argv[3])) < 0 || A > 15 ||
	(F = atoi(argv[2])) < 0 || F > 31) exit(4);
  } else if(argc == 6){
    if ((C = atoi(argv[5])) < 0 || C >  7 ||
	(N = atoi(argv[1])) < 1 ||(N > 24 && N != 30)||
	(A = atoi(argv[3])) < 0 || A > 15 ||
	(F = atoi(argv[2])) < 0 || F > 31) exit(4);
        c = argv[4][0];
	c = tolower(c);

	switch(c){
	case '0':
	  if(argv[4][1] == 'x'){ /* Hex input   */
	    sscanf(argv[4],"0x%x",&data);
	  } else {               /* Octal input */
	    sscanf(argv[4],"0%o",&data);
	  }

	  break;
	case 'h': /* Hex input   */
	  sscanf(++argv[4],"%x",&data);
	  break;
	case 'b': /* Binary input */
	  data = 0;

	  while( (c = *(++argv[4])) != 0){
	    if( c == '1'){
	      data = (data<<1 | 1);
	    } else if(c == '0'){
	      data = (data<<1);
	    } else {
	      printf("Usage: %s n f a [data] c\n", argv[0]); exit(4);
	    }
	  }
	  break;
	default: /* Decimal input */
	  if(sscanf(argv[4],"%d",&data) != 1){
	    printf("Usage: %s n f a [data] c\n", argv[0]); exit(4);
	  }

	}

  } else {
    printf("Usage: %s n f a [data] c\n", argv[0]); exit(4);
  }

  /*  int ii; */

  if(vme_init(&bus_handle)){
    perror("Error initializing the VMEbus");
    return -1;
  }

  if (vme_master_window_create(bus_handle, &window_handle,
                               K2917_BASE, K2917_MODE, K2917_SIZE,
                               VME_CTL_PWEN, NULL)) {
    perror("Error creating the window");
    vme_term(bus_handle);
    return -1;
  }

  phys_addr = (char *) vme_master_window_phys_addr(bus_handle, window_handle);

  mem = (unsigned short *) vme_master_window_map(bus_handle, window_handle, 0);
  if(!mem){
    perror("Error mapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    return -1;
  }
  


  //  mem[K2917_CSR] = 0x1000;

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

  flag = 0;
  if (CSR & K2917_CSR_NOX) flag += 1;
  if (CSR & K2917_CSR_NOQ) flag += 2;

  if(flag == 0){
    Q = 1;
    X = 1;
  } else if (flag == 1){
    Q = 1;
    X = 0;
  } else if (flag == 2){
    Q = 0;
    X = 1;
  } else if (flag == 3){
    Q = 0;
    X = 0;
  } else {
  }

  fprintf(stdout,"  N=%d F=%d A=%d Q=%d X=%d  Data:0x%x(Hex) %d(Dec)\n",
         N, F, A, Q, X, data&0xffff, data&0xffff);
  
  return 0;

  fprintf(stderr,"Error occured in NFA\n");
  free(mem);
  
 Error:

  fprintf(stdout,"error\n");
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
  
  exit(4);

}
