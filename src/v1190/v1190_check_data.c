/* v1190_check_data.c ---- check V1190 TDC data                             */
/*	read data from a TDC,                                               */
/*      check the format of the data,                                       */
/*      and show event                                                      */
/*								            */
/*								            */
/*  Version 1.00        2013-02-03      by A. Tamii (For Linux2.6)GE-FANUC  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1190.h"

#define DEBUG 0   /* 1 = debug mode */

int geometrical_address = -1;
long event_count = -1;

#define MAX_DUMP_BUF 0x1000
unsigned long dump_buffer[MAX_DUMP_BUF];

void dump_data(unsigned long *buf, int length){
  int i;
  for(i=0; i<length; i++){
    fprintf(stdout, "%.8lx ", buf[i]);
    if((i%4)==3) fprintf(stdout, "\n");
  }
  fprintf(stdout, "\n");
}

void check_event(V1190_p v1190, V1190_GH_p global_header_p){
  V1190_DATA_t   data;
  unsigned long *output_buffer;
  int i;
  int ntdc = 0;
  int word_count;
  int tdc_word_count;
  int end_flag = 0;
  int in_tdc_data = 0;    /* a flag to indicate that the pointer is in between a TDC header and a trailer */


#if DEBUG
   printf("  Global Header\n");
   printf("    event count: 0x%.6x(%d)\n", 
  	     global_header_p->event_count, global_header_p->event_count);
   printf("    geo:         0x%.2x(%d)\n", 
	     global_header_p->geo, global_header_p->geo);
#endif

   if(geometrical_address<0){
     geometrical_address = global_header_p->geo;
     printf("Geometrical Address =  0x%.2d\n", geometrical_address);
   }else{
     if(global_header_p->geo != geometrical_address){
       fprintf(stdout, "The geometrical address has changed from 0x%.2x to 0x%.2x\n",
	       geometrical_address, global_header_p->geo);
       geometrical_address = global_header_p->geo;
     }
   }

  output_buffer = v1190->output_buffer;

  dump_buffer[0] = *(unsigned long*)global_header_p;

  for(word_count=1; !end_flag; word_count++){
    data = *(V1190_DATA_t*)output_buffer;
    dump_buffer[word_count] = *(unsigned long*)&data;

    switch(data.global_header.id){
    case V1190_HEADER_ID_GH:
        fprintf(stdout, "Unexpected global header in an event.\n");
        fprintf(stdout, "    event count: 0x%.6x(%d)\n", 
	     data.global_header.event_count, data.global_header.event_count);
        fprintf(stdout, "    geo:         0x%.2x(%d)\n", 
	     data.global_header.geo, data.global_header.geo);
        dump_data(dump_buffer, word_count+1);
      break;
    case V1190_HEADER_ID_GT:
#if DEBUG
      fprintf(stdout, "TDC Global Trailer\n");
#endif
      if(in_tdc_data) fprintf(stdout, "Unexpected TDC global trailer (no TDC trailer)\n");
      if(data.global_trailer.status!=0){
        fprintf(stdout, "Error in global trailer status.\n");
        if(data.global_trailer.status & 0x1)
          fprintf(stdout, "  TDC Error\n");
        if(data.global_trailer.status & 0x2)
          fprintf(stdout, "  Output buffer overflow\n");
        if(data.global_trailer.status & 0x4)
          fprintf(stdout, "  Trigger lost\n");
      }
      if(data.global_trailer.geo != geometrical_address){
        fprintf(stdout, "The geometrical address in global trailer (%.2x) is different from the global header (%.2x)\n",
	       data.global_trailer.geo, geometrical_address);
      }
      if(data.global_trailer.word_count != word_count+1){
        fprintf(stdout, "The word_count in global trailer (%.4x) is different from the real word count (%.4x)\n",
	       data.global_trailer.word_count, word_count+1);
        dump_data(dump_buffer, word_count+1);
      }
      end_flag = 1;
      break;
    case V1190_HEADER_ID_TH:
#if DEBUG
      fprintf(stdout, "TDC Header\n");
#endif
      if(in_tdc_data) fprintf(stdout, "Unexpected TDC header\n");
      in_tdc_data = 1;
      if(data.tdc_header.tdc != ntdc){
        fprintf(stdout, "The number 'TDC' in TDC header (%d) is different from the expected value (%d)\n", data.tdc_header.tdc, ntdc);
      }
      if(data.tdc_header.event_id != (global_header_p->event_count & 0x00000FFFL)){
        fprintf(stdout, "The event_id in TDC header (0x%.4x)is different from the event counter in the global header (%.6x)\n", data.tdc_header.event_id, global_header_p->event_count);
      }
      tdc_word_count = word_count;
      break;
    case V1190_HEADER_ID_TM:
#if DEBUG
      fprintf(stdout, "TDC Measurement\n");
#endif
      if(in_tdc_data) fprintf(stdout, "Unexpected TDC measurement\n");
      break;
    case V1190_HEADER_ID_TT:
#if DEBUG
      fprintf(stdout, "TDC Trailer\n");
#endif
      if(!in_tdc_data) fprintf(stdout, "Unexpected TDC trailer\n");
      if(data.tdc_trailer.tdc != ntdc){
        fprintf(stdout, "The number 'TDC' in TDC trailer (%d) is different from the expected value (%d)\n", data.tdc_trailer.tdc, ntdc);
      }
      if(data.tdc_trailer.event_id != (global_header_p->event_count & 0x00000FFFL)){
        fprintf(stdout, "The event_id in TDC trailer (0x%.4x)is different from the event counter in the global header (0x%.6x)\n", 
            data.tdc_trailer.event_id, global_header_p->event_count);
      }
      if(data.tdc_trailer.word_count!=word_count-tdc_word_count+1){
        fprintf(stdout, "The word count in TDC trailer (0x%.4x) is different from the real word count (0x%.4x)\n", 
		data.tdc_trailer.word_count, word_count-tdc_word_count+1);
      }
      ntdc++;
      in_tdc_data = 0;
      break;
    case V1190_HEADER_ID_TE:
       fprintf(stdout, "TDC Error word. TDC=%d\n", data.tdc_error.tdc);
       for(i=0; i<v1190_n_tdc_error_string; i++){
         int bit = 1<<i; 
         if(data.tdc_error.error_flags & bit){
           fprintf(stdout, "  %s\n", v1190_tdc_error_string[i]);
	 } 
      }
      break;
    case V1190_HEADER_ID_ETTT:
#if DEBUG
      fprintf(stdout, "Extended Trigger Time Tag\n");
#endif
      //fprintf(stdout, "ettt = %ld\n", (long)data.time_tag.time_tag);
      break;
    case V1190_HEADER_ID_FILLER:
      fprintf(stdout, "Unexpected filler in an event.\n");
      //end_flag = 1;
      break;
    default:
      fprintf(stdout, "    Unknown Header id:  0x%.2x(%d), Data = 0x%.8lx\n", 
	     data.global_header.id, data.global_header.id, 
	     *(unsigned long*)&data
             );
      end_flag = 1;
      break;
    }
        
  }
}

