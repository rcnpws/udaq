/*
 *
 * k2917
 * Sample program to access K2917
 * VMIC Pentium SBC version
 *              2008/08/10 M.Uchida
 *
 */

#include <vme/vme.h>
#include <vme/vme_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "k2917.h"

typedef unsigned long  DWORD;
typedef DWORD* DWORDP;

int main(int argc, char **argv){

  vme_bus_handle_t bus_handle;
  vme_master_handle_t window_handle;

  /*  uint8_t *ptr; */
  unsigned char *phys_addr;
  unsigned short dat;
  unsigned short naf,n,a,f;

  int result = 0;

  K2917 k2917;

  /*  int ii; */

  if(vme_init(&bus_handle)){
    perror("Error initializing the VMEbus");
    return -1;
  }

  if(vme_master_window_create(bus_handle, &window_handle,
                               K2917_ADDR, K2917_MODE, K2917_SIZE,
                               VME_CTL_PWEN, NULL)) {
    perror("Error creating the window");
    vme_term(bus_handle);
    return -1;
  }

  phys_addr = (unsigned char *)vme_master_window_phys_addr(bus_handle, window_handle);
  fprintf(stdout,"VME master window addrress: 0x%x\n",(unsigned int)phys_addr);

  k2917 = (K2917)vme_master_window_map(bus_handle, window_handle, 0);
  if(!k2917){
    perror("Error mapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    return -1;
  }

  dat = k2917->cse;

  dat = 0x0060;
  k2917->cmr = dat;

  naf = 0;
  n = 30<<9;
  a = 0<<5;
  f= 17;
  naf = n|a|f;
  k2917->cmr = naf;
  dat = 0x2;
  k2917->cmr = dat;
  dat = 0x0;
  k2917->cmr = dat;

  naf = 0;
  n = 30<<9;
  a = 0<<5;
  f= 1;
  naf = n|a|f;
  k2917->cmr = naf;


  fprintf(stdout,"cse : 0x%x\n",dat);
  
  if(vme_master_window_unmap(bus_handle, window_handle)){
    perror("Error unmapping the window");
    result = -1;
  }

  if(vme_master_window_release(bus_handle, window_handle)){
    perror("Error releasing the window");
    result = -1;
  }

  if(vme_term(bus_handle)){
    perror("Error terminating");
    result = -1;
  }

  return result;
}

