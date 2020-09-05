/*
 *
 *  nfa.c
 *  Simple nfa function for K2917-K3922
 *  2008.08.11   M. Uchida
 *  Version 1.10        2008-11-23      by A. Tamii (cleaned-up)
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "k2917.h"
#include "camlib.h"

#define MAX_RETRY   50   
#define RETRY_WAIT  5L

char *program_name;

void usage(){
  printf("Usage: %s n f a [data] c\n", program_name); 
}

int main(int argc, char *argv[])
{
  unsigned long  C, N, A, F, Q, X;
  unsigned long  data=0;
  struct K_REG *k2917;
  char c;

  program_name = *argv;

  if(argc<5 || argc>6){
    usage();
    exit(-1);
  }else if(argc==5){
    C = atoi(argv[4]);
  }else{
    C = atoi(argv[5]);
  }

  N = atoi(argv[1]);
  A = atoi(argv[3]);
  F = atoi(argv[2]);

  if(C<0 || 7<C){
    fprintf(stderr, "Illegal value for C (%ld)\n", C);
    usage();
    exit(-1);
  }
  if(N<1 || (24<N && N!=30)){
    fprintf(stderr, "Illegal value for N (%ld)\n", N);
    usage();
    exit(-1);
  }
  if(A<0 || 15<A){
    fprintf(stderr, "Illegal value for A (%ld)\n", A);
    usage();
    exit(-1);
  }
  if(F<0 || 31<F){
    fprintf(stderr, "Illegal value for F (%ld)\n", F);
    usage();
    exit(-1);
  }

  if(argc==6){
    c = tolower(argv[4][0]);
    switch(c){
    case '0':
      if(argv[4][1] == 'x'){ /* Hex input   */
	sscanf(argv[4],"0x%lx",&data);
      } else {               /* Octal input */
	sscanf(argv[4],"0%lo",&data);
      }
      break;
    case 'h': /* Hex input   */
      sscanf(++argv[4],"%lx",&data);
      break;
    case 'b': /* Binary input */
      data = 0;
      while((c=*(++argv[4])) != 0){
	if(c=='1'||c=='0'){
	  data = (data<<1) | (c-'0');
	} else {
	  usage();
	  exit(-1);
	}
      }
      break;
    default: /* Decimal input */
      if(sscanf(argv[4],"%ld",&data) != 1){
	usage(); 
	exit(-1);
      }
    }
  }else{
    data = 0;
  }

  if((k2917 = camOpen()) == NULL){
    fprintf(stderr,"Can't open camac\n");
    exit(1);
  }

  if(CNAF(k2917,C,N,A,F,&data,&Q,&X)){
    printf("Error in CNAF.\n");
  }

  printf("  N=%2ld F=%2ld A=%2ld C=%1ld Q=%ld X=%ld  Data:0x%.4lx (%ld)\n",
         N, F, A, C, Q, X, data&0x00ffffff, data&0x00ffffff);
  
  camClose(k2917);

  return(0);
}
