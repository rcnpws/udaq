/*
 * v830_read.c ... CAEN v830 100MHz scaler module access program
 * for GE Fanuc VMIVME-7807RC
 * Version 0.01 on 2009.03.18 by A. Tamii
 * Version 0.5  on 2014.11.02 by A. Tamii
 *
 */

#include <vme/vme.h>
#include <vme/vme_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "v830.h"

void usage(char *pname){
  fprintf(stdout, "%s ... read v830d scaler\n", pname);
  fprintf(stdout, "Usage: %s [-h] [-c] [module number]\n", pname);
  fprintf(stdout, " -h: show this help.\n");
  fprintf(stdout, " -c: clear the module\n");
  fprintf(stdout, " module_number: the default is 0.\n");
}

int main(int argc, char **argv){

  /*  uint8_t *ptr; */
  char *pname;
  V830_p v830 = (V830_p)NULL;

  int module_number = 0;
  int init_flag = 0;
  int clear_flag = 0;

  int i;
  int result = 0;
  unsigned long counter;

  pname = argv[0];
  argc--;
  argv++;

  while(argc>0){
    if(!strcmp(argv[0],"-h")){
      usage(pname);
      exit(0);
    }else if(!strcmp(argv[0],"-c")){
      clear_flag = 1;
    }else if(!strcmp(argv[0],"-i")){
      init_flag = 1;
    }else if('0'<=argv[0][0] && argv[0][0]<='9'){
      module_number = atoi(argv[0]);
    }else{
      fprintf(stderr, "Unrecognized argument '%s'.", argv[0]);
      exit(-1);
    }
    argc--;
    argv++;
  }

  fprintf(stdout, "V830 module number = %d.\n", module_number);
  fprintf(stdout, "Base address = %8x\n", V830_BASE(module_number));
   
  
  if(v830_open()){
    perror("Error in v830_open()");
    return -1;
  }

  if((v830=v830_map(module_number))==(V830_p)NULL){
    perror("Error in v830_map()");
    v830_close();
    return -1;
  }

  if(clear_flag){
    fprintf(stdout, "Clear the module.\n");
    v830->softwareClear = 1;
  }

  for(i=0; i<32; i++){
    counter = v830->counter[i];
    printf("Scaler[%.2d] = %12ld   = 0x%.12lx \n", i, counter, counter);
  }

  fprintf(stdout, "Event number = %d\n", v830->MEBEventNumber);

  for(i=0; i<sizeof(v830->meb); i++){
    fprintf(stdout, "%8lx\n", *v830->meb);
  }

  if(v830_unmap(module_number)){
    perror("Error in v830_unmap()");
    result = -1;
  }
  v830 = (V830_p)NULL;

  if(v830_close()){
    perror("Error in v830_close()");
    result = -1;
  }

  return result;
}

