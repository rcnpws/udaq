/* madc32_init.c ---- init MADC32                                          */
/*								           */
/*  Version 1.00        2016-03-08      by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "madc32.h"

//#define MADC32_DEFAULT_THRESHOLD  (0x30)
#define MADC32_DEFAULT_THRESHOLD  (0x00)

int main(int argc, char *argv[]){
  int ret;
  int module_number = 0;
  int ch;
 
  /* default values */
  int irq_threshold = MADC_IRQ_THRESHOLD_DEFAULT;
  int multi_event = MADC32_MULTI_EVENT_YES_UNLIMITED;
  int marking_type = MADC32_MARKING_TYPE_EVENT_COUNTER;
  int adc_resolution = MADC32_ADC_RESOLUTION_4K_HIGH_RES;          /*  3.2 micro sec */
  int delay0 = MADC32_DELAY0_25NS;  /* default = 0  = 25 nsec */
  int delay1 = MADC32_DELAY1_25NS;  /* default = 0  = 25 nsec */
  int width0 = MADC32_WIDTH0_2US;   /* default = 40 =  2 usec */
  int width1 = MADC32_WIDTH1_2US;   /* default = 40 =  2 usec */
  int use_gg = MADC32_USE_GG_GG0;   /* default =  0 = use GG0 */
  int input_range = MADC32_INPUT_RANGE_10V;    /* default = 1 = 10V */

  //  int nim_gat1_osc = MADC32_NIM_GAT1_OSC_GG1;  /* default = 0 = GG1 input */
  int nim_gat1_osc = MADC32_NIM_GAT1_OSC_TIME;  /* changed on 16/04/12 for fast clear /*

  int nim_fc_reset = MADC32_NIM_FC_RESET_FAST_CLEAR; /* default = 0 fast clear input */
  //int nim_busy = MADC32_NIM_BUSY_BUSY;  /* default = 0 = busy output */
  int nim_busy = MADC32_NIM_BUSY_DATA_THRESHOLD;  /* default = 0 = busy output */

  MADC32_p madc32=(MADC32_p)NULL;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... init MADC32.\n", argv[0]);
    fprintf(stderr, "Usage: %s [module_number]\n", argv[0]);
    fprintf(stderr, " module_number: the default is 0.\n");
    fprintf(stderr, " -hd0 delay0   0=25ns, 1=150ns, 2=200ns, n=100+50*n\n");
    fprintf(stderr, " -hd1 delay1   0=25ns, 1=150ns, 2=200ns, n=100+50*n\n");
    fprintf(stderr, " -hw0 width0   50nsec * width0\n");
    fprintf(stderr, " -hw1 width1   50nsec * width1\n");
    fprintf(stderr, " -gg  gg_sel   0=not connected, 1=GG0, 2=GG1 3=GG0&GG1\n");
    exit(0);
  }



  if(argc>1){
    module_number = atoi(argv[1]);
  }

  /* ------------- Open ------------ */

  fprintf(stderr, "MADC32 base address = 0x%.8x\n", MADC32_BASE(module_number));

  fprintf(stderr, "open MADC32\n");
  ret = madc32_open();
  if(ret!=0){
    fprintf(stderr, "Error in madc32_open()\n");
    exit(-1);
  }

  fprintf(stderr, "map MADC32(module_number=%d)\n", module_number);
  madc32 = madc32_map(module_number);
  if(madc32==(MADC32_p)NULL){
    fprintf(stderr, "Error in madc32_map()\n");
    madc32_close();
    exit(-1);
  }

  fprintf(stderr, "madc32 = 0x%.8lx\n", (long)madc32);

  /* ------------- initialize MADC32  ------------ */
  madc32->module_id = module_number;
  madc32->irq_threshold = irq_threshold;
  madc32->data_len_format = MADC32_DATA_LEN_FORMAT_32BIT;
  madc32->multi_event = multi_event;
  madc32->marking_type = marking_type;  
  madc32->bank_operation = MADC32_BANK_OPERATION_BANKS_CONNECTED;
  madc32->adc_resolution = adc_resolution;
  madc32->hold_delay0 = delay0;
  madc32->hold_delay1 = delay1;
  madc32->hold_width0 = width0;
  madc32->hold_width1 = width1;
  madc32->use_gg = use_gg;
  madc32->input_range = input_range;
  madc32->nim_gate1_osc = nim_gat1_osc;
  madc32->nim_fc_reset = nim_fc_reset;
  madc32->nim_busy = nim_busy;

  for(ch=0; ch<MADC32_NUM_CHANNELS; ch++){
    madc32->threshold[ch] = MADC32_DEFAULT_THRESHOLD;
  }
  

  printf("--------------------------------------------------------------------------\n");

  int rev;
  rev = madc32->firmware_revision;
  printf("firmware revision = 0x%.4x [0x%.4lx]\n", rev, (long)&madc32->firmware_revision - (long)madc32);

  printf("--------------------------------------------------------------------------\n");


  /* ------------- Close ------------ */
  fprintf(stderr, "unmap MADC32\n");
  ret = madc32_unmap(module_number);
  if(ret!=0){
    fprintf(stderr, "Error in madc32_unmap()\n");
  }

  fprintf(stderr, "close MADC32\n");
  ret = madc32_close();
  if(ret!=0){
    fprintf(stderr, "Error in madc32_close()\n");
    exit(-1);
  }

  return 0; 
}


