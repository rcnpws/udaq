/* vme_sysreset ---- generate VME SYSRESET                                 */
/*			                                                   */
/*  Version 1.00        2012-12-24      by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

static vme_bus_handle_t bus_handle;

int main(int argc, char *argv[]){

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... generate VME SYSRESET.\n", argv[0]);
    fprintf(stderr, "Usage: %s \n", argv[0]);
    exit(0);
  }

  if(vme_init(&bus_handle)){
    perror("v1190_open: Error initializing the VMEbus");
    return -1;
  }

  vme_sysreset(bus_handle);
  return 0;
}
