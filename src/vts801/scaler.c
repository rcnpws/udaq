/*
 * scaler.c ... CAEN v260 100MHz scaler module access program
 * for GE Fanuc VMIVME-7807RC
 * Version 0.01 on 2009.03.18 by A. Tamii
 *
 */

#include <vme/vme.h>
#include <vme/vme_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vts801.h"

int main(int argc, char **argv){

  vme_bus_handle_t bus_handle;
  vme_master_handle_t window_handle;

  /*  uint8_t *ptr; */
  unsigned char *phys_addr;
  Vts801 scaler;

  int i,j;
  int result = 0;

  if(vme_init(&bus_handle)){
    perror("Error on initializing the VMEbus");
    return -1;
  }

  if(vme_master_window_create(bus_handle, &window_handle,
                               VTS801_ADDR, VTS801_MODE, VTS801_SIZE,
			       0, NULL)) {
    perror("Error on creating a VME window");
    vme_term(bus_handle);
    return -1;
  }

  phys_addr = (unsigned char *)vme_master_window_phys_addr(bus_handle, window_handle);
  fprintf(stdout,"VME master window addrress: 0x%lx\n",(unsigned long)phys_addr);

  scaler = (Vts801) vme_master_window_map(bus_handle, window_handle, 0);
  if(!scaler){
    perror("Error mapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    return -1;
  }

  for(i=0; i<16; i++){
    printf("Scaler[%.2d] = ", i);
      for(j=0;j<4;j++){
	printf(" %12lx ", scaler->gate[j][i]);
      }
    printf("\n");
  }

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

