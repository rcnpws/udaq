/* v1190_status.c ---- show V1190 status                                   */
/*								           */
/*  Version 1.00        2013-01-17      by Itot (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1190.h"

int main(int argc, char *argv[]){
  int ret;
  unsigned short mcstcont; /* variable for MCST control */
  int module_number = 0;
  V1190_p v1190=(V1190_p)NULL;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... show v1190 status.\n", argv[0]);
    fprintf(stderr, "Usage: %s [module_number]\n", argv[0]);
    fprintf(stderr, " module_number: the default is 0.\n");
    exit(0);
  }

  if(argc>1){
    module_number = atoi(argv[1]);
  }

  /* ------------- Open ------------ */

  fprintf(stderr, "V1190A base address = 0x%.8x\n", V1190_BASE(module_number));

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

  fprintf(stderr, "v1190 = 0x%.8lx\n", (long)v1190);

  /* ------------- Show status ------------ */
  printf("----------------------------------\n");

  mcstcont = v1190->mcst_control;
  printf("MCST Control = 0x%.4x\n", mcstcont);

  if((mcstcont&0x0000)==0){	  
  	printf("Disable\n");
  }
  else if((mcstcont&0x0010)==1){	  
  	printf("Active First\n");
  }
  else if((mcstcont&0x0001)==1){	  
  	printf("Active Last\n");
  }
  else if((mcstcont&0x0011)==2){	  
  	printf("Active Midiam\n");
  }
  
  printf("----------------------------------\n");

  /* ------------- Close ------------ */
  fprintf(stderr, "unmap V1190A\n");
  ret = v1190_unmap(module_number);
  if(ret!=0){
    fprintf(stderr, "Error in v1190_unmap()\n");
  }

  fprintf(stderr, "close V1190A\n");
  ret = v1190_close();
  if(ret!=0){
    fprintf(stderr, "Error in v1190_close()\n");
    exit(-1);
  }

  return 0; 
}


