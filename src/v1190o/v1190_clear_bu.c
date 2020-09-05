/* v1190_cont_storage_mode ---- set V1190A to the continuous storage  mode */
/*								           */
/*  Version 1.00        2013-01-23      by Itot (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1190.h"

int main(int argc, char *argv[]){
  int ret;
  int module_number = 0;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... set v1190 to the continuous storage mode.\n", argv[0]);
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

  fprintf(stderr, "Register address map write: V1190_SoftwareClear\n");
  ret = v1190_micro_handshake_write(v1190, V1190_SoftwareClear);
  if(ret!=0){
    fprintf(stderr, "return code from v1190_micro_handshake_write: %d\n", ret);
  }
  usleep(1000000);  /* this wait is required before reading the status. */


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


