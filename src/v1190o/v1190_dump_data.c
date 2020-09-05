/* v1190_dump_data.c ---- dump V1190 TDC data                              */
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
  int module_number = 0;
  int i;
  unsigned long  ldata;
  V1190_DATA_t   data;
  V1190_p v1190=(V1190_p)NULL;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... dump v1190 TDC data.\n", argv[0]);
    fprintf(stderr, "Usage: %s [module_number]\n", argv[0]);
    fprintf(stderr, " module_number: the default is 0.\n");
    exit(0);
  }

  if(argc>1){
    module_number = atoi(argv[1]);
  }

  printf("Module Number: %d\n", module_number);

  /* ------------- Open ------------ */

  if(v1190_open()){
    fprintf(stderr, "Error in v1190_open()\n");
    exit(-1);
  }

  v1190 = v1190_map(module_number);
  if(v1190==(V1190_p)NULL){
    fprintf(stderr, "Error in v1190_map()\n");
    v1190_close();
    exit(-1);
  }

  /* ------------- Show status ------------ */

  unsigned long event_counter = v1190->event_counter;
  printf("Event Count: 0x%.8lx(%ld)\n", event_counter, event_counter);

  /* ------------- Dump TDC data ------------ */

  unsigned long *buf = v1190->output_buffer;
  int endflag = 0;
  for(i=0; i<sizeof(v1190->output_buffer)/sizeof(long) && !endflag; i++){
    // ldata = buf[i];
    ldata = *buf;
    printf("%.4x: %.8lx\n", i*4, ldata);
    *(unsigned long *)&data = ldata;
    switch(data.global_header.id){
    case V1190_HEADER_ID_GH:
      printf("  Global Header\n");
      printf("    id:          %.2x(%d)\n", 
	     data.global_header.id, data.global_header.id);
      printf("    event count: %.6x(%d)\n", 
	     data.global_header.event_count, data.global_header.event_count);
      printf("    geo:         %.2x(%d)\n", 
	     data.global_header.geo, data.global_header.geo);
      break;
    case V1190_HEADER_ID_TH:
      printf("  TDC Header\n");
      printf("    tdc:         %.1x(%d)\n", 
	     data.tdc_header.tdc, data.tdc_header.tdc);
      printf("    event id:    %.3x(%d)\n", 
	     data.tdc_header.event_id, data.tdc_header.event_id);
      printf("    bunch id:    %.3x(%d)\n", 
	     data.tdc_header.bunch_id, data.tdc_header.bunch_id);
      break;
    case V1190_HEADER_ID_TM:
      printf("  TDC Measurement\n");
      printf("    trailing:    %.1x(%d) %s\n", 
	     data.tdc_measurement.trailing, data.tdc_measurement.trailing,
             data.tdc_measurement.trailing==0 ? "leading" : "trailing"
             );
      printf("    channel:     %.2x(%d)\n", 
	     data.tdc_measurement.channel, data.tdc_measurement.channel);
      printf("    measurement: %.5x(%d)\n", 
	     data.tdc_measurement.measurement, data.tdc_measurement.measurement);
      break;
    case V1190_HEADER_ID_TT:
      printf("  TDC Trailer\n");
      printf("    tdc:         %.1x(%d)\n", 
	     data.tdc_trailer.tdc, data.tdc_trailer.tdc);
      printf("    event_id:    %.3x(%d)\n", 
	     data.tdc_trailer.event_id, data.tdc_trailer.event_id);
      printf("    word_count:  %.3x(%d)\n", 
	     data.tdc_trailer.word_count, data.tdc_trailer.word_count);
      break;
    case V1190_HEADER_ID_TE:
      printf("  TDC Error\n");
      printf("    tdc:         %.1x(%d)\n", 
	     data.tdc_error.tdc, data.tdc_error.tdc);
      printf("    error flag:  %.4x(%d)\n", 
	     data.tdc_error.error_flag, data.tdc_error.error_flag);
      break;
    case V1190_HEADER_ID_ETTT:
      printf("  Extended Trigger Time Tag\n");
      printf("    time tag:    %.7x(%d)\n", 
	     data.time_tag.time_tag, data.time_tag.time_tag);
      break;
    case V1190_HEADER_ID_GT:
      printf("  Global Trailer\n");
      printf("    status:      %.1x(%d)\n", 
	     data.global_trailer.status, data.global_trailer.status);
      printf("    word count:  %.6x(%d)\n", 
	     data.global_trailer.word_count, data.global_trailer.word_count);
      printf("    geo:         %.2x(%d)\n", 
	     data.global_trailer.geo, data.global_trailer.geo);
      break;
    case V1190_HEADER_ID_FILLER:
      printf("  Filler\n");
      endflag = 1;
      break;
    default:
      printf("    Unknown Header id:         %.2x(%d)\n", 
	     data.global_header.id, data.global_header.id);
      endflag = 1;
      break;
    }
  }

  /* ------------- Close ------------ */
  if(v1190_unmap(module_number)){
    fprintf(stderr, "Error in v1190_unmap()\n");
  }

  if(v1190_close()){
    fprintf(stderr, "Error in v1190_close()\n");
  }

  return 0; 
}


