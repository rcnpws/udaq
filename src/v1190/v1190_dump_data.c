/* v1190_dump_data.c ---- dump V1190 TDC data                           */
/*								        */
/*  Version 1.00 2012-12-24 by A. Tamii (For Linux2.6)GE-FANUC          */
/*  Version 2.00 2015-03-13 by A. Tamii (separted from v1190_dump)      */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1190.h"

static int prev_time_tag = -1;

/* ------------- Dump TDC data ------------ */

int v1190_dump_data(unsigned long *buffer, unsigned long length_in_bytes)
{
  int endflag = 0;
  unsigned long  ldata;
  V1190_DATA_t   data;
  int i;
  for(i=0; !endflag && i<length_in_bytes/sizeof(long); i++){
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
	     data.tdc_error.error_flags, data.tdc_error.error_flags);
       fprintf(stderr, "TDC Error word. TDC=%d\n", data.tdc_error.tdc);
       for(i=0; i<v1190_n_tdc_error_string; i++){
         int bit = 1<<i; 
         if(data.tdc_error.error_flags & bit){
           fprintf(stderr, "  %s\n", v1190_tdc_error_string[i]);
	 } 
      }
      break;
    case V1190_HEADER_ID_ETTT:
      printf("  Extended Trigger Time Tag\n");
      printf("    time tag:    %.7x(%d)\n", 
	     data.time_tag.time_tag, data.time_tag.time_tag);

      if(prev_time_tag > data.time_tag.time_tag){
	printf("time tag cleared from %d to %d\n", prev_time_tag, data.time_tag.time_tag);
      }
      prev_time_tag = data.time_tag.time_tag;
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
  return(endflag);
}
