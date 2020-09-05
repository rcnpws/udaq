/* MADC32.c ---- library to access MADC32                                  */
/*								           */
/*  Version 1.00        2016-03-08      by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "madc32.h"

static vme_bus_handle_t bus_handle;

static vme_master_handle_t window_handle;
static MADC32_p madc32[MADC32_MAX_N_MODULES];

static int madc32_check_existence(MADC32_p madc32);


/* 
  return code
    0 ... success
   -1 ... error
*/
int madc32_open()
{
  if(vme_init(&bus_handle)){
    perror("madc32_open: Error initializing the VMEbus");
    return -1;
  }

  return 0;
}

/* 
  if sucess MADC32 pointer is returned.
  On error NULL is returned.
*/
MADC32_p madc32_map(int module_number){
  int i;

  if(module_number<0 || MADC32_MAX_N_MODULES<=module_number){
    fprintf(stderr, "madc32_map: the specified module number (%d) is out of range.\n", 
        module_number);
    return NULL;
  }

  if(window_handle==NULL){
    if(vme_master_window_create(bus_handle, &window_handle, 
				MADC32_BASE(0), MADC32_MODE, MADC32_BASE_INC*MADC32_MAX_N_MODULES, 0, NULL)){
      perror("madc32Open: Error creating the window");
      fprintf(stderr, "madc32_map: the module number is %d\n", module_number);
      vme_term(bus_handle);
      return NULL;
    }
    
    madc32[0] = (MADC32_p)vme_master_window_map(bus_handle, window_handle, 0);
    if(madc32[0]==(MADC32_p)NULL){
      perror("madc32Open: Error mapping the window");
      vme_master_window_release(bus_handle, window_handle);
      vme_term(bus_handle);
      return NULL;
    }
    for(i=1; i<MADC32_MAX_N_MODULES; i++){
      madc32[i] = (MADC32_p)&((unsigned char*)madc32[0])[MADC32_BASE_INC*i];
    }
  }

  if(!madc32_check_existence(madc32[module_number])){
    fprintf(stderr, "madc32O_map: the module (%d) does not exist.\n", module_number);
    madc32_unmap(module_number);
    return NULL;
  }
  return madc32[module_number];
}

/* 
  return code
    0 success
   -1 error
*/
int madc32_unmap(int module_number){
  if(module_number<0 || MADC32_MAX_N_MODULES<=module_number){
    fprintf(stderr, "madc32_unmap: the specified module number (%d) is out of range.\n", 
	    module_number);
    return -1;
  }

  return 0;

}


/* 
  return code
    0 success
   -1 error
*/
int madc32_close(){
  int i;
  if(window_handle==NULL){
    fprintf(stderr, "madc32_unmap: not mapped.\n");
    return -1;
  }

  if(vme_master_window_unmap(bus_handle, window_handle)){
    perror("madc32_unmap: Error unmapping the window");
    vme_master_window_release(bus_handle, window_handle);
  }
  
  if(vme_master_window_release(bus_handle, window_handle)){
    perror("madc32_unmap: Error releasing the window");
  }
  
  window_handle = NULL;
  
  for(i=0; i<MADC32_MAX_N_MODULES; i++){
    madc32[i] =  NULL;
  }

  if(vme_term(bus_handle)){
    perror("madc32_close: Error terminating");
    return -1;
  }

  return 0;
}

/*
 * madc32_check_existence()
 * return code
 *   0 ... NOT exist
 *   1 ... exists
 *  -1 ... error
 */

static int madc32_check_existence(MADC32_p madc32){
  /* not implemented yet */
  return 1; /* exist */
}
