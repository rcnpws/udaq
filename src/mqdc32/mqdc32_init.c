/* mqdc32_init.c ---- init MQDC32                                          */
/*								           */
/*  Version 0.10        2020-09-04      by T. Furuno (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "mqdc32.h"

#define MQDC32_DEFAULT_THRESHOLD  (0x00)

int main(int argc, char *argv[]){
  int ret;
  int module_number = 0;
  int ch;
 
  /* default values */
  int irq_threshold = MQDC32_IRQ_THRESHOLD_DEFAULT;
  int multi_event = MQDC32_MULTI_EVENT_YES_UNLIMITED;
  //  int marking_type = MQDC32_MARKING_TYPE_EVENT_COUNTER;
  int marking_type = MQDC32_MARKING_TYPE_TIME_STAMP; /* changed on 16/04/12 for fast clear */
  int input_coupling = MQDC32_INPUT_COUPLING_B0AC_B1AC;
  int nim_gat1_osc = MQDC32_NIM_GAT1_OSC_TIME;  /* changed on 16/04/12 for fast clear */
  int nim_busy = MQDC32_NIM_BUSY_DATA_THRESHOLD;  /* default = 0 = busy output */
  int exp_trig_delay = 0;


  MQDC32_p mqdc32=(MQDC32_p)NULL;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... init MQDC32.\n", argv[0]);
    fprintf(stderr, "Usage: %s [module_number]\n", argv[0]);
    fprintf(stderr, " module_number: the default is 0.\n");
    exit(0);
  }

  if(argc>1){
    module_number = atoi(argv[1]);
  }

  /* ------------- Open ------------ */

  fprintf(stderr, "MQDC32 base address = 0x%.8x\n", MQDC32_BASE(module_number));

  fprintf(stderr, "open MQDC32\n");
  ret = mqdc32_open();
  if(ret!=0){
    fprintf(stderr, "Error in mqdc32_open()\n");
    exit(-1);
  }

  fprintf(stderr, "map MQDC32(module_number=%d)\n", module_number);
  mqdc32 = mqdc32_map(module_number);
  if(mqdc32==(MQDC32_p)NULL){
    fprintf(stderr, "Error in mqdc32_map()\n");
    mqdc32_close();
    exit(-1);
  }

  fprintf(stderr, "mqdc32 = 0x%.8lx\n", (long)mqdc32);

  /* ------------- initialize MQDC32  ------------ */
  mqdc32->module_id = module_number;
  mqdc32->irq_threshold = irq_threshold;
  mqdc32->data_len_format = MQDC32_DATA_LEN_FORMAT_32BIT;
  mqdc32->multi_event = multi_event;
  mqdc32->marking_type = marking_type;  
  mqdc32->bank_operation = MQDC32_BANK_OPERATION_BANKS_CONNECTED;

  mqdc32->exp_trig_delay0 = exp_trig_delay;

  mqdc32->input_coupling = input_coupling;
  mqdc32->nim_gate1_osc = nim_gat1_osc;
  mqdc32->nim_busy = nim_busy;
  mqdc32->ts_sources = MQDC32_TS_SOURCES_EXT;


  for(ch=0; ch<MQDC32_NUM_CHANNELS; ch++){
    mqdc32->threshold[ch] = MQDC32_DEFAULT_THRESHOLD;
  }
  

  printf("--------------------------------------------------------------------------\n");

  int rev;
  rev = mqdc32->firmware_revision;
  printf("firmware revision = 0x%.4x [0x%.4lx]\n", rev, (long)&mqdc32->firmware_revision - (long)mqdc32);

  printf("--------------------------------------------------------------------------\n");


  /* ------------- Close ------------ */
  fprintf(stderr, "unmap MQDC32\n");
  ret = mqdc32_unmap(module_number);
  if(ret!=0){
    fprintf(stderr, "Error in mqdc32_unmap()\n");
  }

  fprintf(stderr, "close MQDC32\n");
  ret = mqdc32_close();
  if(ret!=0){
    fprintf(stderr, "Error in mqdc32_close()\n");
    exit(-1);
  }

  return 0; 
}


