/* v1720.c ---- library to access V1720                                   */
/*								           */
/*  Version 1.00        2013-07-31      by A. oiwa (For Linux2.6)GE-FANUC  based on v1190 source by Tamii */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1720.h"

static vme_bus_handle_t bus_handle;

static vme_master_handle_t window_handle[V1720_MAX_N_MODULES];
static V1720_p v1720[V1720_MAX_N_MODULES];

static int v1720_check_existence(V1720_p v1720);

/* 
  return code
    0 ... success
   -1 ... error
*/
int v1720_open()
{
  if(vme_init(&bus_handle)){
    perror("v1720_open: Error initializing the VMEbus");
    return -1;
  }
  return 0;
}

/* 
  if sucess V1720 pointer is returned.
  On error NULL is returned.
*/
V1720_p v1720_map(int module_number){
  if(module_number<0 || V1720_MAX_N_MODULES<=module_number){
    fprintf(stderr, "v1720_map: the specified module number (%d) is out of range.\n", 
        module_number);
    return NULL;
  }

  if(window_handle[module_number]!=NULL){
    fprintf(stderr, "v1720_map: the module (%d) is already mapped.\n", 
        module_number);
    return NULL;
  }

  if(vme_master_window_create(bus_handle, &window_handle[module_number], 
       V1720_BASE(module_number), V1720_MODE, V1720_SIZE, 0, NULL)){
    perror("v1720Open: Error creating the window");
    fprintf(stderr, "v1720_map: the module number is %d\n", module_number);
    vme_term(bus_handle);
    return NULL;
  }

  v1720[module_number] = 
      (V1720_p)vme_master_window_map(bus_handle, window_handle[module_number], 0);
  if(v1720[module_number]==(V1720_p)NULL){
    perror("v1720Open: Error mapping the window");
    fprintf(stderr, "v1720_map: the module number is %d\n", module_number);
    vme_master_window_release(bus_handle, window_handle[module_number]);
    vme_term(bus_handle);
    return NULL;
  }

  if(!v1720_check_existence(v1720[module_number])){
    fprintf(stderr, "v1720O_map: the module (%d) does not exist.\n", module_number);
    v1720_unmap(module_number);
    return NULL;
  }
  return v1720[module_number];
}

/* 
  return code
    0 success
   -1 error
*/
int v1720_unmap(int module_number){
  if(module_number<0 || V1720_MAX_N_MODULES<=module_number){
    fprintf(stderr, "v1720_unmap: the specified module number (%d) is out of range.\n", 
        module_number);
    return -1;
  }

  if(window_handle[module_number]==NULL){
    fprintf(stderr, "v1720_unmap: the module (%d) is not opened.\n", 
        module_number);
    return -1;
  }

  if(vme_master_window_unmap(bus_handle, window_handle[module_number])){
    perror("v1720_unmap: Error unmapping the window");
    fprintf(stderr, "v1720_unmap: the module number is %d\n", module_number);
    vme_master_window_release(bus_handle, window_handle[module_number]);
    return -1;
  }

  if(vme_master_window_release(bus_handle, window_handle[module_number])){
    perror("v1720_unmap: Error releasing the window");
    fprintf(stderr, "v1720_unmap: the module number is %d\n", module_number);
    return -1;
  }

  window_handle[module_number] = NULL;

  return 0;
}


/* 
  return code
    0 success
   -1 error
*/
int v1720_close(){
  int module_number;
  for(module_number=0; module_number<V1720_MAX_N_MODULES; module_number++){
    if(window_handle[module_number]!=NULL){
      v1720_unmap(module_number);
      fprintf(stderr, "v1720_close: the module %d has been unmaped before closing.\n", 
         module_number);
    }
  }

  if(vme_term(bus_handle)){
    perror("v1720_close: Error terminating");
    return -1;
  }

  return 0;
}


/*
 * v1720_check_existence()
 * return code
 *   0 ... NOT exist
 *   1 ... exists
 *  -1 ... error
 */

static int v1720_check_existence(V1720_p v1720){
  if(v1720==(V1720_p)NULL){
    fprintf(stderr, "v1720_check_existnece: null pointer.\n");
    return -1;
  }
   if(v1720->BOARD_INFO==0xffff){
	  return 0; // does not exist
  }
  return 1; /* exist */
}