void check_data(V1190_p v1190){
  V1190_DATA_t   data;
  unsigned long *output_buffer;
  int end_flag = 0;
  
  output_buffer = v1190->output_buffer;

  while(!end_flag){
    data = *(V1190_DATA_t*)output_buffer;

    switch(data.global_header.id){
    case V1190_HEADER_ID_GH:
      if(event_count<0){
        event_count = data.global_header.event_count;
      }else{
        event_count++;
        if(event_count != data.global_header.event_count){
          fprintf(stdout, "    Event count 0x%.6x(%d) is different from the expected value 0x%.6lx(%ld)\n",
             data.global_header.event_count, data.global_header.event_count,
             event_count, event_count);
          event_count = data.global_header.event_count;
	}
      }
      if(event_count<=100 || (event_count<=100000 && (event_count%1000)==0) || (event_count%100000)==0)
         printf(" event count = %ld\n", event_count);

      check_event(v1190, &data.global_header);
      break;
    case V1190_HEADER_ID_TH:
      fprintf(stdout, "    Unexpected TDC header (no global header)\n");
      break;
    case V1190_HEADER_ID_TM:
      fprintf(stdout, "    Unexpected TDC measurement (no global header)\n");
      break;
    case V1190_HEADER_ID_TT:
      fprintf(stdout, "    Unexpected TDC trailer (no global header)\n");
      break;
    case V1190_HEADER_ID_TE:
      fprintf(stdout, "    Unexpected TDC error (no global header)\n");
      break;
    case V1190_HEADER_ID_ETTT:
      fprintf(stdout, "    Unexpected TDC extended trigger time tag (no global header)\n");
      break;
    case V1190_HEADER_ID_GT:
      fprintf(stdout, "    Unexpected TDC global trailer (no global header)\n");
      break;
    case V1190_HEADER_ID_FILLER:
#if DEBUG
      printf(" Filler\n");
#endif
      end_flag = 1;
      break;
    default:
      fprintf(stdout, "    Unknown Header id:  0x%.2x(%d), Data = 0x%.8lx\n", 
	     data.global_header.id, data.global_header.id, 
	     *(unsigned long*)&data
             );
      end_flag = 1;
      break;
    }
        
  }
}

