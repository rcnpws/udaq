/* myriad_status.c ---- show MyRIAD status                                 */
/*								           */
/*  Version 1.00        2016-03-04      by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "myriad.h"

char *ts_latch_source_strings[] = {
  "No latch",
  "Local detector trigger",
  "Master trigger over SerDes",
  "Both Local & Master"
};


void usage(char *prog_name){
  fprintf(stderr, "%s ... show MyRIAD status.\n", prog_name);
  fprintf(stderr, "Usage: %s [-hf]\n", prog_name);
  fprintf(stderr, "  -h: show this help\n");
  fprintf(stderr, "  -f: show FIFO data\n");
}

int main(int argc, char *argv[]){
  int ret;
  int i, j;
  int f_flag = 0;
  char *prog_name;
  char *opt_str;

  MyRIAD_p myriad=(MyRIAD_p)NULL;

  if(argc>1 && !strcmp(argv[1],"-h")){
    exit(0);
  }

  prog_name = *argv;

  argc--;
  argv++;

  while(argc>0){
    if(argv[0][0]=='-'){
      opt_str = &argv[0][1];
      if(!strcmp(opt_str, "h")){
	usage(prog_name);
	exit(0);
      }else if(!strcmp(opt_str, "f")){
	f_flag = 1;
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


  /* ------------- Show status ------------ */
  printf("--------------------------------------------------------------------------\n");

  printf("Board ID             : 0x%.4x      should be = %.4x\n", myriad->board_id, MYRIAD_BOARD_ID);

  printf("Code Revision        : 0x%.4x [0x%.4lx]                     \n", 
	 myriad->code_revision, (long)&myriad->code_revision - (long)myriad);
  MyRIAD_code_revision_t code_revision;
  *(unsigned short*)&code_revision = myriad->code_revision;
  printf("  PCB Revision:      : %d               \n", code_revision.PCB_revision);
  printf("  Firmware Type:     : %d               \n", code_revision.firmware_type);
  printf("  Code Revision:     : %d.%d            \n", 
	 code_revision.code_revision_major, code_revision.code_revision_minor );
  printf("  Code Date:         : %.4x%.4x         \n", 
	 myriad->code_date[1], myriad->code_date[0]);


  printf("FIFO Status          : 0x%.4x [0x%.4lx]                     \n", 
	 myriad->fifo_status, (long)&myriad->fifo_status - (long)myriad);
  MyRIAD_FIFO_status_t fifo_status;
  *(unsigned short*)&fifo_status = myriad->fifo_status;
  printf("  FLAG: 0 1 2 3      : %d %d %d %d     extra or parity bits \n", 
	 fifo_status.flag0, fifo_status.flag1, fifo_status.flag2, fifo_status.flag3);
  printf("  EF:   A B          : %d %d         empty flags            \n", 
	 fifo_status.ef_a, fifo_status.ef_b);
  printf("  PAE:  A B          : %d %d         partially empty flags  \n", 
	 fifo_status.pae_a, fifo_status.pae_b);
  printf("  HF:   A B          : %d %d         harf full               \n", 
	 fifo_status.hf_a, fifo_status.hf_b);
  printf("  FFIR: A B          : %d %d         full flag or input ready\n", 
	 fifo_status.ffir_a, fifo_status.ffir_b);
  printf("  DAV:               : %d           DAV\n", 
	 fifo_status.dav);

  printf("Hardware Status      : 0x%.4x [0x%.4lx]                     \n", 
	 myriad->hardware_status, (long)&myriad->hardware_status - (long)myriad);
  MyRIAD_hardware_status_t hardware_status;
  *(unsigned short*)&hardware_status = myriad->hardware_status;
  printf("  STAT: 0 1          : %d %d         non-SerDes LVDS lines    \n", 
	 hardware_status.stat0, hardware_status.stat1);
  printf("  SD_LOCK:           : %d           SerDes LOCK* (active low) \n", 
	 hardware_status.sd_lock);
  printf("  SD_SM_LOCK:        : %d           SerDes State Machine Lock \n", 
	 hardware_status.sd_sm_lock);
  printf("  SD_SM_LST_LK:      : %d           SerDes State Machine Lost Lock \n", 
	 hardware_status.sd_sm_lst_lk);
  printf("  UNQ_SM_LCK:        : %d           SerDes Unqualified State Machine Lock \n", 
	 hardware_status.sd_unq_sm_lk);
  printf("  SYNC:              : %d           SerDes SYNC command \n", 
	 hardware_status.sync);
  printf("  ISYNC:             : %d           SerDes Imperial SYNC command \n", 
	 hardware_status.isync);
  printf("  DCM_LOCK:          : %d           Digital Clock Manager Logic Lock \n", 
	 hardware_status.dcm_lock);
  printf("  DCM_STATUS:        : %.2d          Digital Clock Manager Logic Status \n", 
	 hardware_status.dcm_status);
  printf("  CNTR_ERROR:        : %d           Counter Error \n", 
	 hardware_status.cntr_err);

  printf("FIFO Control         : 0x%.4x [0x%.4lx]                     \n", 
	 myriad->fifo_control, (long)&myriad->fifo_control - (long)myriad);
  MyRIAD_FIFO_control_t fifo_control;
  *(unsigned short*)&fifo_control = myriad->fifo_control;
  printf("  FLAG: 0 1 2 3      : %d %d %d %d     extra or parity input    \n", 
	 fifo_control.flag0, fifo_control.flag1, fifo_control.flag2, fifo_control.flag3);
  printf("  BE/LE:             : %d           %s endian    \n", 
	 fifo_control.be_le, fifo_control.be_le ? "Big":"Little" );
  printf("  FWFT/SI:           : %d           %s FIFO Mode\n", 
	 fifo_control.fwft_si, fifo_control.fwft_si ? "First-Word-Fall-Thru":"Standard" );

  printf("NIM Input Status     : 0x%.4x [0x%.4lx]                     \n", 
	 myriad->nim_status, (long)&myriad->nim_status - (long)myriad);

  printf("Gating Register      : 0x%.4x [0x%.4lx]                     \n", 
	 myriad->gating_register, (long)&myriad->gating_register - (long)myriad);
  MyRIAD_gating_register_t gating_register;
  *(unsigned short*)&gating_register = myriad->gating_register;
  printf("  TRIG_IN_SEL:       : %d         = %s    \n", 
	 gating_register.trig_in_sel, gating_register.trig_in_sel ? "ECL":"NIM");
  printf("  INT_MUX: A B       : %d %d         internal logic analyzer selection  \n", 
	 gating_register.ila_mux_a, gating_register.ila_mux_b);
  printf("  FORCE_FIFO:        : %d         = %s  \n", 
	 gating_register.force_fifo, gating_register.force_fifo ? "Yes":"No");
  printf("  TS_LATCH_SOURCE:   : %d         = %s  \n", 
	 gating_register.ts_latch_source, ts_latch_source_strings[gating_register.ts_latch_source]);

  printf("ECL-A Input Status   : 0x%.4x [0x%.4lx]                     \n", 
	 myriad->ecl_a_status, (long)&myriad->ecl_a_status - (long)myriad);
  printf("ECL-B Input Status   : 0x%.1x    [0x%.4lx]                     \n", 
	 myriad->ecl_b_status, (long)&myriad->ecl_b_status - (long)myriad);


  unsigned long long time_stamp;
  time_stamp
    = ((unsigned long long)myriad->latched_time_stamp[0] << 32) 
    + ((unsigned long long)myriad->latched_time_stamp[1] << 16) 
    + ((unsigned long long)myriad->latched_time_stamp[2] <<  0);
  printf("Latched Time Stamp   : 0x%.4x-%.4x-%.4x   [0x%.4lx] = %.8lld.%.8lld sec \n", 
	 myriad->latched_time_stamp[0],
	 myriad->latched_time_stamp[1],
	 myriad->latched_time_stamp[2],
	 (long)&myriad->live_time_stamp - (long)myriad,
	 time_stamp/100000000,time_stamp%100000000);
  time_stamp
    = ((unsigned long long)myriad->live_time_stamp[0] << 32) 
    + ((unsigned long long)myriad->live_time_stamp[1] << 16) 
    + ((unsigned long long)myriad->live_time_stamp[2] <<  0);
  printf("Live Time Stamp      : 0x%.4x-%.4x-%.4x   [0x%.4lx] = %.8lld.%.8lld sec \n", 
	 myriad->live_time_stamp[0],
	 myriad->live_time_stamp[1],
	 myriad->live_time_stamp[2],
	 (long)&myriad->latched_time_stamp - (long)myriad,
	 time_stamp/100000000,time_stamp%100000000);
  printf("Coin. Window Delay   : 0x%.4x [0x%.4lx] = %dx10 nsec         \n", 
	 myriad->coin_window_delay, (long)&myriad->coin_window_delay - (long)myriad, 
	 myriad->coin_window_delay);
  printf("Coin. Window Width   : 0x%.4x [0x%.4lx] = %dx10 nsec         \n", 
	 myriad->coin_window_width, (long)&myriad->coin_window_width - (long)myriad, 
	 myriad->coin_window_width);

  printf("TS Error Counter Ctrl: 0x%.4x 0x%.4x [0x%.4lx]                     \n", 
	 myriad->ts_error_counter_ctrl, myriad->ts_error_counter_rate, 
	 (long)&myriad->ts_error_counter_ctrl - (long)myriad);
  MyRIAD_TS_error_counter_ctrl_t ts_error_counter_ctrl;
  *(unsigned short*)&ts_error_counter_ctrl = myriad->ts_error_counter_ctrl;
  printf("  RESET:             : %d         = %s    \n", 
	 ts_error_counter_ctrl.reset, ts_error_counter_ctrl.reset ? "Yes":"No");
  printf("  MODE:              : %d         = %s    \n", 
	 ts_error_counter_ctrl.mode, ts_error_counter_ctrl.mode ? "Accumulate":"Rate");
  printf("  INHIBIT:           : %d         = %s    \n", 
	 ts_error_counter_ctrl.inhibit, ts_error_counter_ctrl.inhibit ? "Yes":"No");
  printf("  Rate Period:       : %d         x 10 nsec  \n", 
	 myriad->ts_error_counter_rate);
  printf("TS Error Counter     : 0x%.4x%.4x [0x%.4lx]                     \n", 
	 myriad->ts_error_counter[0], myriad->ts_error_counter[1], 
	 (long)&myriad->ts_error_counter - (long)myriad);

  printf("Trigger Time Offset  : 0x%.4x [0x%.4lx] = %dx10 nsec         \n", 
	 myriad->ttcl_time_offset, (long)&myriad->ttcl_time_offset - (long)myriad, 
	 myriad->ttcl_time_offset);

  printf("Miss Trig Count      : 0x%.4x [0x%.4lx]                     \n", 
	 myriad->missed_trig_counter, (long)&myriad->missed_trig_counter - (long)myriad);

  printf("Miss Trig Count TS   : 0x%.4x [0x%.4lx]                     \n", 
	 myriad->missed_trig_counter_ts_oe, (long)&myriad->missed_trig_counter_ts_oe - (long)myriad);

  printf("TS Propagation Ctrl  : 0x%.4x [0x%.4lx]                     \n", 
	 myriad->ts_propagation_control, (long)&myriad->ts_propagation_control - (long)myriad);
  MyRIAD_TS_propagation_control_t ts_propagation_control;
  *(unsigned short*)&ts_propagation_control = myriad->ts_propagation_control;
  printf("  DES:               : 0x%.2x              \n", 
	 ts_propagation_control.trig_des);
  printf("  SYNC:              : %d         = %s    \n", 
	 ts_propagation_control.sync, ts_propagation_control.sync ? "Yes":"No");

  printf("FIFO Counter         : 0x%.4x [0x%.4lx]  = %d       \n", 
	 myriad->fifo_counter, (long)&myriad->fifo_counter - (long)myriad,
	 myriad->fifo_counter);
  printf("Trig Counter         : 0x%.4x [0x%.4lx]  = %d       \n", 
	 myriad->trig_counter, (long)&myriad->trig_counter - (long)myriad,
	 myriad->trig_counter);
  for(i=0; i<8; i++){
  printf("User Counter NIM-%1d   : 0x%.4x [0x%.4lx]  = %d       \n", 
	 i,
	 myriad->user_counter[i], (long)&myriad->user_counter[i] - (long)myriad,
	 myriad->user_counter[i]);
  }

  printf("SerDes Config        : 0x%.4x 0x%.4x [0x%.4lx]  Serializer/Deserializer      \n", 
	 myriad->serdes_config, myriad->serdes_config, 
	 (long)&myriad->serdes_config - (long)myriad);
  MyRIAD_serdes_config_t serdes_config;
  *(unsigned short*)&serdes_config = myriad->serdes_config;
  printf("  DEN:               : %d           SerDes DEN\n", serdes_config.den);
  printf("  REN:               : %d           SerDes REN\n", serdes_config.ren);
  printf("  LINE LE:           : %d           SerDes Diagnostic Loop for Line \n", serdes_config.line_le);
  printf("  LOC LE:            : %d           SerDes Diagnostic Loop for Local\n", serdes_config.loc_le);
  printf("  SYNC:              : %d           SerDes Ignore TX\n", serdes_config.sync);
  printf("  T PWR*:            : %d           SerDes Transmitter Power\n", serdes_config.t_pwr);
  printf("  R PWR*:            : %d           SerDes Receiver Power\n", serdes_config.r_pwr);
  printf("  SL:                : %d           SerDes Stringent Lock for Monitoring Errs\n", serdes_config.sl);
  printf("  CLK SEL:           : %d           Local Board Oscillator Clock\n", serdes_config.clk_sel);

  unsigned short fifo_data;

  if(f_flag){
    printf("FIFO Data            : [0x%.4lx]  \n", (long)&myriad->fifo_access - (long)myriad);
    for(i=0; i<16; i++){
      printf("  %.4x: ", i*16);
      for(j=0; j<8; j++){
	//      fifo_data = ((unsigned short*)&myriad->fifo_access)[i*16+j];
	fifo_data = ((unsigned short*)&myriad->fifo_access)[0];
	printf("%.4x ", fifo_data);
      }
      printf("\n");
    }
  }
  
#if 0
  int offset = 0x0000;
  unsigned short *ptr;
  ptr = (unsigned short*)&((unsigned char*)myriad)[offset];
  printf("Dump                 : [0x%.4lx]  \n", (long)ptr - (long)myriad);
  for(i=0; i<16*16; i++){
    printf("  %.4lx: ", (long)ptr - (long)myriad);
    for(j=0; j<8; j++){
      printf("%.4x ", *ptr++);
    }
    printf("\n");
  }
#endif


  printf("--------------------------------------------------------------------------\n");

  return 0; 
}


