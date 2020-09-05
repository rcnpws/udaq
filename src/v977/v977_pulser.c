/* v977_pulser.c ---- produce pulse(s) by V977A                                  */
/*								           */
/*  Version 1.10        2013-07-31      by A. oiwa (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v977.h"

int main(int argc, char *argv[]){
  
  if(argc!=3){
    printf("Usage: $v977_pulser [number of output ch] [number of pulse]\n");
    return 0;
  }
  int ch = atoi(argv[1]);
  int num = atoi(argv[2]);
  
  
  int ret;
  unsigned short cr;
  int module_number = 0;
  unsigned short *ptr;
  
  V977_p v977=(V977_p)NULL;
  
  fprintf(stderr, "open V977A\n");
  ret = v977_open();
  if(ret!=0){
    fprintf(stderr, "Error in v977_open()\n");
    exit(-1);
  }
  
  fprintf(stderr, "map V977A(module_number=%d)\n", module_number);
  v977 = v977_map(module_number);
  if(v977==(V977_p)NULL){
    fprintf(stderr, "Error in v977_map()\n");
    exit(-1);
  }
  
  printf("v977 = 0x%.8lx\n", (long)v977);
  
  printf("size of *v977 = %d\n", sizeof(*v977));
  ptr = (unsigned short*)v977;
  printf("dummy = %d\n", ptr[0x002a/2]);
  
  
  v977->control_register = 0x0000; 
  cr = v977->control_register;
  printf("control register = 0x%.4x\n", cr);
  
  printf("  Pattern Bit: %s\n", (cr&0x0001)==0 ? "0...I/O REGISTER" : "1...MULTIHIT PATTERN UNIT");
  printf("  Gate From FRONT PANEL signal is: %s\n", (cr&0x0002)==0 ? "0...enabled " : "1...masked");
  printf("  OR and /OR FRONT outputs are: %s\n", (cr&0x0002)==0 ? "0...enabled " : "1...masked");
  
  //job start
  
  
  int i=0;
  int data;
  data = 1<<ch;
  
  fprintf(stderr,"output to %d (0x%.4x) ch for %d times\n",ch, data, num);
  

  for(i=0; i<num;i++){
    v977->output_set = data;
    v977->output_set = 0x0000;
    usleep(0);
  }
  
  //job end
  
  fprintf(stderr, "close V977A\n");
  ret = v977_unmap(module_number);
  if(ret!=0){
    fprintf(stderr, "Error in v977_unmap()\n");
  }
  
  
  ret = v977_close();
  if(ret!=0){
    fprintf(stderr, "Error in v977_close()\n");
    exit(-1);
  }
  return 0; 
}


