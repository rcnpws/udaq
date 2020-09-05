/* mqdc32_threshold.c ---- set threshold of MQDC32                         */
/*								           */
/*  Version 0.10        2020-09-05      by T. Furuno (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <unistd.h>

#include <vme/vme.h>
#include <vme/vme_api.h>

#include "mqdc32.h"

int usage(char *name){
  fprintf(stderr, "%s ... set threshold of MQDC32.\n", name);
  fprintf(stderr, "Usage: %s [-h] [-p module_number] [module_number channel threshold]\n", name);
  fprintf(stderr, "       -h show this help\n");
  fprintf(stderr, "       -p show the present thresholds\n");
  fprintf(stderr, "       if threshold==0x1FFF the channel is killed.\n");
  return 0;
}

int main(int argc, char *argv[]){
  unsigned int module_number = 0;
  unsigned int channel = 0;
  unsigned int threshold = 0;
  int i;
  int p_flag = 0;  /* flag for showing thresholds */

  MQDC32_p mqdc32=(MQDC32_p)NULL;

  if(argc>1 && !strcmp(argv[1],"-h")){
    usage(argv[0]);
    exit(0);
  }

  if(argc>1 && !strcmp(argv[1],"-p")){
    p_flag = 1;
  }

  if((p_flag && argc<2) || (!p_flag && argc<4)){
    fprintf(stderr, "Too few arguments\n");
    usage(argv[0]);
    exit(-1);
  }

  module_number = atoi(argv[1]);
  if(!p_flag){
    channel = atoi(argv[2]);
    if(argv[3][0]=='0' && argv[3][1]=='x'){
      sscanf(&argv[3][2], "%x", &threshold);
    }else{
      threshold = atoi(argv[3]);
    }
  }


  //printf("Module Number: %d\n", module_number);

  if(channel<0 || MQDC32_NUM_CHANNELS<=channel){
    fprintf(stderr, "Invalid channel number = %d\n", channel);
    exit(-1);
  }
  
  if(threshold<0 || 0x1FFF<threshold){
    fprintf(stderr, "Invalid threshold value = %d (0x%x)\n", threshold, threshold);
    exit(-1);
  }
  
  /* ------------- Open ------------ */

  if(mqdc32_open()){
    fprintf(stderr, "Error in mqdc32_open()\n");
    exit(-1);
  }
  
  mqdc32 = mqdc32_map(module_number);
  if(mqdc32==(MQDC32_p)NULL){
    fprintf(stderr, "Error in mqdc32_map()\n");
    mqdc32_close();
    exit(-1);
  }

  /* ------------- Show thresholds  ------------ */
  if(p_flag){
    printf("module number = %1d\n", module_number);
    for(i=0; i<MQDC32_NUM_CHANNELS; i++){
      int th = mqdc32->threshold[i];
      printf("%2d) %4d (0x%.4x) %s\n", i, th, th, th==0x1FFF ? "killed":"");
    }
  }
	      
  /* ------------- Set threshold  ------------ */
  if(!p_flag){
    printf("module = %d, channel = %d, threshold = %d (0x%x)\n", module_number, channel, threshold, threshold);
    mqdc32->threshold[channel] = threshold;
  }

  /* ------------- Close ------------ */
  if(mqdc32_unmap(module_number)){
    fprintf(stderr, "Error in mqdc32_unmap()\n");
  }
  
  if(mqdc32_close()){
    fprintf(stderr, "Error in mqdc32_close()\n");
  }

  return 0; 
}


