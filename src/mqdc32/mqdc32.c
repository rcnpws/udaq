/* MQDC32.c ---- library to access MQDC32                                  */
/*								           */
/*  Version 0.10        2020-09-05      by T. Furuno (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "mqdc32.h"

static vme_bus_handle_t bus_handle;

static vme_master_handle_t window_handle;
static MQDC32_p mqdc32[MQDC32_MAX_N_MODULES];

static int mqdc32_check_existence(MQDC32_p mqdc32);


/* 
  return code
    0 ... success
   -1 ... error
*/
int mqdc32_open()
{
  if(vme_init(&bus_handle)){
    perror("mqdc32_open: Error initializing the VMEbus");
    return -1;
  }

  return 0;
}

/* 
  if sucess MQDC32 pointer is returned.
  On error NULL is returned.
*/
MQDC32_p mqdc32_map(int module_number){
  int i;

  if(module_number<0 || MQDC32_MAX_N_MODULES<=module_number){
    fprintf(stderr, "mqdc32_map: the specified module number (%d) is out of range.\n", 
        module_number);
    return NULL;
  }

  if(window_handle==NULL){
    if(vme_master_window_create(bus_handle, &window_handle, 
				MQDC32_BASE(0), MQDC32_MODE, MQDC32_BASE_INC*MQDC32_MAX_N_MODULES, 0, NULL)){
      perror("mqdc32Open: Error creating the window");
      fprintf(stderr, "mqdc32_map: the module number is %d\n", module_number);
      vme_term(bus_handle);
      return NULL;
    }
    
    mqdc32[0] = (MQDC32_p)vme_master_window_map(bus_handle, window_handle, 0);
    if(mqdc32[0]==(MQDC32_p)NULL){
      perror("mqdc32Open: Error mapping the window");
      vme_master_window_release(bus_handle, window_handle);
      vme_term(bus_handle);
      return NULL;
    }
    for(i=1; i<MQDC32_MAX_N_MODULES; i++){
      mqdc32[i] = (MQDC32_p)&((unsigned char*)mqdc32[0])[MQDC32_BASE_INC*i];
    }
  }

  if(!mqdc32_check_existence(mqdc32[module_number])){
    fprintf(stderr, "mqdc32O_map: the module (%d) does not exist.\n", module_number);
    mqdc32_unmap(module_number);
    return NULL;
  }
  return mqdc32[module_number];
}

/* 
  return code
    0 success
   -1 error
*/
int mqdc32_unmap(int module_number){
  if(module_number<0 || MQDC32_MAX_N_MODULES<=module_number){
    fprintf(stderr, "mqdc32_unmap: the specified module number (%d) is out of range.\n", 
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
int mqdc32_close(){
  int i;
  if(window_handle==NULL){
    fprintf(stderr, "mqdc32_unmap: not mapped.\n");
    return -1;
  }

  if(vme_master_window_unmap(bus_handle, window_handle)){
    perror("mqdc32_unmap: Error unmapping the window");
    vme_master_window_release(bus_handle, window_handle);
  }
  
  if(vme_master_window_release(bus_handle, window_handle)){
    perror("mqdc32_unmap: Error releasing the window");
  }
  
  window_handle = NULL;
  
  for(i=0; i<MQDC32_MAX_N_MODULES; i++){
    mqdc32[i] =  NULL;
  }

  if(vme_term(bus_handle)){
    perror("mqdc32_close: Error terminating");
    return -1;
  }

  return 0;
}

/*
 * mqdc32_check_existence()
 * return code
 *   0 ... NOT exist
 *   1 ... exists
 *  -1 ... error
 */

static int mqdc32_check_existence(MQDC32_p mqdc32){
  /* not implemented yet */
  return 1; /* exist */
}
