/* v1190_hit.c ---- show V1190 hit wires                                  */
/*								           */
/*  Version 1.00        2013-04-19      by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1190.h"

unsigned long wire_hits[V1190_MAX_N_MODULES][V1190_N_CH_PER_MODULE]; // wire hits


int prev_time_tag = -1;

void show_hit(){
  int n;
  int ch;
  int i, j;
  int sum;
  int k;
  printf("---------------------------------------------------------\n");
  for(n=0; n<V1190_MAX_N_MODULES; n++){
    sum = 0;
    for(ch=0; ch<V1190_N_CH_PER_MODULE; ch++){
      sum+= wire_hits[n][ch];
    }
    if(sum>0){
      for(i=0; i<V1190_N_CH_PER_MODULE/32; i++){
        printf("%1d:%c ", n, 'A'+i);
	for(j=0; j<32; j++){
          ch = i*32+j;
          if(wire_hits[n][ch]==0){
            printf("-");
	  }else if(wire_hits[n][ch]<=9){
            printf("%1ld", wire_hits[n][ch]);
	  }else{
            k=log10((double)wire_hits[n][ch])-1;
            printf("%c", 'A'+k);
	  }
          if(j%4==3){
            if(j%8==7){
              printf(" ");	  
	    }else{
              printf(".");	  
	    }
          }
	}
	printf("\n");
      }
    }
  }
}

int main(int argc, char *argv[]){
  int module_number = 0;
  int i;
  unsigned long  ldata;
  V1190_DATA_t   data;
  FILE *fd;
  int file_flag = 0;
  int geo=0;

  V1190_p v1190=(V1190_p)NULL;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... dump v1190 TDC data.\n", argv[0]);
    fprintf(stderr, "Usage: %s [module_number or file_name]\n", argv[0]);
    fprintf(stderr, " module_number: the default is 0.\n");
    exit(0);
  }

  if(argc>1){
    if('0'<=argv[1][0] && argv[1][0]<='9'){
      module_number = atoi(argv[1]);
    }else{
      fprintf(stderr, "Opening file '%s'.\n", argv[1]);
      fd = fopen(argv[1], "r");
      if(fd==(FILE*)NULL){
        fprintf(stderr, "Could not open file '%s'.\n", argv[1]);
        exit(-1);
      }
      file_flag = 1;
    }
  }


  /* ------------- Open ------------ */

  if(!file_flag){
    if(v1190_open()){
      fprintf(stderr, "Error in v1190_open()\n");
      exit(-1);
    }

    printf("Module Number: %d\n", module_number);
    v1190 = v1190_map(module_number);
    if(v1190==(V1190_p)NULL){
      fprintf(stderr, "Error in v1190_map()\n");
      v1190_close();
      exit(-1);
    }
  }

  /* ------------- Show status ------------ */

  if(!file_flag){
    unsigned long event_counter = v1190->event_counter;
    printf("Event Count: 0x%.8lx(%ld)\n", event_counter, event_counter);
  }

  /* ------------- Read TDC data ------------ */

  unsigned long *buf;
  if(!file_flag){
    buf = v1190->output_buffer;
  }

  int endflag = 0;
  for(i=0; !endflag; i++){
    if((i<0x100 && (i&0x000000F)==0)
       || (i<0x1000 && (i&0x000000FF)==0)
       || (i<0x10000 && (i&0x00000FFF)==0)
       || (i&0x0000FFFF)==0) show_hit();
    if(file_flag){
      if(fread(&ldata, sizeof(ldata), 1, fd)==0) break;
    }else{
      ldata = *buf;
    }
    //printf("%.4x: %.8lx\n", i*4, ldata);
    *(unsigned long *)&data = ldata;
    switch(data.global_header.id){
    case V1190_HEADER_ID_GH:
      geo = data.global_header.geo;
      break;
    case V1190_HEADER_ID_TH:
      break;
    case V1190_HEADER_ID_TM:
      wire_hits[geo][data.tdc_measurement.channel]++;
#if 0
      if(data.tdc_measurement.channel==112){
        fprintf(stderr, "pul%d = %8d\n", geo, data.tdc_measurement.measurement);
      }
#endif
      break;
    case V1190_HEADER_ID_TT:
    case V1190_HEADER_ID_TE:
    case V1190_HEADER_ID_ETTT:
    case V1190_HEADER_ID_GT:
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
  if(!file_flag){
    if(v1190_unmap(module_number)){
      fprintf(stderr, "Error in v1190_unmap()\n");
    }

    if(v1190_close()){
      fprintf(stderr, "Error in v1190_close()\n");
    }
  }

  return 0; 
}


