/*
 *
 * pcos_delay.c 
 *    Set pcos3 delay from 4299
 *
 *    Modified 06-APR-2006 by A.Tamii
 */

#include <stdio.h>
#include <stdlib.h>
#include "camlib.h"

#define DEBUG 0
#define SET_DELAY 0x0c

#define t_offset   (300.0)
#define t_ulimit   (682.0)
#define t_unit     (((t_ulimit)-(t_offset))/255.)

void usage(){
  fprintf(stderr,"Usage:pcos_threshold c n np nd delay\n");
	  fprintf(stderr,"\t c:    4299/RDTM crate number\n");
	  fprintf(stderr,"\t n:    4299/RDTM station number\n");
	  fprintf(stderr,"\t np:   PCOS-III Controler number\n");
	  fprintf(stderr,"\t nd:   2731 station number\n");
	  fprintf(stderr,"\t delay: delay (in nsec) (300-682)\n");
	  exit(1);
}

int main(int argc, char **argv)
{
  
  int q,x,status,np,nd;
  nfa_t nfa;
  char c;
  double delay;
  int idelay;

  nfa.f = 16;
  nfa.a =  0;
  nfa.data = 0;

  if(argc == 6){              /* read operation       */
    nfa.c= atoi(argv[1]);     /* crate number         */
    nfa.n= atoi(argv[2]);     /* station number(4299) */
    np   = atoi(argv[3]);     /* PCOS III number      */
    nd   = atoi(argv[4]);     /* station number(2731) */
    delay= atof(argv[5]);     /* delay in nsec        */
  } else {
    usage();
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

  if(delay<(t_offset) || (t_ulimit)<delay){
    fprintf(stderr, 
      "Specified threshold (%5.3f) is out or range (%5.3f<=delay<=%5.3f)\n",
      delay, t_offset, t_ulimit);
    exit(-1);
  }


  idelay = (int) ((delay-(t_offset))/(t_unit)) & 0x00FF;

#if 1
  printf(" c=%1d n=%2d", nfa.c, nfa.n);
  printf(" pcos=%1d station=%2d delay=%5.1fnsec(0x%.2x)",
    np, nd, delay, idelay);
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
  nfa.data = ((np<<10) | SET_DELAY);
  if(DEBUG) printf(" Write Data = 0x%.4x = %5d\n", nfa.data, nfa.data);
  status=CAMAC(NAF(nfa.n, nfa.a, nfa.f),&nfa.data, &q, &x);
  if(DEBUG) printf(" Q=%1d, X=%1d\n", q, x);
  usleep(10000);

  /* Send Word2 to 4299     */
  nfa.data = (0x8000 | idelay);
  if(DEBUG) printf(" Write Data = 0x%.4x = %5d\n", nfa.data, nfa.data);
  status=CAMAC(NAF(nfa.n, nfa.a, nfa.f),&nfa.data, &q, &x);
  if(DEBUG) printf(" Q=%1d, X=%1d\n", q, x);
  usleep(10000);

  /* Send Word3 to 4299     */
  nfa.data = (0x8000 | (nd<<3));
  if(DEBUG) printf(" Write Data = 0x%.4x = %5d\n", nfa.data, nfa.data);
  status=CAMAC(NAF(nfa.n, nfa.a, nfa.f),&nfa.data, &q, &x);
  printf(" Q=%1d, X=%1d\n", q, x);
  usleep(10000);

  CAM_Close();
}

