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

static vme_master_handle_t window_handle;
static V1190_p v1190[V1190_MAX_N_MODULES];

static int v1190_check_existence(V1190_p v1190);


char *(v1190_tdc_error_string[]) = {
  "Hit lost in group 0 from read-out FIFO overflow",
  "Hit lost in group 0 from L1 buffer overflow",
  "Hit error have been detected in group 0",
  "Hit lost in group 1 from read-out FIFO overflow",
  "Hit lost in group 1 from L1 buffer overflow",
  "Hit error have been detected in group 1",
  "Hit lost in group 2 from read-out FIFO overflow",
  "Hit lost in group 2 from L1 buffer overflow",
  "Hit error have been detected in group 2",
  "Hit lost in group 3 from read-out FIFO overflow",
  "Hit lost in group 3 from L1 buffer overflow",
  "Hit error have been detected in group 3",
  "Hits rejected because of programmed event size limit",
  "Event lost (trigger FIOF overflow)",
  "Internal fatal chip error has been detected"
};

// See Sec. 5.5.4 of the V1190 manual
char *max_number_hits_s[] = {
  "0 hits",   // 0 = 000
  "1 hits",   // 1 = 0001
  "2 hits",   // 2 = 0010
  "4 hits",   // 3 = 0011
  "8 hits",   // 4 = 0100
  "16 hits",  // 5 = 0101
  "32 hits",  // 6 = 0110
  "64 hits",  // 7 = 0111
  "128 hits", // 8 = 1000
  "no limit", // 9 = 1001
  "undefined",//10
  "undefined",//11
  "undefined",//12
  "undefined",//13
  "undefined",//14
  "undefined" //15
};



int v1190_n_tdc_error_string = sizeof(v1190_tdc_error_string)/sizeof(char *);



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
  int i;

  if(module_number<0 || V1190_MAX_N_MODULES<=module_number){
    fprintf(stderr, "v1190_map: the specified module number (%d) is out of range.\n", 
        module_number);
    return NULL;
  }

  if(window_handle==NULL){
    if(vme_master_window_create(bus_handle, &window_handle, 
				V1190_BASE(0), V1190_MODE, V1190_BASE_INC*V1190_MAX_N_MODULES, 0, NULL)){
      perror("v1190Open: Error creating the window");
      fprintf(stderr, "v1190_map: the module number is %d\n", module_number);
      vme_term(bus_handle);
      return NULL;
    }
    
    v1190[0] = (V1190_p)vme_master_window_map(bus_handle, window_handle, 0);
    if(v1190[0]==(V1190_p)NULL){
      perror("v1190Open: Error mapping the window");
      vme_master_window_release(bus_handle, window_handle);
      vme_term(bus_handle);
      return NULL;
    }
    for(i=1; i<V1190_MAX_N_MODULES; i++){
      v1190[i] = (V1190_p)&((unsigned char*)v1190[0])[V1190_BASE_INC*i];
    }
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

  return 0;

}


/* 
  return code
    0 success
   -1 error
*/
int v1190_close(){
  int i;
  if(window_handle==NULL){
    fprintf(stderr, "v1190_unmap: not mapped.\n");
    return -1;
  }

  if(vme_master_window_unmap(bus_handle, window_handle)){
    perror("v1190_unmap: Error unmapping the window");
    vme_master_window_release(bus_handle, window_handle);
  }
  
  if(vme_master_window_release(bus_handle, window_handle)){
    perror("v1190_unmap: Error releasing the window");
  }
  
  window_handle = NULL;
  
  for(i=0; i<V1190_MAX_N_MODULES; i++){
    v1190[i] =  NULL;
  }

  if(vme_term(bus_handle)){
    perror("v1190_close: Error terminating");
    return -1;
  }

  return 0;
}

/* 
 internal command
 v1190_mciro_write_w()
 write a word to micro
  return code
    0 success
*/

static int v1190_micro_write_w(V1190_p v1190, unsigned short w){
  v1190->micro = w;
  return 0;
}


/* 
 internal command
 v1190_mciro_handshake_read_wait()
 wait for read data ready for micro handshake read
  return code
    0 success
    1 timeout
*/

static int v1190_micro_handshake_read_wait(V1190_p v1190){
  int i;
  for(i=0; i<10; i++){
    if(v1190->micro_handshake & V1190_MicroHandshakeRegister_ReadOK)
      return 0;
    usleep(100000);
  }
  return 1;
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
  unsigned short mh;

  if(v1190==(V1190_p)NULL){
    fprintf(stderr, "v1190_micro_handshake_write: null pointer.\n");
    return -1;
  }

  if((v1190->micro_handshake & V1190_MicroHandshakeRegister_WriteOK)==0){
    fprintf(stderr, "v1190_micro_handshake_write: warning the board is not ready for the write action.\n");
  mh=v1190->micro_handshake;
  printf("micro_handshake :  %x\n", mh);
    return 1;
  }

  mh=v1190->micro_handshake;
  //printf("micro_handshake :  %x\n", mh);
  v1190->micro = opecode;
  return 0;
}

/* 
  v1190_micro_handshake_write_1W()
  Perform micro-code handshake write operation for one word.

  Note that depending on the operation, user need to wait for ~1 second until
  the operation is completed.

  return code
    0 success
    1 the board is not ready for the write action
   -1 error
*/
int v1190_micro_handshake_write_1w(V1190_p v1190, int opecode, unsigned short w){
  int res;
 
  res = v1190_micro_handshake_write(v1190, opecode);
  if(res) return res;

  v1190_micro_write_w(v1190, w);

  return 0;
  
}

/* 
  v1190_micro_handshake_read()
  Perform micro-code handshake read operation for one word.

  Note that depending on the operation, user need to wait for ~1 second until
  the operation is completed.

  return code
    0 success
    1 the board is not ready for the write action
   -1 error
*/
int v1190_micro_handshake_read(V1190_p v1190, int opecode, unsigned short *data){
  if(v1190==(V1190_p)NULL){
    fprintf(stderr, "v1190_micro_handshake_read: null pointer.\n");
    return -1;
  }

  //  if(v1190_micro_handshake_read_wait(v1190)){
  //    fprintf(stderr, "v1190_micro_handshake_read #1: timeout\n");
  //    return 1;
  //  }

  v1190->micro = opecode;

  if(v1190_micro_handshake_read_wait(v1190)){
    fprintf(stderr, "v1190_micro_handshake_read #2: timeout\n");
    return 1;
  }

  *data = v1190->micro;
  return 0;
}

int v1190_micro_handshake_read_cont(V1190_p v1190, unsigned short *data){
  if(v1190==(V1190_p)NULL){
    fprintf(stderr, "v1190_micro_handshake_read_cont: null pointer.\n");
    return -1;
  }

  if(v1190_micro_handshake_read_wait(v1190)){
    fprintf(stderr, "v1190_micro_handshake_read_cont: timeout\n");
    return 1;
  }

  *data = v1190->micro;
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

