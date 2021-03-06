/* v1190.c ---- library to access V1190A                                   */
/*								           */
/*  Version 1.00        2012-12-23      by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1190.h"

static vme_bus_handle_t bus_handle;

static vme_master_handle_t window_handle[V1190_MAX_N_MODULES];
static V1190_p v1190[V1190_MAX_N_MODULES];

static int v1190_check_existence(V1190_p v1190);

/* 
  return code
    0 ... success
   -1 ... error
*/
int v1190_open()
{
  if(vme_init(&bus_handle)){
    perror("v1190_open: Error initializing the VMEbus");
    return -1;
  }
  return 0;
}

/* 
  if sucess V1190 pointer is returned.
  On error NULL is returned.
*/
V1190_p v1190_map(int module_number){
  if(module_number<0 || V1190_MAX_N_MODULES<=module_number){
    fprintf(stderr, "v1190_map: the specified module number (%d) is out of range.\n", 
        module_number);
    return NULL;
  }

  if(window_handle[module_number]!=NULL){
    fprintf(stderr, "v1190_map: the module (%d) is already mapped.\n", 
        module_number);
    return NULL;
  }

  if(vme_master_window_create(bus_handle, &window_handle[module_number], 
       V1190_BASE(module_number), V1190_MODE, V1190_SIZE, 0, NULL)){
    perror("v1190Open: Error creating the window");
    fprintf(stderr, "v1190_map: the module number is %d\n", module_number);
    vme_term(bus_handle);
    return NULL;
  }

  v1190[module_number] = 
      (V1190_p)vme_master_window_map(bus_handle, window_handle[module_number], 0);
  if(v1190[module_number]==(V1190_p)NULL){
    perror("v1190Open: Error mapping the window");
    fprintf(stderr, "v1190_map: the module number is %d\n", module_number);
    vme_master_window_release(bus_handle, window_handle[module_number]);
    vme_term(bus_handle);
    return NULL;
  }

  if(!v1190_check_existence(v1190[module_number])){
    fprintf(stderr, "v1190O_map: the module (%d) does not exist.\n", module_number);
    v1190_unmap(module_number);
    return NULL;
  }
  return v1190[module_number];
}

/* 
  return code
    0 success
   -1 error
*/
int v1190_unmap(int module_number){
  if(module_number<0 || V1190_MAX_N_MODULES<=module_number){
    fprintf(stderr, "v1190_unmap: the specified module number (%d) is out of range.\n", 
        module_number);
    return -1;
  }

  if(window_handle[module_number]==NULL){
    fprintf(stderr, "v1190_unmap: the module (%d) is not opened.\n", 
        module_number);
    return -1;
  }

  if(vme_master_window_unmap(bus_handle, window_handle[module_number])){
    perror("v1190_unmap: Error unmapping the window");
    fprintf(stderr, "v1190_unmap: the module number is %d\n", module_number);
    vme_master_window_release(bus_handle, window_handle[module_number]);
    return -1;
  }

  if(vme_master_window_release(bus_handle, window_handle[module_number])){
    perror("v1190_unmap: Error releasing the window");
    fprintf(stderr, "v1190_unmap: the module number is %d\n", module_number);
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
int v1190_close(){
  int module_number;
  for(module_number=0; module_number<V1190_MAX_N_MODULES; module_number++){
    if(window_handle[module_number]!=NULL){
      v1190_unmap(module_number);
      fprintf(stderr, "v1190_close: the module %d has been unmaped before closing.\n", 
         module_number);
    }
  }

  if(vme_term(bus_handle)){
    perror("v1190_close: Error terminating");
    return -1;
  }

  return 0;
}

/* 
  v1190_micro_handshake_write()
  Perform micro-code handshake write operation.

  Note that depending on the operation, user need to wait for ~1 second until
  the operation is completed.

  return code
    0 success
    1 the board is not ready for the write action
   -1 error
*/
int v1190_micro_handshake_write(V1190_p v1190, int opecode){
//  unsigned short mh;
  fprintf(stderr,"opecode adrress : 0x%.4x \n",opecode);

  if(v1190==(V1190_p)NULL){
    fprintf(stderr, "v1190_micro_handshake_write: null pointer.\n");
    return -1;
  }

  if((v1190->micro_handshake & V1190_MicroHandshakeRegister_WriteOK)==0){
    fprintf(stderr, "v1190_micro_handshake_write: warning the board is not ready for the write action.\n");
//  mh=v1190->micro_handshake;
//  printf("micro_handshake :  %x\n", mh);
    return 1;
  }

//  mh=v1190->micro_handshake;
//  printf("micro_handshake :  %x\n", mh);
  v1190->micro = opecode;
  return 0;
}

int v1190_micro_handshake_read(V1190_p v1190, int opecode){
//  unsigned short mh;
  fprintf(stderr,"opecode adrress : 0x%.4x \n",opecode);
  if(v1190==(V1190_p)NULL){
    fprintf(stderr, "v1190_micro_handshake_read: null pointer.\n");
    return -1;
  }

  if((v1190->micro_handshake & V1190_MicroHandshakeRegister_ReadOK)==0){
    fprintf(stderr, "v1190_micro_handshake_read: warning the board is not ready for the read action.\n");
    //mh=v1190->micro_handshake;
    //printf("micro_handshake :  %x\n", mh);
    return 1;
  }

  v1190->micro = opecode;
  return 0;
}


/*
 * v1190_check_existence()
 * return code
 *   0 ... NOT exist
 *   1 ... exists
 *  -1 ... error
 */

static int v1190_check_existence(V1190_p v1190){
  if(v1190==(V1190_p)NULL){
    fprintf(stderr, "v1190_check_existnece: null pointer.\n");
    return -1;
  }

  if(v1190->control_register==0xffff){
    return 0; /* does not exist */
  }
  return 1; /* exist */
}
