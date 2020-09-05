/* v1190_read.c ---- read V1190 TDC data and output to stdout               */
/*								            */
/*  Version 1.00        2013-02-05      by A. Tamii (For Linux2.6)GE-FANUC  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1190.h"

#define DEBUG 0   /* 1 = debug mode */

int geometrical_address = -1;

#define MAX_DUMP_BUF 0x1000
unsigned long dump_buffer[MAX_DUMP_BUF];


int n_modules = 0;
int module_numbers[V1190_MAX_N_MODULES];
V1190_p v1190[V1190_MAX_N_MODULES];

long check_data(V1190_p v1190, FILE *fd){
  V1190_DATA_t   data;
  unsigned long *output_buffer;
  unsigned long ldata;
  long event_count = -1;

  output_buffer = v1190->output_buffer;

  while(1){
    ldata = *output_buffer;
    data = *(V1190_DATA_t*)&ldata;

    switch(data.global_header.id){
    case V1190_HEADER_ID_FILLER:
      return event_count;
    case V1190_HEADER_ID_GH:
      event_count = data.global_header.event_count;
      if(event_count<=100 || (event_count<=100000 && (event_count%1000)==0) || (event_count%100000)==0)
         fprintf(stderr, " event count = %ld\n", event_count);
    default:
      fwrite(&ldata, sizeof(unsigned long), 1, fd);
      fflush(fd);
      break;
    }
  }
}

int main(int argc, char *argv[]){
  FILE *fd = stdout;
  long count;
  int i;

  int t_clock = 25;
  //unsigned short match_window_width = 1000/t_clock;     // from 1 to 4095
  unsigned short match_window_width = 4000/t_clock;     // from 1 to 4095
  //short window_offset = -100/t_clock;                   // from -2048 to 40
  short window_offset = -2000/t_clock;                   // from -2048 to 40
  unsigned short extra_search_window_width = 0/t_clock; // no greater than 50
  unsigned short reject_margin = 100/t_clock;           // should be >= 1


  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stdout, "%s ... read v1190 TDC and output to stdout.\n", argv[0]);
    fprintf(stdout, "Usage: %s [-h] [module_numbers...] file_name \n", argv[0]);
    fprintf(stdout, " module_number: the default is 0.\n");
    fprintf(stdout, " -h: show this help.\n");
    exit(0);
  }

  argc--;
  argv++;

  while(argc>0){
    if('0'<=argv[0][0] && argv[0][0]<='9'){
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

  for(i=0; i<n_modules; i++){
    fprintf(stderr, "Opening module: %d\n", module_numbers[i]);
    v1190[i] = v1190_map(module_numbers[i]);
    if(v1190[i]==(V1190_p)NULL){
      fprintf(stderr, "Error in v1190_map()\n");
      v1190_close();
      exit(-1);
    }
  }


  /* ------------- Do my task ------------ */

  for(i=0; i<n_modules; i++){
    fprintf(stderr, "Initializing module: %d\n", module_numbers[i]);
    v1190[i]->software_clear = 1;   /* software clear, clear the event counter */
    v1190[i]->output_prog_control = 2; /* select Almost Full for output */
    v1190[i]->almost_full_level = 16384; /* 16384 words = a half of the memory */
    v1190[i]->geo_address_register = module_numbers[i];
#if 1
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
#endif
    fprintf(stderr, "done\n");
  }

  while(1){
    for(i=0; i<n_modules; i++){
      count = check_data(v1190[i], fd);
      if(count>=0)
        fprintf(stderr, "module: %2d, count = %12ld\n", module_numbers[i], count);
    }
  }

  /* ------------- Close ------------ */
  for(i=0; i<n_modules; i++){
    if(v1190_unmap(module_numbers[i])){
      fprintf(stderr, "Error in v1190_unmap()\n");
    }
  }

  if(v1190_close()){
    fprintf(stderr, "Error in v1190_close()\n");
  }

  return 0; 
}

