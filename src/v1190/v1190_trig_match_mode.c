/* v1190_trig_match_mode ---- set V1190A to trigger matching mode          */
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
  V1190_CR_t cr;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... set v1190 to the trigger matching mode.\n", argv[0]);
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

  fprintf(stderr, "micro code write: V1190_OPECODE_TRIG_MATCH\n");
  ret = v1190_micro_handshake_write(v1190, V1190_OPECODE_TRIG_MATCH);
  if(ret!=0){
    fprintf(stderr, "return code from v1190_micro_handshake_write: %d\n", ret);
  }

  usleep(1000000);  /* this wait is required before reading the status. */

  sr = v1190->status_register;
  printf("status register = 0x%.4x\n", sr);
  printf("  TRG MATCH: operating mode: %s\n", 
     (sr&0x0008)==0 ? "0...continuous" : "1...trigger matching");

  fprintf(stderr, "micro code write: V1190_OPECODE_EN_SUB_TRG\n");
  ret = v1190_micro_handshake_write(v1190, V1190_OPECODE_EN_SUB_TRG);
  if(ret!=0){
    fprintf(stderr, "return code from v1190_micro_handshake_write: %d\n", ret);
  }

  usleep(1000000);  /* this wait is required before reading the status. */


  
  *(unsigned short*)&cr = v1190->control_register;
  cr.extended_trigger_time_tag_enable = 1;
  v1190->control_register = *(unsigned short*)&cr;

  *(unsigned short*)&cr = v1190->control_register;
  printf("control register = 0x%.4x\n", *(unsigned short*)&cr);
  printf("control register = 0x%.4x\n", v1190->control_register);
  printf("  Extended Trigger Time Tag Enable: %s\n", cr.extended_trigger_time_tag_enable ? "enable" : "disable");

  int res;
  unsigned short udata;
  unsigned short trigger_time_subtraction;

  res = v1190_micro_handshake_read(v1190, V1190_OPECODE_READ_TRG_CONF, &udata);
  res = v1190_micro_handshake_read_cont(v1190, &udata);
  res = v1190_micro_handshake_read_cont(v1190, &udata);
  res = v1190_micro_handshake_read_cont(v1190, &udata);
  res = v1190_micro_handshake_read_cont(v1190, &trigger_time_subtraction);
  if(res){
    fprintf(stderr, "Error in v1190_micro_handshake_read_cont for READ_TRG_CONF (%d)\n", res);
  }

  printf("trigger_time_subtraction   = %.4x (%s)\n", trigger_time_subtraction, (trigger_time_subtraction&0x0001) ? "enabled":"disabled");



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


