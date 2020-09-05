/* myriad_control.c ---- MyRIAD control                                    */
/*								           */
/*  Version 1.00        2016-03-04      by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "myriad.h"


int usage(char *prog_name){
  fprintf(stderr, "%s ... show MyRIAD status.\n", prog_name);
  fprintf(stderr, "Usage: %s [-hdtf]\n",  prog_name);
  fprintf(stderr, " -d: reset Digital Clock Manger (DCM)\n");
  fprintf(stderr, " -t: reset Time Stamp\n");
  fprintf(stderr, " -f: reset FIFO\n");
  fprintf(stderr, " -c: clear trigger counter\n");
  fprintf(stderr, " -l: reset timestamp lost lock\n");
  fprintf(stderr, " -nim: select local nim trigger\n");
  fprintf(stderr, " -ecl: select local ecl trigger (WSI)\n");
  fprintf(stderr, " -master: select master trigger (WSI)\n");
  fprintf(stderr, " -int: select internal time stamp clock\n");
  fprintf(stderr, " -ext: select external time stamp clock\n");
  fprintf(stderr, " -h: show this help\n");
  return 0;
}

int main(int argc, char *argv[]){
  int ret;
  int d_flag = 0;
  int t_flag = 0;
  int f_flag = 0;
  int c_flag = 0;
  int l_flag = 0;
  int nim_flag = 0;
  int ecl_flag = 0;
  int master_flag = 0;
  int int_flag = 0;
  int ext_flag = 0;
  char *prog_name;
  char *opt_str;


  prog_name = *argv;
  argc--;
  argv++;

  if(argc<=0){
    usage(prog_name);
    exit(0);
  }

  while(argc>0){
    if(argv[0][0]=='-'){
      opt_str = &argv[0][1];
      if(!strcmp(opt_str, "h")){
	usage(prog_name);
	exit(0);
      }else if(!strcmp(opt_str, "d")){
	d_flag = 1;
      }else if(!strcmp(opt_str, "t")){
	t_flag = 1;
      }else if(!strcmp(opt_str, "f")){
	f_flag = 1;
      }else if(!strcmp(opt_str, "c")){
	c_flag = 1;
      }else if(!strcmp(opt_str, "l")){
	l_flag = 1;
      }else if(!strcmp(opt_str, "nim")){
	nim_flag = 1;
      }else if(!strcmp(opt_str, "ecl")){
	ecl_flag = 1;
      }else if(!strcmp(opt_str, "master")){
	master_flag = 1;
      }else if(!strcmp(opt_str, "int")){
	int_flag = 1;
      }else if(!strcmp(opt_str, "ext")){
	ext_flag = 1;
      }else{
	fprintf(stderr, "Unknown switch '%s'\n", *argv);
	usage(prog_name);
	exit(-1);
      }
    }else{
      fprintf(stderr, "Invaid argument '%s'\n", *argv);
      usage(prog_name);
      exit(-1);
    }
    argc--;
    argv++;
  }
      
  MyRIAD_p myriad=(MyRIAD_p)NULL;

  fprintf(stderr, "MyRIAD base address = 0x%.8x\n", MYRIAD_BASE);

  fprintf(stderr, "open MyRIAD\n");
  ret = myriad_open();
  if(ret!=0){
    fprintf(stderr, "Error in myriad_open()\n");
    exit(-1);
  }

  fprintf(stderr, "map MyRIAD\n");
  myriad = myriad_map();
  if(myriad==(MyRIAD_p)NULL){
    fprintf(stderr, "Error in myriad_map()\n");
    myriad_close();
    exit(-1);
  }

  fprintf(stderr, "myriad = 0x%.8lx\n", (long)myriad);

  MyRIAD_pulsed_control_t pulsed_control;
  // fprintf(stderr, "size of pulsed.. = %d\n", sizeof(pulsed_control));
  pulsed_control.raw.sint = 0x0000;

  if(d_flag){
    fprintf(stderr, "Send 'DCM Reset' to the Myriad.\n");
    pulsed_control.bit.dcm_reset = 1;
  }
  if(t_flag){
    fprintf(stderr, "Send 'Timestamp Reset' to the Myriad.\n");
    pulsed_control.bit.local_ts_reset = 1;
  }
  if(f_flag){
    fprintf(stderr, "Send 'FIFO Reset' to the Myriad.\n");
    pulsed_control.bit.fifo_reset = 1;
  }
  if(c_flag){
    fprintf(stderr, "Send 'Clear Trigger Counter' to the Myriad.\n");
    pulsed_control.bit.clear_trig_counter = 1;
  }
  if(l_flag){
    fprintf(stderr, "Send 'Timestamp Lost Lock Reset' to the Myriad.\n");
    pulsed_control.bit.sm_lost_lock_reset = 1;
  }
  fprintf(stderr, "pulsed control data = %.4x\n", pulsed_control.raw.sint);
  myriad->pulsed_control = pulsed_control.raw.sint;

  if(nim_flag){
    fprintf(stderr, "Select trigger: local nim-0.\n");
    myriad->gating_register = 0x0001;
  }else if(ecl_flag){
    fprintf(stderr, "Select trigger: local ecl (WSI).\n");
    myriad->gating_register = 0x8001;
  }else if(master_flag){
    fprintf(stderr, "Select trigger: master trigger.\n");
    myriad->gating_register = 0x0002;
  }else if(int_flag){
    fprintf(stderr, "Select internal time stamp clock.\n");
    myriad->serdes_config = myriad->serdes_config | 0x8000;
    myriad->ts_propagation_control = myriad->ts_propagation_control & 0xfffe;
  }else if(ext_flag){
    fprintf(stderr, "Select external time stamp clock.\n");
    myriad->serdes_config = myriad->serdes_config & 0x7fff;
    myriad->ts_propagation_control = myriad->ts_propagation_control | 0x0001;
  }
  
  fprintf(stderr, "unmap MyRIAD\n");
  myriad_unmap();
  fprintf(stderr, "close MyRIAD\n");
  myriad_close();

  return 0; 
}


