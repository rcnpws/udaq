/*
 * vme_reset ... generate VME sys-resest 
 * for GE Fanuc VMIVME-7807RC
 * Version 1.0  on 2014.11.02 by A. Tamii
 *
 */

#include <vme/vme.h>
#include <vme/vme_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void usage(char *pname){
  fprintf(stdout, "%s ... generate VME sys-reset\n", pname);
  fprintf(stdout, "Usage: %s [-h] \n", pname);
  fprintf(stdout, " -h: show this help.\n");
}

int main(int argc, char **argv){

  vme_bus_handle_t bus_handle;
  char *pname;

  int result = 0;

  pname = argv[0];
  argc--;
  argv++;

  fprintf(stderr, "This command causes instantaneous CPU reset.\n");
  fprintf(stderr, "Use the shutdown command.\n");
  exit(0);

  while(argc>0){
    if(!strcmp(argv[0],"-h")){
      usage(pname);
      exit(0);
    }else{
      fprintf(stderr, "Unrecognized argument '%s'.", argv[0]);
      exit(-1);
    }
    argc--;
    argv++;
  }

  if(vme_init(&bus_handle)){
    perror("Error on initializing the VMEbus");
    return -1;
  }

  fprintf(stdout, "Generate VME Sys-Reset.\n");

  if(vme_sysreset(bus_handle)){
    perror("Error on generating sysreset.");
    result = -1;
  }

  if(vme_term(bus_handle)){
    perror("Error terminating");
    result = -1;
  }

  return result;
}

