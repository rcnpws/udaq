/* mluLoad_uv.c	download MLU Memory (Model 2373) dU vs dV Cut   */
/*								*/
/* Copyright (C) 1994, Atsushi Tamii				*/
/* 	This program was made at RCNP.				*/
/*								*/
/*  Version 0.00	1994-07-02	by Atsushi Tamii	*/
/*  Version 0.10	1996-02-25	by Atsushi Tamii	*/

# include <stdio.h>
# include <stdlib.h>
# include <errno.h>
# include <unistd.h>

#include "camlib.h"

# define Dim16	0x00	/* Dimension = 16bit */
# define Dim15	0x01	/* Dimension = 15bit */
# define Dim14	0x02	/* Dimension = 14bit */
# define Dim13	0x03	/* Dimension = 13bit */
# define Dim12	0x04	/* Dimension = 12bit */

# define ModeStrobe	0x40	/* Strobe Mode */
# define ModeTrans	0x48	/* Transparent Mode */
# define ModeInhibit	0x50	/* Inhibit Mode */

# define Dim	Dim16
# define Mode	ModeStrobe

#define DEBUG 0

static int r, r2;
static int ofs1;
static int ofs2;

/*#initData*/
void initData()
{
  printf("Input r:");
  scanf("%d",&r);
  r2 = r*r;
  printf("Input offset U:");
  scanf("%d",&ofs1);
  printf("Input offset V:");
  scanf("%d",&ofs2);
}

/*#getDataFromWindow ---- get data (1bit) from window (=seppen)*/
int getDataFromWindow(x1,x2)
  short		x1,x2;
{
  int  d;
  d = x1*x1+x2*x2;
  if(d>=r2) return 1;
  return 0;
}

/*#getData---- get data to load from X1 and X2 */
short getData(x1,x2)
  short		x1,x2;
{
  short		data;
  data = 0;
  x1 = (x1-ofs1) & 0x00FF;
  x2 = (x2-ofs2) & 0x00FF;
  if(x1>=128) x1 -= 256;
  if(x2>=128) x2 -= 256;
    if(getDataFromWindow(x1,x2))
      data |= 0xFFFF;
    else
      data = 0;
  return data;
}

/*#main--- main function */
int main( argc, argv )
  short           argc;
  unsigned char   *argv[];
{
  unsigned long	data;
  long  	i, j;
  unsigned long C, N, A, F, Q, X;
  int           status;
  unsigned short *mem;

  if(argc!=3){
    printf( "Usage:%s crate station \n", argv[0] );
    printf( "  crate  .... Crate number\n" );
    printf( "  station ... Station number of 4299 or RDTM\n" );
    return(-1);
  }

  C = atoi(argv[1]);
  N = atoi(argv[2]);

  printf("MLU Load Program\n");

  initData();
  
  printf( "Load MLU Memory at Crate= %ld Station=%ld.\n", C, N );
  printf( "r*r = %d\n", r2 );


  if((mem=camOpen())==NULL) {
    fprintf(stderr, "Error in camOpen().\n");
    return(-1);
  }

  /* Inhibit */
  //data = ModeInhibit|Dim;
  data = ModeInhibit|Dim12;  // Dim12 is required. See the manual.
  if(DEBUG) printf("\n Inhibit\n");
  status=CNAF(mem, C, N, 2, 16, &data, &Q, &X);
  if(DEBUG) printf(" Q=%1ld, X=%1ld\n\n", Q, X);
  if(status){
    fprintf(stderr, "\nError on setting Inhibit.\n" );
    return(-1);
  }
  usleep(10000);

  /* set address register */
  data = 0;
  if(DEBUG) printf("\n Set Address Register\n");
  status=CNAF(mem, C, N, 1, 16, &data, &Q, &X);
  if(DEBUG) printf(" Q=%1ld, X=%1ld\n\n", Q, X);
  if(status){
    fprintf(stderr, "\nError on setting the address register.\n" );
    return(-1);
  }
  usleep(10000);

  printf("Loading ...\n");

  F = 16;
  A = 0;

  for(i=0;i<0x1000;i++){
    data = 0;
    for(j=0;j<16;j++){
      if(getData(i&0x00FF,((i>>8)&0x000F)|(j<<4)))
        data |= 1<<j;
    }
    status = CNAF(mem, C, N, F, A, &data, &Q, &X);
    if(status){
      fprintf(stderr, "\nError on writing to the MLU Memory.\n");
      break;
    }
    usleep(10000);
  }
  printf("Completed\n ");
  
  /* Clear Inhibit */
  data = Mode|Dim;
  if(DEBUG) printf("\n Clear inhibit\n");
  status=CNAF(mem, C, N, 2, 16, &data, &Q, &X);
  if(DEBUG) printf(" Q=%1ld, X=%1ld\n\n", Q, X);
  if(status){
    fprintf(stderr, "\nError on clearing Inhibit.\n" );
    return(-1);
  }

  printf("Finished\n ");

  camClose(mem);

  return 0;
}