int main(int argc, char *argv[]){
  int module_number = 0;
  V1190_p v1190=(V1190_p)NULL;
  int loop_flag = 0;
  int ldata;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stdout, "%s ... check v1190 TDC data.\n", argv[0]);
    fprintf(stdout, "Usage: %s [-lh] [module_number]\n", argv[0]);
    fprintf(stdout, " module_number: the default is 0.\n");
    fprintf(stdout, " -h: show this help.\n");
    fprintf(stdout, " -l: loop until stopped.\n");
    exit(0);
  }

  argc--;
  argv++;

  while(argc>0 && argv[0][0]=='-'){
    switch(argv[0][1]){
    case 'l':
      loop_flag = 1;
      break;
    }
    argc--;
    argv++;
  }

  if(argc>0){
    module_number = atoi(argv[0]);
  }

  printf("V1190-TDC Module Number: %d\n", module_number);

  /* ------------- Open ------------ */

  if(v1190_open()){
    fprintf(stdout, "Error in v1190_open()\n");
    exit(-1);
  }

  v1190 = v1190_map(module_number);
  if(v1190==(V1190_p)NULL){
    fprintf(stdout, "Error in v1190_map()\n");
    v1190_close();
    exit(-1);
  }

  /* ------------- Do my task ------------ */

  v1190->software_clear = 1;   /* software clear, clear the event counter */

  v1190->output_prog_control = 2; /* select Almost Full for output */
  v1190->almost_full_level = 16384; /* 16384 words = a half of the memory */

  while(1){
    if(loop_flag){
      while(1){
        ldata = v1190->status_register;
        if((*(V1190_SR_p)&ldata).data_ready) break;
        usleep(0);
      }
    }
#if DEBUG
  fprintf(stdout, "enter  check_data()\n");
#endif
    check_data(v1190);
#if DEBUG
  fprintf(stdout, "eixt  check_data()\n");
#endif
    usleep(100000);
    if(!loop_flag) break;
  }



  /* ------------- Close ------------ */
  if(v1190_unmap(module_number)){
    fprintf(stdout, "Error in v1190_unmap()\n");
  }

  if(v1190_close()){
    fprintf(stdout, "Error in v1190_close()\n");
  }

  return 0; 
}

