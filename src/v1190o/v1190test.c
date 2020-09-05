/* v1190test.c ---- access test to V1190A                                  */
/*								           */
/*  Version 1.00        2012-12-23      by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1190.h"

int main(){
  int ret;
  unsigned short cr;
  unsigned short sr;
  int module_number = 0;

  V1190_p v1190=(V1190_p)NULL;

  fprintf(stderr, "open V1190A\n");
  ret = v1190_open();
  if(ret!=0){
    fprintf(stderr, "Error in v1190_open()\n");
    exit(-1);
  }

  fprintf(stderr, "map V1190A(module_number=%d)\n", module_number);
  v1190 = v1190_map(module_number);
  if(v1190==(V1190_p)NULL){
    fprintf(stderr, "Error in v1190_map()\n");
    exit(-1);
  }

  printf("v1190 = 0x%.8lx\n", (long)v1190);

//--------------
  
#if 1
  fprintf(stderr, "micro code write: V1190_OPECODE_TRIG_MATCH\n");
  ret = v1190_micro_handshake_write(v1190, V1190_OPECODE_TRIG_MATCH);
  if(ret!=0){
    fprintf(stderr, "return code from v1190_micro_handshake_write: %d\n", ret);
  }
  usleep(1000000);
#endif

#if 0
  fprintf(stderr, "micro code write: V1190_OPECODE_CONTINUOUS_STORAGE\n");
  ret = v1190_micro_handshake_write(v1190, V1190_OPECODE_CONTINUOUS_STORAGE);
  if(ret!=0){
    fprintf(stderr, "return code from v1190_micro_handshake_write: %d\n", ret);
  }
  usleep(1000000);
#endif


//--------------

  cr = v1190->control_register;
  printf("control register = 0x%.4x\n", cr);

  printf("  BERR EN: Bus Error Enable: %s\n", (cr&0x8000)==0 ? "0...Disabled" : "1...Enabled");
  printf("  TERM: Software Termination: %s\n", (cr&0x4000)==0 ? "0...OFF" : "1...ON");

  sr = v1190->status_register;
  printf("status register = 0x%.4x\n", sr);
  printf("  TRG MATCH: operating mode: %s\n", 
     (sr&0x0008)==0 ? "0...continuous" : "1...trigger matching");

  fprintf(stderr, "unmap V1190A\n");

  fprintf(stderr, "close V1190A\n");
  ret = v1190_unmap(module_number);
  if(ret!=0){
    fprintf(stderr, "Error in v1190_unmap()\n");
  }

  ret = v1190_close();
  if(ret!=0){
    fprintf(stderr, "Error in v1190_close()\n");
    exit(-1);
  }
  return 0; 
}


