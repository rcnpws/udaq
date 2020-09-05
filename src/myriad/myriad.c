/* myriad.c ---- library to access MyRIAD                                  */
/*								           */
/*  Version 1.00        2016-03-05     by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "myriad.h"

static vme_bus_handle_t bus_handle;

static vme_master_handle_t window_handle;

static MyRIAD_p myriad = (MyRIAD_p)NULL;

/*
  return code
    0 ... success
   -1 ... error
*/
int myriad_open()
{
  if(vme_init(&bus_handle)){
    perror("myriad_open: Error initializing the VMEbus");
    return -1;
  }

  return 0;
}

/* 
  if sucess MyRIAD pointer is returned.
  On error NULL is returned.
*/
MyRIAD_p myriad_map(){
  if(window_handle==(vme_master_handle_t)NULL){
    if(vme_master_window_create(bus_handle, &window_handle, 
				MYRIAD_BASE, MYRIAD_MODE, MYRIAD_SIZE, 0, NULL)){
      perror("myriad_map: Error on creating the window");
      vme_term(bus_handle);
      return NULL;
    }
    
    myriad = (MyRIAD_p)vme_master_window_map(bus_handle, window_handle, 0);
    if(myriad==(MyRIAD_p)NULL){
      perror("myriad_map: Error on mapping the window");
      vme_master_window_release(bus_handle, window_handle);
      vme_term(bus_handle);
      return NULL;
    }
  }

#if 0
  if(!v1190_check_existence(v1190[module_number])){
    fprintf(stderr, "v1190O_map: the module (%d) does not exist.\n", module_number);
    v1190_unmap(module_number);
    return NULL;
  }
#endif
  return myriad;
}

/* 
  return code
    0 success
   -1 error
*/
int myriad_unmap(){
  if(window_handle==(vme_master_handle_t)NULL){
    fprintf(stderr, "myriad_unmap: not mapped.\n");
    return -1;
  }
  if(myriad==(MyRIAD_p)NULL){
    fprintf(stderr, "myriad_unmap: not mapped.\n");
  }else{
    if(vme_master_window_unmap(bus_handle, window_handle)){
      perror("myriad_close: Error on unmapping the window");
    }
  }
  vme_master_window_release(bus_handle, window_handle);

  myriad = (MyRIAD_p)NULL;
  window_handle = (vme_master_handle_t)NULL;

  return 0;
}


/* 
  return code
    0 success
   -1 error
*/
int myriad_close(){
  return 0;
}

