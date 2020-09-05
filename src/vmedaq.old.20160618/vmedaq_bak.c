/* vmedaq.c ---- vme DAQ with V1190                                         */
/*								            */
/*  Version 1.00        2013-09-05      by A. Tamii (For Linux2.6)GE-FANUC  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1190.h"
#include "v977.h"

#define DEBUG 0   /* 1 = debug mode */

int geometrical_address = -1;

#define MAX_DUMP_BUF 0x1000
unsigned long dump_buffer[MAX_DUMP_BUF];

#define V977_MODULE_NUMBER 0
static V977_p v977=(V977_p)NULL;

#define IOREG_CH_DAQ_INHIBIT     0
#define IOREG_CH_BLOCK_TRANSFER  1
#define IOREG_CH_ALMOST_FULL     2


int n_modules = 0;
long data_size = 0;
int module_numbers[V1190_MAX_N_MODULES];
V1190_p v1190[V1190_MAX_N_MODULES];

static int v977_init(){
  if(v977_open()){
    fprintf(stderr, "Error in v977_open()\n");
    return -1;
  }
  v977 = v977_map(V977_MODULE_NUMBER);
  if(v977==(V977_p)NULL){
    fprintf(stderr, "Error in v977_map()\n");
    return -1;
  }

  v977->control_register = 0x0002;  /* default setting */
  v977->input_set = 0;
  v977->input_mask = 0;
  v977->output_mask = 0;
  v977->clear_output = 0;
  v977->output_set = 1<<IOREG_CH_DAQ_INHIBIT;
  v977->test_control_register = 0;

  data_size = 0;
  
  return 0;
}

static int v977_exit(){
  if(v977_unmap(V977_MODULE_NUMBER)){
    fprintf(stderr, "Error in v977_unmap()\n");
    return -1;
  }
  if(v977_close()){
    fprintf(stderr, "Error in v977_close()\n");
    return -1;
  }
  return 0;
}

long check_data(V1190_p v1190, FILE *fd){
  V1190_DATA_t   data;
  unsigned long *output_buffer;
  unsigned long ldata;
  long event_count = -1;
  int end_flag = 0;
#define BUF_SIZE 16384
  unsigned long buffer[BUF_SIZE];

  output_buffer = v1190->output_buffer;

  while(!end_flag){
    ldata = *output_buffer;
    data = *(V1190_DATA_t*)&ldata;
    //memcpy(buffer, output_buffer, BUF_SIZE*sizeof(unsigned long));

    switch(data.global_header.id){
    case V1190_HEADER_ID_FILLER:
      end_flag = 1;
      break;
    case V1190_HEADER_ID_GH:
      event_count = data.global_header.event_count;
      if(event_count<=100 || (event_count<=100000 && (event_count%1000)==0) || (event_count%100000)==0){
         fprintf(stderr, " event count = %ld, data size = %ld\n", event_count, data_size);
      }
    default:
#if 1
      fwrite(&ldata, sizeof(unsigned long), 1, fd);
#endif
      data_size += sizeof(unsigned long);
      break;
    }
  }
  fflush(fd);
  return event_count;
}

