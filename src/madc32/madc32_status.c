/* madc32_status.c ---- show MADC32 status                                 */
/*								           */
/*  Version 1.00        2016-03-08      by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>
#include <math.h>

#include "madc32.h"

int main(int argc, char *argv[]){
  int ret;
  int module_number = 0;
  int ch;
  MADC32_p madc32=(MADC32_p)NULL;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... show madc32 status.\n", argv[0]);
    fprintf(stderr, "Usage: %s [module_number]\n", argv[0]);
    fprintf(stderr, " module_number: the default is 0.\n");
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

  /* ------------- Show status ------------ */
  int i;

  printf("\n");
  printf("--------------------------------------------------------------------------\n");

  /* module id */
  int module_id = madc32->module_id;
  printf("module id = %d [0x%.4lx]\n", 
	 module_id, (long)&madc32->module_id - (long)madc32);

  /* firmware revision */
  int rev = madc32->firmware_revision;
  printf("firmware revision = 0x%.4x [0x%.4lx]\n", 
	 rev, (long)&madc32->firmware_revision - (long)madc32);


  /* IRQ level */
  int irq_level = madc32->irq_level;
  printf("IRQ level = %d (0=off) [0x%.4lx]\n", 
	 irq_level, (long)&madc32->irq_level - (long)madc32);

  /* IRQ threshold */
  int irq_threshold = madc32->irq_threshold;
  if(irq_level>0){
    printf("IRQ threshold = 0x%.4x [0x%.4lx]\n",
	   irq_threshold, (long)&madc32->irq_threshold - (long)madc32);
  }


  /* data length format */
  int data_len_format = madc32->data_len_format;
  printf("data length format = %d bit [0x%.4lx]\n",
	 (int)(8*pow(2.0, (double)data_len_format)), 
	 (long)&madc32->data_len_format - (long)madc32);
  
  /* multi event */
  int multi_event = madc32->multi_event;
  switch (multi_event){
  case 0:
    printf("NOT multi event mode [0x%.4lx]\n",
	   (long)&madc32->multi_event - (long)madc32);    
    break;
  case 1:
    printf("multi event (unlimited transfer) [0x%.4lx]\n",
	   (long)&madc32->multi_event - (long)madc32);
    break;
  case 3:
    printf("multi event (transfer limited amount of data) [0x%.4lx]\n",
	   (long)&madc32->multi_event - (long)madc32);
    break;
  default:
    printf("ERROR of the multi event value = %d [0x%.4lx]\n",
	   multi_event, (long)&madc32->multi_event - (long)madc32);
    break;
  }

  /* marking type */
  int marking_type = madc32->marking_type;
  switch (marking_type){
  case 0:
    printf("marking type: event counter [0x%.4lx]\n",
	   (long)&madc32->marking_type - (long)madc32);
    break;
  case 1:
    printf("marking type: time stamp [0x%.4lx]\n",
	   (long)&madc32->marking_type - (long)madc32);
    break;
  case 2:
    printf("marking type: extended time stamp [0x%.4lx]\n",
	   (long)&madc32->marking_type - (long)madc32);
    break;
  default:
    printf("ERROR of the marking type value = %d [0x%.4lx]\n",
	   marking_type, (long)&madc32->marking_type - (long)madc32);
    break;
  }

  /* bank operation */
  int bank_operation = madc32->bank_operation;
  switch (bank_operation){
  case 0:
    printf("banks are connected [0x%.4lx]\n",
	   (long)&madc32->bank_operation - (long)madc32);
    break;
  case 1:
    printf("operate banks independent [0x%.4lx]\n",
	   (long)&madc32->bank_operation - (long)madc32);
    break;
  case 3:
    printf("toggle mode for zero dead time [0x%.4lx]\n",
	   (long)&madc32->bank_operation - (long)madc32);
    break;
  default:
    printf("ERROR of the bank operation value = %d [0x%.4lx]\n",
	   bank_operation, (long)&madc32->bank_operation - (long)madc32);
    break;
  }

  /* ADC resolution */
  int adc_resolution = madc32->adc_resolution;
  switch (adc_resolution){
  case 0:
    printf("ADC resolution = 2k (800 ns conversion) [0x%.4lx]\n",
	   (long)&madc32->adc_resolution - (long)madc32);
    break;
  case 1:
    printf("ADC resolution = 4k (1.6 us conversion) [0x%.4lx]\n",
	   (long)&madc32->adc_resolution - (long)madc32);
    break;
  case 2:
    printf("ADC resolution = 4k high-res (3.2 conversion) [0x%.4lx]\n",
	   (long)&madc32->adc_resolution - (long)madc32);
    break;
  case 3:
    printf("ADC resolution = 8k (6.4 us conversion) [0x%.4lx]\n",
	   (long)&madc32->adc_resolution - (long)madc32);
    break;
  case 4:
    printf("ADC resolution = 8k high-res (12.5 us conversion) [0x%.4lx]\n",
	   (long)&madc32->adc_resolution - (long)madc32);
    break;
  default:
    printf("ERROR of the ADC resolution value = %d [0x%.4lx]\n",
	   adc_resolution, (long)&madc32->adc_resolution - (long)madc32);
    break;
  }

  /* delay0 & delay1 */
  int hold_delay[2], delay[2];
  hold_delay[0] = madc32->hold_delay0;
  hold_delay[1] = madc32->hold_delay1;

  for(i=0; i<2; i++){
    if(hold_delay[i]==0){
      delay[i]=25;
    }
    if(hold_delay[i]>0){
      delay[i]=150+50*(i-1);
    }
  }

  printf("delay0 =%d ns , delay1 =%d ns [0x%.4lx] [0x%.4lx]\n",
	 delay[0], delay[1],
	 (long)&madc32->hold_delay0 - (long)madc32,
	 (long)&madc32->hold_delay1 - (long)madc32);

  /* width0 & width1 */
  int hold_width[2], width[2];
  hold_width[0] = madc32->hold_width0;
  hold_width[1] = madc32->hold_width1;
  for(i=0; i<2; i++){
    width[i] = hold_width[i]*50;
  }
  printf("width0 =%d ns, width1 =%d ns [0x%.4lx] [0x%.4lx]\n",
	 width[0], width[1],
	 (long)&madc32->hold_width0 - (long)madc32,
	 (long)&madc32->hold_width1 - (long)madc32);
  
  /* input range */
  int input_range;
  input_range = madc32->input_range;
  
  switch (input_range){
  case 0:
    printf("input range: 4 V [0x%.4lx]\n",
	   (long)&madc32->hold_width0 - (long)madc32);
    break;
  case 1:
    printf("input range: 10 V [0x%.4lx]\n",
	   (long)&madc32->hold_width0 - (long)madc32);
    break;
  case 2:
    printf("input range: 8 V [0x%.4lx]\n",
	   (long)&madc32->hold_width0 - (long)madc32);
    break;
  default:
    printf("ERROR of the input range value = %d [0x%.4lx]\n",
	   input_range, (long)&madc32->input_range - (long)madc32);
    break;
  }

  /* NIM gate1 oscillator */
  int nim_gat1_osc = madc32->nim_gate1_osc;
  switch (nim_gat1_osc){
  case 0:
    printf("NIM gate1: gate1 input [0x%.4lx]\n",
	   (long)&madc32->nim_gate1_osc - (long)madc32);
    break;
  case 1:
    printf("NIM gate1: Oscillator input [0x%.4lx]\n",
	   (long)&madc32->nim_gate1_osc - (long)madc32);
    break;
  default:
    printf("ERROR of the nim gate1 osc value = %d [0x%.4lx]\n",
	   nim_gat1_osc, (long)&madc32->nim_gate1_osc - (long)madc32);
    break;
  }

  /* NIM fast clear reset */
  int nim_fc_reset = madc32->nim_fc_reset;
  switch (nim_fc_reset){
  case 0:
    printf("NIM FC/Res: fast clear input [0x%.4lx]\n",
	   (long)&madc32->nim_fc_reset - (long)madc32);
    break;
  case 1:
    printf("NIM FC/Res: reset time stamp oscillator [0x%.4lx]\n",
	   (long)&madc32->nim_fc_reset - (long)madc32);
    break;
  default:
    printf("ERROR of the nim fc reset value = %d [0x%.4lx]\n",
	   nim_fc_reset, (long)&madc32->nim_fc_reset - (long)madc32);
    break;
  }

  /* NIM busy */
  int nim_busy = madc32->nim_busy;
  switch (nim_busy){
  case 0:
    printf("NIM busy: busy output [0x%.4lx]\n",
	   (long)&madc32->nim_busy - (long)madc32);
    break;
  case 1:
    printf("NIM busy: gate0 output [0x%.4lx]\n",
	   (long)&madc32->nim_busy - (long)madc32);
    break;
  case 2:
    printf("NIM busy: gate1 output [0x%.4lx]\n",
	   (long)&madc32->nim_busy - (long)madc32);
    break;
  case 3:
    printf("NIM busy: Cbus output [0x%.4lx]\n",
	   (long)&madc32->nim_busy - (long)madc32);
    break;
  case 4:
    printf("NIM busy: buffer full output [0x%.4lx]\n",
	   (long)&madc32->nim_busy - (long)madc32);
    break;
  case 8:
    printf("NIM busy: data in buffer above threshold [0x%.4lx]\n",
	   (long)&madc32->nim_busy - (long)madc32);
    printf("          the threshold = %d (0x%x)\n",
	  madc32->irq_threshold, madc32->irq_threshold);
    break;
  default:
    printf("ERROR of the nim busy value = %d [0x%.4lx]\n",
	   nim_busy, (long)&madc32->nim_busy - (long)madc32);
    break;
  }

  for(ch=0; ch<MADC32_NUM_CHANNELS; ch++){
    printf("threshold[%2d] = %4d\n", ch, madc32->threshold[ch]);
  }

  /* */

  printf("--------------------------------------------------------------------------\n");
  printf("\n");

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


