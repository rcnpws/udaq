/* v830.c ---- library to access V830 latching scaler                      */
/*								           */
/*  Version 1.00      02-NOV-2014  by A. Tamii (For Linux2.6)GE-FANUC      */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v830.h"

static vme_bus_handle_t bus_handle;
static vme_master_handle_t window_handle[V830_MAX_N_MODULES];
static V830_p v830[V830_MAX_N_MODULES];

static int v830_check_existence(V830_p v830);

/* 
  return code
    0 ... success
   -1 ... error
*/
int v830_open()
{
  if(vme_init(&bus_handle)){
    perror("v830_open: Error initializing the VMEbus");
    return -1;
  }
  return 0;
}

/* 
  if sucess V830 pointer is returned.
  On error NULL is returned.
*/
V830_p v830_map(int module_number){
  if(module_number<0 || V830_MAX_N_MODULES<=module_number){
    fprintf(stderr, "v830_map: the specified module number (%d) is out of range.\n", 
        module_number);
    return NULL;
  }

  if(window_handle[module_number]!=NULL){
    fprintf(stderr, "v830_map: the module (%d) is already mapped.\n", 
        module_number);
    return NULL;
  }

  if(vme_master_window_create(bus_handle, &window_handle[module_number], 
       V830_BASE(module_number), V830_MODE, V830_SIZE, 0, NULL)){
    perror("v830_map: Error creating the window");
    fprintf(stderr, "v830_map: the module number is %d\n", module_number);
    vme_term(bus_handle);
    return NULL;
  }

  v830[module_number] = 
      (V830_p)vme_master_window_map(bus_handle, window_handle[module_number], 0);
  if(v830[module_number]==(V830_p)NULL){
    perror("v830Open: Error mapping the window");
    fprintf(stderr, "v830_map: the module number is %d\n", module_number);
    vme_master_window_release(bus_handle, window_handle[module_number]);
    vme_term(bus_handle);
    return NULL;
  }

  if(!v830_check_existence(v830[module_number])){
    fprintf(stderr, "v830O_map: the module (%d) does not exist.\n", module_number);
    v830_unmap(module_number);
    return NULL;
  }
  return v830[module_number];
}

/* 
  return code
    0 success
   -1 error
*/
int v830_unmap(int module_number){
  if(module_number<0 || V830_MAX_N_MODULES<=module_number){
    fprintf(stderr, "v830_unmap: the specified module number (%d) is out of range.\n", 
        module_number);
    return -1;
  }

  if(window_handle[module_number]==NULL){
    fprintf(stderr, "v830_unmap: the module (%d) is not opened.\n", 
        module_number);
    return -1;
  }

  if(vme_master_window_unmap(bus_handle, window_handle[module_number])){
    perror("v830_unmap: Error unmapping the window");
    fprintf(stderr, "v830_unmap: the module number is %d\n", module_number);
    vme_master_window_release(bus_handle, window_handle[module_number]);
    return -1;
  }

  if(vme_master_window_release(bus_handle, window_handle[module_number])){
    perror("v830_unmap: Error releasing the window");
    fprintf(stderr, "v830_unmap: the module number is %d\n", module_number);
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
int v830_close(){
  int module_number;
  for(module_number=0; module_number<V830_MAX_N_MODULES; module_number++){
    if(window_handle[module_number]!=NULL){
      v830_unmap(module_number);
      fprintf(stderr, "v830_close: the module %d has been unmaped before closing.\n", 
         module_number);
    }
  }

  if(vme_term(bus_handle)){
    perror("v830_close: Error terminating");
    return -1;
  }

  return 0;
}


int v830_clear_counters(V830_p v830p){
  v830p->softwareClear = 1;
  return 0;
}

/*
 * v830_check_existence()
 * return code
 *   0 ... NOT exist
 *   1 ... exists
 *  -1 ... error
 */

static int v830_check_existence(V830_p v830){
  if(v830==(V830_p)NULL){
    fprintf(stderr, "v830_check_existnece: null pointer.\n");
    return -1;
  }
  if(v830->controlRegister==0xffff){
    return 0; /* does not exist */
  }
  return 1; /* exist */
}
