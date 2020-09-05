/* mqdc32_status.c ---- show MQDC32 status                                 */
/*								           */
/*  Version 0.10        2020-09-05      by T. Furuno (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>
#include <math.h>

#include "mqdc32.h"

int main(int argc, char *argv[]){
  int ret;
  int module_number = 0;
  int ch;
  MQDC32_p mqdc32=(MQDC32_p)NULL;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... show mqdc32 status.\n", argv[0]);
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

  /* ------------- Show status ------------ */
  printf("\n");
  printf("--------------------------------------------------------------------------\n");

  /* module id */
  int module_id = mqdc32->module_id;
  printf("module id = %d [0x%.4lx]\n", 
	 module_id, (long)&mqdc32->module_id - (long)mqdc32);

  /* firmware revision */
  int rev = mqdc32->firmware_revision;
  printf("firmware revision = 0x%.4x [0x%.4lx]\n", 
	 rev, (long)&mqdc32->firmware_revision - (long)mqdc32);


  /* IRQ level */
  int irq_level = mqdc32->irq_level;
  printf("IRQ level = %d (0=off) [0x%.4lx]\n", 
	 irq_level, (long)&mqdc32->irq_level - (long)mqdc32);

  /* IRQ threshold */
  int irq_threshold = mqdc32->irq_threshold;
  if(irq_level>0){
    printf("IRQ threshold = 0x%.4x [0x%.4lx]\n",
	   irq_threshold, (long)&mqdc32->irq_threshold - (long)mqdc32);
  }

  /* data length format */
  int data_len_format = mqdc32->data_len_format;
  printf("data length format = %d bit [0x%.4lx]\n",
	 (int)(8*pow(2.0, (double)data_len_format)), 
	 (long)&mqdc32->data_len_format - (long)mqdc32);
  
  /* multi event */
  int multi_event = mqdc32->multi_event;
  switch (multi_event){
  case 0:
    printf("NOT multi event mode [0x%.4lx]\n",
	   (long)&mqdc32->multi_event - (long)mqdc32);    
    break;
  case 1:
    printf("multi event (unlimited transfer) [0x%.4lx]\n",
	   (long)&mqdc32->multi_event - (long)mqdc32);
    break;
  case 3:
    printf("multi event (transfer limited amount of data) [0x%.4lx]\n",
	   (long)&mqdc32->multi_event - (long)mqdc32);
    break;
  default:
    printf("ERROR of the multi event value = %d [0x%.4lx]\n",
	   multi_event, (long)&mqdc32->multi_event - (long)mqdc32);
    break;
  }

  /* marking type */
  int marking_type = mqdc32->marking_type;
  switch (marking_type){
  case 0:
    printf("marking type: event counter [0x%.4lx]\n",
	   (long)&mqdc32->marking_type - (long)mqdc32);
    break;
  case 1:
    printf("marking type: time stamp [0x%.4lx]\n",
	   (long)&mqdc32->marking_type - (long)mqdc32);
    break;
  case 2:
    printf("marking type: extended time stamp [0x%.4lx]\n",
	   (long)&mqdc32->marking_type - (long)mqdc32);
    break;
  default:
    printf("ERROR of the marking type value = %d [0x%.4lx]\n",
	   marking_type, (long)&mqdc32->marking_type - (long)mqdc32);
    break;
  }

  /* bank operation */
  int bank_operation = mqdc32->bank_operation;
  switch (bank_operation){
  case 0:
    printf("banks are connected [0x%.4lx]\n",
	   (long)&mqdc32->bank_operation - (long)mqdc32);
    break;
  case 1:
    printf("operate banks independent [0x%.4lx]\n",
	   (long)&mqdc32->bank_operation - (long)mqdc32);
    break;
  }
  
  /* experimental trigger */
  int exp_trig0 = mqdc32->exp_trig_delay0;
  printf("experimental trigger delay bank0 = %d [0x%.4lx]\n",
	 exp_trig0, (long)&mqdc32->exp_trig_delay0 - (long)mqdc32);

  /* input coupling */
  int input_coupling;
  input_coupling = mqdc32->input_coupling;
  
  switch (input_coupling){
  case 0:
    printf("input coupling: bank0=AC, bank1=AC [0x%.4lx]\n",
	   (long)&mqdc32->input_coupling - (long)mqdc32);
    break;
  case 1:
    printf("input coupling: bank0=DC, bank1=AC [0x%.4lx]\n",
	   (long)&mqdc32->input_coupling - (long)mqdc32);
    break;
  case 2:
    printf("input coupling: bank0=AC, bank1=DC [0x%.4lx]\n",
	   (long)&mqdc32->input_coupling - (long)mqdc32);
    break;
  case 3:
    printf("input coupling: bank0=DC, bank1=DC [0x%.4lx]\n",
	   (long)&mqdc32->input_coupling - (long)mqdc32);
    break;
  }

  /* NIM gate1(NIM CH2) oscillator */
  int nim_gat1_osc = mqdc32->nim_gate1_osc;
  switch (nim_gat1_osc){
  case 0:
    printf("NIM gate1(NIM CH2): gate1 input [0x%.4lx]\n",
	   (long)&mqdc32->nim_gate1_osc - (long)mqdc32);
    break;
  case 1:
    printf("NIM gate1(NIM CH2): Oscillator input [0x%.4lx]\n",
	   (long)&mqdc32->nim_gate1_osc - (long)mqdc32);
    break;
  default:
    printf("ERROR of the nim gate1(NIM CH2): Osc value = %d [0x%.4lx]\n",
	   nim_gat1_osc, (long)&mqdc32->nim_gate1_osc - (long)mqdc32);
    break;
  }

  /* NIM fast clear reset */
  int nim_fc_reset = mqdc32->nim_fc_reset;
  switch (nim_fc_reset){
  case 0:
    printf("NIM FC/Res(NIM CH1): fast clear input [0x%.4lx]\n",
	   (long)&mqdc32->nim_fc_reset - (long)mqdc32);
    break;
  case 1:
    printf("NIM FC/Res(NIM CH1): reset time stamp counter [0x%.4lx]\n",
	   (long)&mqdc32->nim_fc_reset - (long)mqdc32);
    break;
  case 2:
    printf("NIM FC/Res(NIM CH1): experiment trigger input [0x%.4lx]\n",
	   (long)&mqdc32->nim_fc_reset - (long)mqdc32);
    break;
  default:
    printf("ERROR of the nim fc reset value = %d [0x%.4lx]\n",
	   nim_fc_reset, (long)&mqdc32->nim_fc_reset - (long)mqdc32);
    break;
  }

  /* NIM busy */
  int nim_busy = mqdc32->nim_busy;
  switch (nim_busy){
  case 0:
    printf("NIM busy(NIM CH0): busy output [0x%.4lx]\n",
	   (long)&mqdc32->nim_busy - (long)mqdc32);
    break;
  case 3:
    printf("NIM busy(NIM CH0): Cbus output [0x%.4lx]\n",
	   (long)&mqdc32->nim_busy - (long)mqdc32);
    break;
  case 4:
    printf("NIM busy(NIM CH0): buffer full [0x%.4lx]\n",
	   (long)&mqdc32->nim_busy - (long)mqdc32);
    break;
  case 8:
    printf("NIM busy(NIM CH0): data in buffer above threshold [0x%.4lx]\n",
	   (long)&mqdc32->nim_busy - (long)mqdc32);
    break;
  case 9:
    printf("NIM busy(NIM CH0): events in buffer above threshold [0x%.4lx]\n",
	   (long)&mqdc32->nim_busy - (long)mqdc32);
    break;
  default:
    printf("ERROR of the nim busy value = %d [0x%.4lx]\n",
	   nim_busy, (long)&mqdc32->nim_busy - (long)mqdc32);
    break;
  }

  for(ch=0; ch<MQDC32_NUM_CHANNELS; ch++){
    printf("threshold[%2d] = %4d\n", ch, mqdc32->threshold[ch]);
  }

  /* */

  printf("--------------------------------------------------------------------------\n");
  printf("\n");

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


