/* v1190_status.c ---- show V1190 status                                   */
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
  int ret;
  unsigned short cr;
  unsigned short sr;
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
  printf("--------------------------------------------------------------------------\n");

  cr = v1190->control_register;
  printf("control register = 0x%.4x\n", cr);

  printf("  BERR EN  		: Bus Error Enable			: %s\n", 
    (cr&0x0001)==0 ? "0...Disabled" : "1...Enabled");
  printf("  TERM     		: Software Termination			: %s\n", 
    (cr&0x0002)==0 ? "0...Off" : "1...On");
  printf("  TERM SW    		: Software Termination Mode		: %s\n", 
    (cr&0x0004)==0 ? "0...Dip_switch" : "1...Software");
  printf("  EMPTY EVENT		: If No Data, How is GH and GT		: %s\n", 
    (cr&0x0008)==0 ? "0...Not Written" : "1...Written GH and GT");
  printf("  ALIGN 64    		: Add A 32 Bit Dummy_Word		: %s\n", 
    (cr&0x0010)==0 ? "0...No Added" : "1...Added");
  printf("  COMPENSATION		: Compensation Of The INL		: %s\n", 
    (cr&0x0020)==0 ? "0...Disabled" : "1...Enabled");
  printf("  TEST FIFO		: Buffer Test Mode			: %s\n", 
    (cr&0x0040)==0 ? "0...Disabled" : "1...Enabled");
  printf("  COMPENSITION SRAM	: Sram In Compensation Table		: %s\n", 
    (cr&0x0080)==0 ? "0...Can Be Readout" : "1...Can't Be Readout");
  printf("  EVENT FIFO		: Event Fifo Enable			: %s\n", 
    (cr&0x0100)==0 ? "0...Disabled" : "1...Enabled");
  printf("  TRIGGER TIME TAG	: Extentend Trigger Time Tag Enable	: %s\n", 
    (cr&0x0200)==0 ? "0...Disabled" : "1...Enabled");
  
  printf("\n");

  sr = v1190->status_register;
  printf("status register = 0x%.4x\n", sr);
  printf("  DREADY		: Data In Output Buffer			: %s\n", 
     (sr&0x0001)==0 ? "0...No Data Ready" : "1...Event Ready and Data Ready");
  printf("  ALMOST FULL		: Almost Full Level In Output Buffer	: %s\n", 
     (sr&0x0002)==0 ? "0...Not Full" : "1... Almost Full");
  printf("  FULL			: Output Buffer				: %s\n", 
     (sr&0x0004)==0 ? "0...Not Full" : "1...Full");
  printf("  TRG MATCH		: Operating Mode			: %s\n", 
     (sr&0x0008)==0 ? "0...Continuous" : "1...Trigger Matching");
  printf("  HEADER EN		: TDC's Header and TRAILER		: %s\n", 
     (sr&0x0010)==0 ? "0...Disabled" : "1...Enabled");
  printf("  TERMINATION		: On/Off				: %s\n", 
     (sr&0x0020)==0 ? "0...Off" : "1...On");
  printf("  ERROR TDC0		: Check Error In The TDC0		: %s\n", 
     (sr&0x0040)==0 ? "0...Operated properly" : "1...Error Occurred");
  printf("  ERROR TDC1		: Check Error In The TDC1		: %s\n", 
     (sr&0x0080)==0 ? "0...Operated properly" : "1...Error Occurred");
  printf("  ERROR TDC2		: Check Error In The TDC2		: %s\n", 
     (sr&0x0100)==0 ? "0...Operated properly" : "1...Error Occurred");
  printf("  ERROR TDC3		: Check Error In The TDC3		: %s\n", 
     (sr&0x0200)==0 ? "0...Operated properly" : "1...Error Occurred");   
  printf("  BERR FLAG		: Check Error In The Bus		: %s\n", 
     (sr&0x0400)==0 ? "0...Operated properly" : "1...Error Occurred");
  printf("  PURGED		: Check Board				: %s\n", 
     (sr&0x0800)==0 ? "0...Not Purged" : "1...Purged");
  
  if((sr&0x1000)==0){	  
     printf("  RESOLUTION		: Time Resolution			: %s\n",
     (sr&0x2000)==0 ? "0...800ps" : "1...100ps");
  }
  else if((sr&0x1000)==1){	  
     printf("  RESOLUTION		: Time Resolution			: %s\n",
     (sr&0x2000)==0 ? "0...200ps" : "1...resolution error");
  }

  printf("  PAIR MODE		: Module In Pair Mode			: %s\n", 
     (sr&0x4000)==0 ? "0...Not In Pair" : "1...In Pair");
  printf("  TRIGGER LOST		: Check Sending TDC Trigger		: %s\n", 
     (sr&0x8000)==0 ? "0...OK " : "1...A Trigger At Least Lost");
  
  
  printf("--------------------------------------------------------------------------\n");

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


