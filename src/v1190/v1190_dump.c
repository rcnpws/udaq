/* v1190_dump.c ---- dump V1190 TDC data                                */
/*						                        */
/*  Version 1.00 2012-12-24 by A. Tamii (For Linux2.6)GE-FANUC          */
/*  Version 2.00 2015-03-13 by A. Tamii (separte out the dump procedure) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1190.h"

int prev_time_tag = -1;

int main(int argc, char *argv[]){
  int module_number = 0;
  FILE *fd;
  int file_flag = 0;
  unsigned long  ldata;

  V1190_p v1190=(V1190_p)NULL;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... dump v1190 TDC data.\n", argv[0]);
    fprintf(stderr, "Usage: %s [module_number]\n", argv[0]);
    fprintf(stderr, " module_number: the default is 0.\n");
    exit(0);
  }

  if(argc>1){
    if('0'<=argv[1][0] && argv[1][0]<='9'){
      module_number = atoi(argv[1]);
    }else{
      fd = fopen(argv[1], "r");
      if(fd==(FILE*)NULL){
        fprintf(stderr, "Could not open file '%s'.\n", argv[1]);
        exit(-1);
      }
      file_flag = 1;
    }
  }

  printf("Module Number: %d\n", module_number);

  /* ------------- Open ------------ */

  if(!file_flag){
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
  }

  /* ------------- Show status ------------ */

  if(!file_flag){
    unsigned long event_counter = v1190->event_counter;
    printf("Event Count: 0x%.8lx(%ld)\n", event_counter, event_counter);
  }

  /* ------------- Dump TDC data ------------ */

  unsigned long *buf;
  if(!file_flag){
    buf = v1190->output_buffer;
  }

  while(1){
    if(file_flag){
      if(fread(&ldata, sizeof(ldata), 1, fd)==0) break;
    }else{
      ldata = *buf;
    }
    if(v1190_dump_data(&ldata, sizeof(ldata))) break;
  }

  /* ------------- Close ------------ */
  if(!file_flag){
    if(v1190_unmap(module_number)){
      fprintf(stderr, "Error in v1190_unmap()\n");
    }

    if(v1190_close()){
      fprintf(stderr, "Error in v1190_close()\n");
    }
  }

  return 0; 
}


