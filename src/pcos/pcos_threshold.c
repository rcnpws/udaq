/*
 *
 * pcos_threshold.c 
 *    Set pcos3 delay from 4299
 *
 *  Version 3.00   2006.04.06   by A. Tamii  (for Force)
 *  Version 4.00   2008.11.23   by A. Tamii  (for GeFanuc Linux)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "camlib.h"

#define DEBUG 0
#define SET_THRESHOLD 0x04

const double t_offset = 0.00;
const double t_ulimit = 7.65;
const double t_unit   = 0.03;

void usage(){
  fprintf(stderr,"Usage:pcos_threshold c n np nd thr\n");
	  fprintf(stderr,"\t c:    4299/RDTM crate number\n");
	  fprintf(stderr,"\t n:    4299/RDTM station number\n");
	  fprintf(stderr,"\t np:   PCOS-III Controler number\n");
	  fprintf(stderr,"\t nd:   2731 station number\n");
	  fprintf(stderr,"\t thr:  threshold (V) (0.00-7.65)\n");
	  exit(1);
}

int main(int argc, char **argv)
{
  
  unsigned long C, N, A, F;
  unsigned long Q, X;
  unsigned long data;
  unsigned long np,nd;
  double thr;
  long ithr;

  unsigned short* mem;
  int status=0;

  F = 16;
  A =  0;
  data = 0;

  if(argc != 6){
    usage();
    exit(-1);
  }
  
  if(argc == 6){              /* read operation       */
    C    = atoi(argv[1]);     /* crate number         */
    N    = atoi(argv[2]);     /* station number(4299) */
    np   = atoi(argv[3]);     /* PCOS III number      */
    nd   = atoi(argv[4]);     /* station number(2731) */
    thr  = atof(argv[5]);     /* threshold            */
  }

  if(C<0 || 8<=C){
    fprintf(stderr, 
      "4299/RDTM crate number (%ld) is out or range (0<=C<8)\n", C);
    exit(-1);
  }

  if(N<1 || 24<=N){
    fprintf(stderr, 
      "4299/RDM station number (%ld) is out or range (1<=N<24)\n", N);
    exit(-1);
  }

  if(np<0 || 16<=np){
    fprintf(stderr, 
      "PCOS number (%ld) is out or range (0<=np<16)\n", np);
    exit(-1);
  }

  if(nd<1 || 24<=nd){
    fprintf(stderr, 
      "2731 station number (%ld) is out or range (0<=nd<24)\n", nd);
    exit(-1);
  }

  if(thr<t_offset || t_ulimit<=thr){
    fprintf(stderr, 
      "Specified threshold (%5.3f) is out or range (%5.3f<=thr<%5.3f)\n",
      thr, t_offset, t_ulimit);
    exit(-1);
  }


  ithr = (int) ((thr-t_offset)/(t_unit)) & 0x00FF;

#if 1
  printf(" C=%1ld N=%2ld", C, N);
  printf(" pcos=%1ld station=%2ld thr=%5.3fV (0x%.2lx)\n",
    np, nd, thr, ithr);
#endif

  if((mem=camOpen())==NULL) {
    fprintf(stderr, "Error in camOpen().\n");
    exit(-1);
  }

  /* Clear Inhibit     */
  data = 0;
  if(DEBUG) printf("\n Clear Inhibit\n");
  status|=CNAF(mem, C, 30, 0, 17, &data, &Q, &X);
  if(DEBUG) printf(" Q=%1ld, X=%1ld\n\n", Q, X);
  usleep(10000);

  /* Send Clear to 4299     */
  if(DEBUG) printf(" Write Clear\n");
  status|=CNAF(mem, C, N, A, 9, &data, &Q, &X);
  if(DEBUG) printf(" Q=%1ld, X=%1ld\n\n", Q, X);
  usleep(10000);

  /* Send Word1 to 4299     */
  data = ((np<<10) | SET_THRESHOLD);
  if(DEBUG) printf(" Write Data = 0x%.4lx = %5ld\n", data, data);
  status|=CNAF(mem, C, N, A, F, &data, &Q, &X);
  if(DEBUG) printf(" Q=%1ld, X=%1ld\n", Q, X);
  usleep(10000);

  /* Send Word2 to 4299     */
  data = (0x8000 | ithr);
  if(DEBUG) printf(" Write Data = 0x%.4lx = %5ld\n", data, data);
  status|=CNAF(mem, C, N, A, F, &data, &Q, &X);
  if(DEBUG) printf(" Q=%1ld, X=%1ld\n", Q, X);
  usleep(10000);

  /* Send Word3 to 4299     */
  data = (0x8001 | (nd<<3));
  if(DEBUG) printf(" Write Data = 0x%.4lx = %5ld\n", data, data);
  status|=CNAF(mem, C, N, A, F, &data, &Q, &X);
  if(DEBUG) printf(" Q=%1ld, X=%1ld\n", Q, X);
  usleep(10000);

  camClose(mem);

  if(status){
    fprintf(stderr, "Error in CNAF().\n");
    return -1;
  }

  return 0;
}