int main(int argc, char *argv[]){
  FILE *fd = stdout;
  long count;
  int i;
  int val;
  int init_flag = 0;
  int n_data_modules;

  int t_clock = 25;
  //unsigned short match_window_width = 1000/t_clock;     // from 1 to 4095
  unsigned short match_window_width = 4000/t_clock;     // from 1 to 4095
  //short window_offset = -100/t_clock;                   // from -2048 to 40
  short window_offset = -2000/t_clock;                   // from -2048 to 40
  unsigned short extra_search_window_width = 0/t_clock; // no greater than 50
  unsigned short reject_margin = 100/t_clock;           // should be >= 1


  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stdout, "%s ... read v1190 TDC and output to stdout.\n", argv[0]);
    fprintf(stdout, "Usage: %s [-h] [-i] [module_numbers...] file_name \n", argv[0]);
    fprintf(stdout, " -h: show this help.\n");
    fprintf(stdout, " -i: initialize V1190 TDCs\n");
    fprintf(stdout, " module_number: the default is 0.\n");
    exit(0);
  }

  argc--;
  argv++;

  while(argc>0){
    if(!strcmp(argv[0],"-i")){
      init_flag = 1;
    }else if('0'<=argv[0][0] && argv[0][0]<='9'){
      module_numbers[n_modules] = atoi(argv[0]);
      n_modules++;
    }else{
      fd = fopen(argv[0],"w");
     if(fd==(FILE*)NULL){
        fprintf(stderr, "Could not open file '%s'.\n", argv[0]);
        exit(-1);
      }
      break;
    }
    argc--;
    argv++;
  }

  /* ------------- Open ------------ */

  if(v1190_open()){
    fprintf(stderr, "Error in v1190_open()\n");
    exit(-1);
  }

  if(v977_init()){
    fprintf(stderr, "Error in v1190_init()\n");
    exit(-1);
  }

  for(i=0; i<n_modules; i++){
    fprintf(stderr, "Opening module: %d\n", module_numbers[i]);
    v1190[i] = v1190_map(module_numbers[i]);
    if(v1190[i]==(V1190_p)NULL){
      fprintf(stderr, "Error in v1190_map()\n");
      v1190_close();
      exit(-1);
    }
  }


  /* ------------- Initialization ------------ */

  for(i=0; i<n_modules; i++){
    v1190[i]->software_clear = 1;   /* software clear, clear the event counter */

    if(init_flag){
      fprintf(stderr, "Initializing module: %d\n", module_numbers[i]);
      v1190[i]->output_prog_control = 2; /* select Almost Full for output */
      v1190[i]->almost_full_level = 16384; /* 16384 words = a half of the memory */
      v1190[i]->geo_address_register = module_numbers[i];
      v1190_micro_handshake_write(v1190[i], V1190_OPECODE_TRIG_MATCH);
      usleep(700000);
      
      v1190_micro_handshake_write(v1190[i], V1190_OPECODE_SET_WIN_WIDTH);
      usleep(700000);
      v1190_micro_handshake_write(v1190[i], match_window_width);
      usleep(700000);
      
      v1190_micro_handshake_write(v1190[i], V1190_OPECODE_SET_WIN_OFFS);
      usleep(700000);
      v1190_micro_handshake_write(v1190[i], window_offset);
      usleep(700000);
      
      v1190_micro_handshake_write(v1190[i], V1190_OPECODE_SET_SW_MARGIN);
      usleep(700000);
      v1190_micro_handshake_write(v1190[i], extra_search_window_width);
      usleep(700000);
      
      v1190_micro_handshake_write(v1190[i], V1190_OPECODE_SET_REJ_MARGIN);
      usleep(700000);
      v1190_micro_handshake_write(v1190[i], reject_margin);
      usleep(700000);
      fprintf(stderr, "done\n");
    }
  }

  /* ------------- Start DAQ  ------------ */
  v977->output_set = 0;
  while(1){
    n_data_modules = 0;
    for(i=0; i<n_modules; i++){
      count = check_data(v1190[i], fd);
      if(count>=0){
        n_data_modules++;
#if 0
        if(count<=1000 && count%100==0)
          fprintf(stderr, "module: %2d, count = %12ld\n", module_numbers[i], count);
        else if(count<=10000 && count%1000==0)
          fprintf(stderr, "module: %2d, count = %12ld\n", module_numbers[i], count);
        else if(count%10000==0)
          fprintf(stderr, "module: %2d, count = %12ld\n", module_numbers[i], count);
#endif
      }
    }
    if(n_data_modules==0){
      //fprintf(stderr, "idling\n");
      //v977->output_set = 2;
      val=v977->singlehit_read_clear;
      //usleep(1000);
      //v977->output_set = 0;
    }
  }

  /* ------------- Close ------------ */
  for(i=0; i<n_modules; i++){
    if(v1190_unmap(module_numbers[i])){
      fprintf(stderr, "Error in v1190_unmap()\n");
    }
  }

  if(v977_exit()){
    fprintf(stderr, "Error in v1190_exit()\n");
  }

  if(v1190_close()){
    fprintf(stderr, "Error in v1190_close()\n");
  }

  return 0; 
}

