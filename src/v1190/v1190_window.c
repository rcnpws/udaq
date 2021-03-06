/* v1190_window.c ---- set trigger window of  V119                     */
/*						                       */
/*  Version 1.00        2013-2-05      by oiwa (For Linux2.6)GE-FANUC  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1190.h"

int main(int argc, char *argv[]){
  int ret;
  int module_number = 0;

  int res;
  int t_clock = 25;
  unsigned short match_window_width = 1000/t_clock; // from 1 to 4095
  short window_offset = -100/t_clock; // from -2048 to 40
  unsigned short extra_search_window_width = 0/t_clock; // no greater than 50
  unsigned short reject_margin = 100/t_clock; // should be >= 1
  //  unsigned short trigger_time_subtraction = 0; // 1 : enabled
  unsigned short trigger_time_subtraction = 1; // 1 : enabled ///added for test 17/04/17 by Furuno 
 

  
  V1190_p v1190=(V1190_p)NULL;

  if(argc>1 && !strcmp(argv[1],"-h")){
    fprintf(stderr, "%s ... show v1190 status.\n", argv[0]);
    fprintf(stderr, "Usage: %s [module_number]\n", argv[0]);
    fprintf(stderr, " module_number: the default is 0.\n");
    exit(0);
  }

  if(argc>1){
    module_number = atoi(argv[1]);
  }

  /* ------------- Open ------------ */

  fprintf(stderr, "V1190A base address = 0x%.8x\n", V1190_BASE(module_number));

  fprintf(stderr, "open V1190A\n");
  ret = v1190_open();
  if(ret!=0){
    fprintf(stderr, "Error in v1190_open()\n");
    exit(-1);
  }

  fprintf(stderr, "map V1190A(module_number=%d)\n", module_number);
  v1190 = v1190_map(module_number);
  if(v1190==(V1190_p)NULL){
    fprintf(stderr, "Error in v1190_map()\n");
    v1190_close();
    exit(-1);
  }

  fprintf(stderr, "v1190 = 0x%.8lx\n", (long)v1190);

  /* ----------------job-------------- */

  v1190_micro_handshake_write(v1190, V1190_OPECODE_SET_WIN_WIDTH);
  sleep(1);
  v1190_micro_handshake_write(v1190, match_window_width);
  sleep(1);

  v1190_micro_handshake_write(v1190, V1190_OPECODE_SET_WIN_OFFS);
  sleep(1);
  v1190_micro_handshake_write(v1190, window_offset);
  sleep(1);
  
  v1190_micro_handshake_write(v1190, V1190_OPECODE_SET_SW_MARGIN);
  sleep(1);
  v1190_micro_handshake_write(v1190, extra_search_window_width);
  sleep(1);

  v1190_micro_handshake_write(v1190, V1190_OPECODE_SET_REJ_MARGIN);
  sleep(1);
  v1190_micro_handshake_write(v1190, reject_margin);
  sleep(1);



  //need to be fixed!!!!!
    printf("hoger hugae kuma\n");// added on 17/04/17 by Furuno
  if(trigger_time_subtraction==1) {
    v1190_micro_handshake_write(v1190, V1190_OPECODE_EN_SUB_TRG);
    sleep(1);// added on 17/04/17 by Furuno
    v1190_micro_handshake_write(v1190, 1);   // added on 17/04/17 by Furuno
    sleep(1);// added on 17/04/17 by Furuno
    printf("hoger\n");// added on 17/04/17 by Furuno
  }// added on 17/04/17 by Furuno

  if(trigger_time_subtraction==0) 
  v1190_micro_handshake_write(v1190, V1190_OPECODE_DIS_SUB_TRG);
  //
  

  res = v1190_micro_handshake_read(v1190, V1190_OPECODE_READ_TRG_CONF, &match_window_width);
  if(res){
    fprintf(stderr, "Error in v1190_micro_handshake_read for READ_TRG_CONF #1 (%d)\n", res);
  }
  res = v1190_micro_handshake_read_cont(v1190, &window_offset);
  if(res){
    fprintf(stderr, "Error in v1190_micro_handshake_read_cont for READ_TRG_CONF #2 (%d)\n", res);
  }
  res = v1190_micro_handshake_read_cont(v1190, &extra_search_window_width);
  if(res){
    fprintf(stderr, "Error in v1190_micro_handshake_read_cont for READ_TRG_CONF #3 (%d)\n", res);
  }
  res = v1190_micro_handshake_read_cont(v1190, &reject_margin);
  if(res){
    fprintf(stderr, "Error in v1190_micro_handshake_read_cont for READ_TRG_CONF #4 (%d)\n", res);
  }
  res = v1190_micro_handshake_read_cont(v1190, &trigger_time_subtraction);
  if(res){
    fprintf(stderr, "Error in v1190_micro_handshake_read_cont for READ_TRG_CONF #5 (%d)\n", res);
  }

  printf("--------------------------------------------------------------------------\n");
  printf("Trigger Configuration:\n");
  printf("  match window width         = %.4x (%d nsec)\n", match_window_width, match_window_width*t_clock);
  printf("  window offset              = %.4x (%d nsec)\n", window_offset, window_offset*t_clock);
  printf("  extra search window width  = %.4x (%d nsec)\n", extra_search_window_width, extra_search_window_width*t_clock);
  printf("  reject_margin              = %.4x (%d nsec)\n", reject_margin, reject_margin*t_clock);
  printf("  trigger_time_subtraction   = %.4x (%s)\n", trigger_time_subtraction, (trigger_time_subtraction&0x0001) ? "enabled":"disabled");
  printf("--------------------------------------------------------------------------\n");

  /* ------------- Close ------------ */
  fprintf(stderr, "unmap V1190A\n");
  ret = v1190_unmap(module_number);
  if(ret!=0){
    fprintf(stderr, "Error in v1190_unmap()\n");
  }

  fprintf(stderr, "close V1190A\n");
  ret = v1190_close();
  if(ret!=0){
    fprintf(stderr, "Error in v1190_close()\n");
    exit(-1);
  }

  return 0; 
}


