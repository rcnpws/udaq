/* mluDump.c    dump MLU Memory                                         */
/*                                                                      */
/* Copyright (C) 1994, Atsushi Tamii                                    */
/*      This program was made at RCNP.                                  */
/*                                                                      */
/*  Version 1.00        1994-07-02      by Atsushi Tamii                */
/*  Version 2.00        2008-11-23      by Atsushi Tamii for GeFanuc    */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "camlib.h"

#define Dim16  0x00    /* Dimension = 16bit */
#define Dim15  0x01    /* Dimension = 15bit */
#define Dim14  0x02    /* Dimension = 14bit */
#define Dim13  0x03    /* Dimension = 13bit */
#define Dim12  0x04    /* Dimension = 12bit */

#define ModeStrobe     0x00    /* Strobe Mode */
#define ModeTrans      0x08    /* Transparent Mode */
#define ModeInhibit    0x10    /* Inhibit Mode */

#define Dim    Dim16
#define Mode   ModeStrobe

#define DEBUG  0

/*#main--- main function */
int main( argc, argv )
  short           argc;
  unsigned char   *argv[];
{
  short         x1,x2;
  long          i;

  unsigned long C,N,A,F;
  unsigned long Q,X;
  unsigned long data;

  unsigned short *mem;
  int  status = 0;

  if(argc!=3){
    printf( "Usage:%s crate station \n", argv[0] );
    printf( "  crate  .... Crate number\n" );
    printf( "  station ... Station number of 4299 or RDTM\n" );
    exit(-1);
  }

  C = atoi(argv[1]);
  N = atoi(argv[2]);

  printf("Dump MLU Memory at Crate= %ld Station=%ld.\n", C, N);

  if((mem=camOpen())==NULL) {
    fprintf(stderr, "Error in camOpen().\n");
    exit(-1);
  }

  /* Inhibit */
  data = ModeInhibit|Dim;
  if(DEBUG) printf("\n Inhibit\n");
  status=CNAF(mem, C, N, 2, 16, &data, &Q, &X);
  if(DEBUG) printf(" Q=%1ld, X=%1ld\n\n", Q, X);
  if(status){
    fprintf(stderr, "\nError on setting Inhibit.\n" );
    exit(-1);
  }
  usleep(10000);

  /* set address register */
  data = 0;
  if(DEBUG) printf("\n Set Address Register\n");
  status=CNAF(mem, C, N, 1, 16, &data, &Q, &X);
  if(DEBUG) printf(" Q=%1ld, X=%1ld\n\n", Q, X);
  if(status){
    fprintf(stderr, "\nError on setting the address register.\n" );
    exit(-1);
  }
  usleep(10000);

  A = 0;
  F = 0;
  for(x1=0;x1<0x100;x1++){
    for(x2=0;x2<0x100;x2+=8){
      printf("%.2X:%.2X ",x1,x2);
      for(i=0;i<0x08;i++){
        status = CNAF(mem, C, N, A, F, &data, &Q, &X );
	if(DEBUG) printf(" Q=%1ld, X=%1ld\n\n", Q, X);
	if(status){
	  fprintf(stderr, "\nError on reading from MLU.\n" );
	  camClose(mem);
	  x1=x2=0x100;
          break;
        }
        printf("%4lx ",data&0x0000FFFF);
      }
      printf("\n");
    }
    printf("\n");
  }
  
  /* Clear Inhibit */
  data = Mode|Dim;
  if(DEBUG) printf("\n Clear inhibit\n");
  status=CNAF(mem, C, N, 2, 16, &data, &Q, &X);
  if(DEBUG) printf(" Q=%1ld, X=%1ld\n\n", Q, X);
  if(status){
    fprintf(stderr, "\nError on clearing Inhibit.\n" );
    exit(-1);
  }
  usleep(10000);

  camClose(mem);

  return 0;
}


