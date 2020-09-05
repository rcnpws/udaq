/* v792.c ---- library to access V792                                   */
/*								           */
/*  Version 1.00        2013-07-31      by A. oiwa (For Linux2.6)GE-FANUC  based on v1190 source by Tamii */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v792.h"

static vme_bus_handle_t bus_handle;

static vme_master_handle_t window_handle[V792_MAX_N_MODULES];
static V792_p v792[V792_MAX_N_MODULES];

static int v792_check_existence(V792_p v792);

/* 
  return code
    0 ... success
   -1 ... error
*/
int v792_open()
{
  if(vme_init(&bus_handle)){
    perror("v792_open: Error initializing the VMEbus");
    return -1;
  }
  return 0;
}

/* 
  if sucess V792 pointer is returned.
  On error NULL is returned.
*/
V792_p v792_map(int module_number){
  if(module_number<0 || V792_MAX_N_MODULES<=module_number){
    fprintf(stderr, "v792_map: the specified module number (%d) is out of range.\n", 
        module_number);
    return NULL;
  }

  if(window_handle[module_number]!=NULL){
    fprintf(stderr, "v792_map: the module (%d) is already mapped.\n", 
        module_number);
    return NULL;
  }

  if(vme_master_window_create(bus_handle, &window_handle[module_number], 
       V792_BASE(module_number), V792_MODE, V792_SIZE, 0, NULL)){
    perror("v792Open: Error creating the window");
    fprintf(stderr, "v792_map: the module number is %d\n", module_number);
    vme_term(bus_handle);
    return NULL;
  }

  v792[module_number] = 
      (V792_p)vme_master_window_map(bus_handle, window_handle[module_number], 0);
  if(v792[module_number]==(V792_p)NULL){
    perror("v792Open: Error mapping the window");
    fprintf(stderr, "v792_map: the module number is %d\n", module_number);
    vme_master_window_release(bus_handle, window_handle[module_number]);
    vme_term(bus_handle);
    return NULL;
  }

  if(!v792_check_existence(v792[module_number])){
    fprintf(stderr, "v792O_map: the module (%d) does not exist.\n", module_number);
    v792_unmap(module_number);
    return NULL;
  }
  return v792[module_number];
}

/* 
  return code
    0 success
   -1 error
*/
int v792_unmap(int module_number){
  if(module_number<0 || V792_MAX_N_MODULES<=module_number){
    fprintf(stderr, "v792_unmap: the specified module number (%d) is out of range.\n", 
        module_number);
    return -1;
  }

  if(window_handle[module_number]==NULL){
    fprintf(stderr, "v792_unmap: the module (%d) is not opened.\n", 
        module_number);
    return -1;
  }

  if(vme_master_window_unmap(bus_handle, window_handle[module_number])){
    perror("v792_unmap: Error unmapping the window");
    fprintf(stderr, "v792_unmap: the module number is %d\n", module_number);
    vme_master_window_release(bus_handle, window_handle[module_number]);
    return -1;
  }

  if(vme_master_window_release(bus_handle, window_handle[module_number])){
    perror("v792_unmap: Error releasing the window");
    fprintf(stderr, "v792_unmap: the module number is %d\n", module_number);
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
int v792_close(){
  int module_number;
  for(module_number=0; module_number<V792_MAX_N_MODULES; module_number++){
    if(window_handle[module_number]!=NULL){
      v792_unmap(module_number);
      fprintf(stderr, "v792_close: the module %d has been unmaped before closing.\n", 
         module_number);
    }
  }

  if(vme_term(bus_handle)){
    perror("v792_close: Error terminating");
    return -1;
  }

  return 0;
}


/*
 * v792_check_existence()
 * return code
 *   0 ... NOT exist
 *   1 ... exists
 *  -1 ... error
 */

static int v792_check_existence(V792_p v792){
  if(v792==(V792_p)NULL){
    fprintf(stderr, "v792_check_existnece: null pointer.\n");
    return -1;
  }
  if(v792->status_register_1==0xffff){
	  return 0; // does not exist
  }
  return 1; /* exist */
}
