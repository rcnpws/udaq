/* v977_ope.c ---- access CAEN V977 I/O register                           */
/*								           */
/*  Version 1.00        2013-09-05      by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v977.h"

#define DEBUG  1

static int verbose = 0;

static V977_p v977=(V977_p)NULL;

static int v977_init(int module_number){
 if(verbose) fprintf(stderr, "open V977\n");
	if(v977_open()){
 	  fprintf(stderr, "Error in v977_open()\n");
	  return -1;
	}

	if(verbose) fprintf(stderr, "map V977 (module number=%d)\n", module_number);
	v977 = v977_map(module_number);
	if(v977==(V977_p)NULL){
		fprintf(stderr, "Error in v977_map()\n");
		return -1;
	}

	if(verbose){
    printf("v977 = 0x%.8lx\n", (long)v977);
  	printf("size of *v977 = %d\n", sizeof(*v977));
	}
  return 0;
}

static int v977_exit(int module_number){
	if(verbose) fprintf(stderr, "unmap V977 (module number=%d)\n", module_number);
	if(v977_unmap(module_number)){
		fprintf(stderr, "Error in v977_unmap()\n");
		return -1;
	}

	if(verbose) fprintf(stderr, "close V977\n");
	if(v977_close()){
		fprintf(stderr, "Error in v977_close()\n");
		return -1;
	}
  return 0;
}

int main(int argc, char *argv[]){
	unsigned short cr;
	int module_number = 0;
  int val;
 
  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stdout, "%s ... operate CAEN V977 I/O register.\n", argv[0]);
    fprintf(stdout, "Usage: %s [-h] [-i|-r|-rs|-rm] [-s number] \n", argv[0]);
    fprintf(stdout, " -i:  initialize\n");
    fprintf(stdout, " -r:  read\n");
    fprintf(stdout, " -rs: single-hit read clear\n");
    fprintf(stdout, " -rm: multi-hit read clear\n");
    fprintf(stdout, " -s:  set output\n");
    fprintf(stdout, " -h:  show this help.\n");
    exit(0);
  }

  if(v977_init(module_number)) exit(-1);

  argc--;
  argv++;
  for(;argc>0;argc--,argv++){
    if(argv[0][0]=='-' && strlen(argv[0])>1){
      switch((*argv)[1]){
      case 'i':
      	printf("Initialize\n");
      	v977->control_register = 0x0002;  /* default setting */
      	v977->input_set = 0;
      	v977->input_mask = 0;
      	v977->output_set = 0;
      	v977->output_mask = 0;
      	v977->clear_output = 0;
      	v977->test_control_register = 0;
      	printf("Done\n");
        break;
      case 'r':
        if(strlen(argv[0])<=2){
          val = v977->input_read;
          printf("Input = %d (0x%x)\n", val, val);
				}else{
          if(argv[0][2]=='s'){
            val = v977->singlehit_read_clear;
            printf("Single Hit Read = %d (0x%x)\n", val, val);
					}else if(argv[0][2]=='m'){
            val = v977->multihit_read_clear;
            printf("Multi Hit Read = %d (0x%x)\n", val, val);
					}else{
            fprintf(stderr, "Unkown option '%s'.\n", argv[0]);
					}
				}
        break;
      case 's':
        if(argc<2){
          fprintf(stderr, "Argument is not given for '-%c'.\n", argv[0][1]);
          break;
				}
        argc--; argv++;
        val = strtol(argv[0],NULL,0);
        printf("Set output to  = %d (0x%x)\n", val, val);
        v977->output_set = val;
        break;
      case 'c':
      	cr = v977->control_register;
      	printf("control register = 0x%.4x\n", cr);
      	printf("  Pattern Bit:      %s\n", 
                 (cr&0x0001)==0 ? "0...I/O REGISTER" : "1...MULTIHIT PATTERN UNIT");
	      printf("  Front Panel Gate: %s\n", (cr&0x0002)==0 ? "0...enabled " : "1...masked");
	      printf("  OR Output:        %s\n", (cr&0x0004)==0 ? "0...enabled " : "1...masked");
				break;
			default:
        fprintf(stderr, "Unkown option '-%c'.\n", argv[0][1]);
        break;
			}
		}
	}




  if(v977_exit(module_number)) exit(-1);

	return 0; 
}

/*
  Local Variables: 
  mode: C
  tab-width: 2
  End:*
*/
