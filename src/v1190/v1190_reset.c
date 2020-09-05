/* v1190_software_trigger.c ---- generate a software trigger to V1190      */
/*								           */
/*  Version 1.00        2012-12-24      by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1190.h"

int main(int argc, char *argv[]){
  int module_number = 0;
  V1190_p v1190=(V1190_p)NULL;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... generate a module reset to V1190.\n", argv[0]);
    fprintf(stderr, "Usage: %s [module_number]\n", argv[0]);
    fprintf(stderr, " module_number: the default is 0.\n");
    exit(0);
  }

  if(argc>1){
    module_number = atoi(argv[1]);
  }

  printf("Module Number: %d\n", module_number);

  /* ------------- Open ------------ */

  if(v1190_open()){
    fprintf(stderr, "Error in v1190_open()\n");
    exit(-1);
  }

  v1190 = v1190_map(module_number);
  if(v1190==(V1190_p)NULL){
    fprintf(stderr, "Error in v1190_map()\n");
    v1190_close();
    exit(-1);
  }

  /* ------------- Jobs ------------ */

  printf("Generate a module reset to %d\n", module_number);

  v1190->module_reset = 0x01;

  /* ------------- Close ------------ */
  if(v1190_unmap(module_number)){
    fprintf(stderr, "Error in v1190_unmap()\n");
  }

  if(v1190_close()){
    fprintf(stderr, "Error in v1190_close()\n");
  }

  return 0; 
}


