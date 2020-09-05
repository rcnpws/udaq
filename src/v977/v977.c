/* v977.c ---- library to access V977A                                   */
/*								           */
/*  Version 1.00        2013-02-01      by A. oiwa (For Linux2.6)GE-FANUC  based on v1190 source by Tamii */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v977.h"

static vme_bus_handle_t bus_handle;

static vme_master_handle_t window_handle[V977_MAX_N_MODULES];
static V977_p v977[V977_MAX_N_MODULES];

static int v977_check_existence(V977_p v977);

/* 
  return code
    0 ... success
   -1 ... error
*/
int v977_open()
{
  if(vme_init(&bus_handle)){
    perror("v977_open: Error initializing the VMEbus");
    return -1;
  }
  return 0;
}

/* 
  if sucess V977 pointer is returned.
  On error NULL is returned.
*/
V977_p v977_map(int module_number){
  if(module_number<0 || V977_MAX_N_MODULES<=module_number){
    fprintf(stderr, "v977_map: the specified module number (%d) is out of range.\n", 
        module_number);
    return NULL;
  }

  if(window_handle[module_number]!=NULL){
    fprintf(stderr, "v977_map: the module (%d) is already mapped.\n", 
        module_number);
    return NULL;
  }

  if(vme_master_window_create(bus_handle, &window_handle[module_number], 
       V977_BASE(module_number), V977_MODE, V977_SIZE, 0, NULL)){
    perror("v977Open: Error creating the window");
    fprintf(stderr, "v977_map: the module number is %d\n", module_number);
    vme_term(bus_handle);
    return NULL;
  }

  v977[module_number] = 
      (V977_p)vme_master_window_map(bus_handle, window_handle[module_number], 0);
  if(v977[module_number]==(V977_p)NULL){
    perror("v977Open: Error mapping the window");
    fprintf(stderr, "v977_map: the module number is %d\n", module_number);
    vme_master_window_release(bus_handle, window_handle[module_number]);
    vme_term(bus_handle);
    return NULL;
  }

  if(!v977_check_existence(v977[module_number])){
    fprintf(stderr, "v977O_map: the module (%d) does not exist.\n", module_number);
    v977_unmap(module_number);
    return NULL;
  }
  return v977[module_number];
}

/* 
  return code
    0 success
   -1 error
*/
int v977_unmap(int module_number){
  if(module_number<0 || V977_MAX_N_MODULES<=module_number){
    fprintf(stderr, "v977_unmap: the specified module number (%d) is out of range.\n", 
        module_number);
    return -1;
  }

  if(window_handle[module_number]==NULL){
    fprintf(stderr, "v977_unmap: the module (%d) is not opened.\n", 
        module_number);
    return -1;
  }

  if(vme_master_window_unmap(bus_handle, window_handle[module_number])){
    perror("v977_unmap: Error unmapping the window");
    fprintf(stderr, "v977_unmap: the module number is %d\n", module_number);
    vme_master_window_release(bus_handle, window_handle[module_number]);
    return -1;
  }

  if(vme_master_window_release(bus_handle, window_handle[module_number])){
    perror("v977_unmap: Error releasing the window");
    fprintf(stderr, "v977_unmap: the module number is %d\n", module_number);
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
int v977_close(){
  int module_number;
  for(module_number=0; module_number<V977_MAX_N_MODULES; module_number++){
    if(window_handle[module_number]!=NULL){
      v977_unmap(module_number);
      fprintf(stderr, "v977_close: the module %d has been unmaped before closing.\n", 
         module_number);
    }
  }

  if(vme_term(bus_handle)){
    perror("v977_close: Error terminating");
    return -1;
  }

  return 0;
}


/*
 * v977_check_existence()
 * return code
 *   0 ... NOT exist
 *   1 ... exists
 *  -1 ... error
 */

static int v977_check_existence(V977_p v977){
  if(v977==(V977_p)NULL){
    fprintf(stderr, "v977_check_existnece: null pointer.\n");
    return -1;
  }

  if(v977->control_register==0xffff){
    return 0; /* does not exist */
  }
  return 1; /* exist */
}
