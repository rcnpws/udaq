/* madc32_dump_data.c ---- dump MADC32 TDC data                              */
/*								           */
/*  Version 1.00        2016-03-08      by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>

#include <vme/vme.h>
#include <vme/vme_api.h>

#include "madc32.h"

int prev_time_tag = -1;

int show_data(FILE *fd, unsigned int data){
  MADC32_HEADER_SIGNATURE_p    signature;
  MADC32_DATA_HEADER_p         header;
  MADC32_DATA_EVENT_p          event;
  MADC32_EXTENDED_TIME_STAMP_p extended_time_stamp;
  MADC32_FILL_p                fill;
  MADC32_END_OF_EVENT_p        end_of_event;

  signature = (MADC32_HEADER_SIGNATURE_p)&data;
  //printf("header = %1x subheader = %3x",  signature->header, signature->subheader);
  switch(signature->header){
  case MADC32_HEADER_SIGNATURE_HEADER:
    header = (MADC32_DATA_HEADER_p)&data;
    fprintf(fd, "Header: Module ID=%3d, Format=%d, Resolution=%d, Num Words=%4d", 
	    header->module_id, header->output_format, header->adc_resolution, header->n_data_words);
    break;
  case MADC32_HEADER_SIGNATURE_DATA: 
    switch(signature->subheader){
    case MADC32_SUBHEADER_EVENT:
      event = (MADC32_DATA_EVENT_p)&data;
      fprintf(fd, "Event: Ch=%2d, ADC=%5d, Out Of Range=%1d", 
	      event->channel_number, event->adc_amplitude, event->out_of_range);
      break;
    case MADC32_SUBHEADER_EXTENDED_TIME_STAMP:
      extended_time_stamp = (MADC32_EXTENDED_TIME_STAMP_p)&data;
      fprintf(fd, "ExtTS: Top 16 bit of Time Stamp=%5d", 
	      extended_time_stamp->time_stamp_msb);
      break;
    case MADC32_SUBHEADER_FILL:
      fill = (MADC32_FILL_p)&data;
      fprintf(fd, "Fill:");
      break;
    default:
      fprintf(fd, "Unknown subheader = 0x%.3x", signature->subheader);
      break;
    }
    break;
  case MADC32_HEADER_SIGNATURE_END_OF_EVENT:
    end_of_event = (MADC32_END_OF_EVENT_p)&data;
    fprintf(fd, "End of Event: Event Counter or Time Stamp =%10d", 
	    end_of_event->event_counter);
    break;
  default:
    fprintf(fd, "Unknown header = 0x%.1x", signature->header);
    break;
  }
  return 0;
}

int main(int argc, char *argv[]){
  int module_number = 0;
  FILE *fd;
  int file_flag = 0;
  int i;

  MADC32_p madc32=(MADC32_p)NULL;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... dump madc32 FIFO.\n", argv[0]);
    fprintf(stderr, "Usage: %s [module_number]\n", argv[0]);
    fprintf(stderr, " module_number: the default is 0.\n");
    exit(0);
  }

  if(argc>1){
    if('0'<=argv[1][0] && argv[1][0]<='9'){
      module_number = atoi(argv[1]);
    }else{
      fd = fopen(argv[1], "r");
      if(fd==(FILE*)NULL){
        fprintf(stderr, "Could not open file '%s'.\n", argv[1]);
        exit(-1);
      }
      file_flag = 1;
    }
  }

  printf("Module Number: %d\n", module_number);

  /* ------------- Open ------------ */

  if(!file_flag){
    if(madc32_open()){
      fprintf(stderr, "Error in madc32_open()\n");
      exit(-1);
    }

    madc32 = madc32_map(module_number);
    if(madc32==(MADC32_p)NULL){
      fprintf(stderr, "Error in madc32_map()\n");
      madc32_close();
      exit(-1);
    }
  }

  /* ------------- initialize MADC32  ------------ */
  madc32->multi_event = MADC32_MULTI_EVENT_YES_UNLIMITED;

  /* ------------- Show status ------------ */

  unsigned short buffer_data_length = madc32->buffer_data_length;
  printf("buffer data length = 0x%.4x (%6d) [0x%.4lx]\n", 
	 buffer_data_length, buffer_data_length,
	 (long)&madc32->buffer_data_length - (long)madc32);

  /* ------------- Dump MADC32 FIFO ------------ */
  
  for(i=0; i<buffer_data_length; i++){
    unsigned int data = madc32->fifo_read;
    printf(" [%.4x] %.8x ", i*2, data);
    show_data(stdout, data);
    printf("\n");
  }
  

  /* ------------- Close ------------ */
  if(!file_flag){
    if(madc32_unmap(module_number)){
      fprintf(stderr, "Error in madc32_unmap()\n");
    }

    if(madc32_close()){
      fprintf(stderr, "Error in madc32_close()\n");
    }
  }

  return 0; 
}


