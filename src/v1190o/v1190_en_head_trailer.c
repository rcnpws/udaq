/* v1190_cont_storage_mode ---- set V1190A to the continuous storage  mode */
/*								           */
/*  Version 1.00        2012-12-24      by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1190.h"

int main(int argc, char *argv[]){
  int ret;
  unsigned short sr;
  int module_number = 0;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... set v1190 to enable TDC header and TRAILER.\n", argv[0]);
    fprintf(stderr, "Usage: %s [module_number]\n", argv[0]);
    fprintf(stderr, " module_number: the default is 0.\n");
    exit(0);
  }

  if(argc>1){
    module_number = atoi(argv[1]);
  }

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
    v1190_close();
    exit(-1);
  }

  fprintf(stderr, "micro code write: V1190_OPECODE_EN_HEAD_TRAILER\n");
  ret = v1190_micro_handshake_write(v1190, V1190_OPECODE_EN_HEAD_TRAILER);
  if(ret!=0){
    fprintf(stderr, "return code from v1190_micro_handshake_write: %d\n", ret);
  }
  usleep(1000000);  /* this wait is required before reading the status. */

  sr = v1190->status_register;
  printf("status register = 0x%.4x\n", sr);
  printf("  HEADER EN		: TDC's Header and TRAILER		: %s\n", 
     (sr&0x0010)==0 ? "0...Disabled" : "1...Enabled");

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


