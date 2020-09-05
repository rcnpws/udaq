/*
 *
 * pcos_setaddress
 *    Set logical address of PCOS-III
 *
 *  2006.04.07 by A. Tamii
 */

#include <stdio.h>
#include <stdlib.h>
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
  
  int q,x,status,np,nd;
  nfa_t nfa;
  char *p;
  unsigned int  addr;

  nfa.f = 16;
  nfa.a =  0;
  nfa.data = 0;

  if(argc != 6){
    usage();
    exit(-1);
  }
  
  if(argc == 6){              /* read operation       */
    nfa.c= atoi(argv[1]);     /* crate number         */
    nfa.n= atoi(argv[2]);     /* station number(4299) */
    np   = atoi(argv[3]);     /* PCOS III number      */
    nd   = atoi(argv[4]);     /* station number(2731) */
    p    = argv[5];           /* logical address      */

    switch(p[0]){
    case '0':
      switch(p[1]){
      case 'x':
	if(sscanf(p,"0x%x",&addr)!=1){
	  fprintf(stderr, "Error input in the logical address (%s).\n", p);
	  exit(-1);
	}
	break;
      default:
        if(p[1]==0 || '0'<=p[1] && p[1]<='9'){
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

  if(nfa.c<0 || 8<=nfa.c){
    fprintf(stderr, 
      "4299/RDTM crate number (%d) is out or range (0<=C<8)\n", nfa.c);
    exit(-1);
  }

  if(nfa.n<1 || 24<=nfa.n){
    fprintf(stderr, 
      "4299/RDM station number (%d) is out or range (1<=N<24)\n", nfa.n);
    exit(-1);
  }

  if(np<0 || 16<=np){
    fprintf(stderr, 
      "PCOS number (%d) is out or range (0<=np<16)\n", np);
    exit(-1);
  }

  if(nd<1 || 24<=nd){
    fprintf(stderr, 
      "2731 station number (%d) is out or range (0<=nd<24)\n", nd);
    exit(-1);
  }

  if(addr<0 || 0x01FF<addr){
    fprintf(stderr, 
      "Specified address (0x%.2d=%d) is out or range (0<=addr<=0x01FF)\n",
      addr,addr);
    exit(-1);
  }

#if 1
  printf(" c=%1d n=%2d", nfa.c, nfa.n);
  printf(" pcos=%1d station=%2d addr=0x%.3x(%5d)",
    np, nd, addr, addr);
#endif

  if (CAM_Open()) {
    perror("CAM_Open: ");
    exit(-1);
  }

  /* Set crate number */

  CSETCR(nfa.c);

  /* Clear Inhibit     */
  nfa.data = 0;
  if(DEBUG) printf("\n Clear Inhibit\n");
  status=CAMAC(NAF(30, 0, 17), &nfa.data, &q, &x);
  if(DEBUG) printf(" Q=%1d, X=%1d\n\n", q, x);
  usleep(10000);

  /* Send Clear to 4299     */
  if(DEBUG) printf(" Write Clear\n");
  status=CAMAC(NAF(nfa.n, nfa.a, 9),&nfa.data, &q, &x);
  if(DEBUG) printf(" Q=%1d, X=%1d\n\n", q, x);
  usleep(10000);


  /* Send Word1 to 4299     */
  nfa.data = ((np<<10) | SET_ADDRESS);
  if(DEBUG) printf(" Write Data = 0x%.4x = %5d\n", nfa.data, nfa.data);
  status=CAMAC(NAF(nfa.n, nfa.a, nfa.f),&nfa.data, &q, &x);
  if(DEBUG) printf(" Q=%1d, X=%1d\n", q, x);
  usleep(10000);

  /* Send Word2 to 4299     */
  nfa.data = (0x8000 | (addr&0x00ff));
  if(DEBUG) printf(" Write Data = 0x%.4x = %5d\n", nfa.data, nfa.data);
  status=CAMAC(NAF(nfa.n, nfa.a, nfa.f),&nfa.data, &q, &x);
  if(DEBUG) printf(" Q=%1d, X=%1d\n", q, x);
  usleep(10000);

  /* Send Word3 to 4299     */
  nfa.data = (0x8000 | (nd<<3) | ((addr>>8)&0x0001));
  if(DEBUG) printf(" Write Data = 0x%.4x = %5d\n", nfa.data, nfa.data);
  status=CAMAC(NAF(nfa.n, nfa.a, nfa.f),&nfa.data, &q, &x);
  printf(" Q=%1d, X=%1d\n", q, x);
  usleep(10000);

  CAM_Close();
}

