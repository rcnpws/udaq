/*
 *
 * pcos_setaddress
 *    Set logical address of PCOS-III
 *
 *  Version 3.00   2006.04.07   by A. Tamii  (for Force)
 *  Version 4.00   2008.11.23   by A. Tamii  (for GeFanuc Linux)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "camlib.h"

#define DEBUG 0
#define SET_ADDRESS 0x02

void usage(){
  fprintf(stderr,"Usage:pcos_setaddress c n np nd adderss\n");
  fprintf(stderr,"\t c:    4299/RDTM crate number\n");
  fprintf(stderr,"\t n:    4299/RDTM station number\n");
  fprintf(stderr,"\t np:   PCOS-III Controler number\n");
  fprintf(stderr,"\t nd:   2731 station number\n");
  fprintf(stderr,"\t address:  logical address. Use 0x... for Hex.\n");
}

int main(int argc, char **argv)
{
  
  unsigned long C, N, A, F;
  unsigned long Q, X;
  unsigned long data;
  unsigned long np,nd;
  unsigned long addr;
  char *p;

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
    p    = argv[5];           /* logical address      */

    switch(p[0]){
    case '0':
      switch(p[1]){
      case 'x':
	if(sscanf(p,"0x%lx",&addr)!=1){
	  fprintf(stderr, "Error input in the logical address (%s).\n", p);
	  exit(-1);
	}
	break;
      default:
        if(p[1]==0 || ('0'<=p[1] && p[1]<='9')){
	  addr = atoi(p);
	  break;
	}
	fprintf(stderr, "Error input in the logical address (%s).\n", p);
	exit(-1);
      }
      break;
    default: /* Decimal input */
      addr = atoi(p);
      break;
    }

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

  if(addr<0 || 0x01FF<addr){
    fprintf(stderr, 
      "Specified address (0x%.2ld=%ld) is out or range (0<=addr<=0x01FF)\n",
      addr,addr);
    exit(-1);
  }

#if 1
  printf(" C=%1ld N=%2ld", C, N);
  printf(" pcos=%ld station=%2ld addr=0x%.3lx (%5ld)\n",
    np, nd, addr, addr);
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
  data = ((np<<10) | SET_ADDRESS);
  if(DEBUG) printf(" Write Data = 0x%.4lx = %5ld\n", data, data);
  status|=CNAF(mem, C, N, A, F, &data, &Q, &X);
  if(DEBUG) printf(" Q=%1ld, X=%1ld\n", Q, X);
  usleep(10000);

  /* Send Word2 to 4299     */
  data = (0x8000 | (addr&0x00ff));
  if(DEBUG) printf(" Write Data = 0x%.4lx = %5ld\n", data, data);
  status|=CNAF(mem, C, N, A, F, &data, &Q, &X);
  if(DEBUG) printf(" Q=%1ld, X=%1ld\n", Q, X);
  usleep(10000);

  /* Send Word3 to 4299     */
  data = (0x8000 | (nd<<3) | ((addr>>8)&0x0001));
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

