/* mluLoad_uv.c	download MLU Memory (Model 2373) dU vs dV Cut   */
/*								*/
/* Copyright (C) 1994, Atsushi Tamii				*/
/* 	This program was made at RCNP.				*/
/*								*/
/*  Version 0.00	1994-07-02	by Atsushi Tamii	*/
/*  Version 0.10	1996-02-25	by Atsushi Tamii	*/
/*  Version 0.20        1996-11-22      by Hidetoshi Akimune    */
/*                       Modified for X, U', V' cut             */
/*  Version 2.00        2008-11-23      by A. Tamii for GeFanuc */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "camlib.h"


// for MLU 2372

#define Dim16	0x00	/* Dimension = 16bit */
#define Dim15	0x01	/* Dimension = 15bit */
#define Dim14	0x02	/* Dimension = 14bit */
#define Dim13	0x03	/* Dimension = 13bit */
#define Dim12	0x04	/* Dimension = 12bit */

#define ModeStrobe	0x00	/* Strobe Mode */
#define ModeTrans	0x08	/* Transparent Mode */
#define ModeInhibit	0x10	/* Inhibit Mode */

#define Dim	Dim16
#define Mode	ModeStrobe

#define DEBUG  0

static int r2;
static int du[16];
static int dv[16];

/*#initData*/
void initData()
{
  int k;
  int r;
  printf("Input r:");
  scanf("%d",&r);
  r2 = r*r;
  k=0;
  printf("Input offset U and V\n");

  printf("Offset for U at X=-8: ");
  scanf("%d",&du[8]);
  printf("             at X= 7: ");
  scanf("%d",&du[7]);
  printf("Offset for V at X=-8: ");
  scanf("%d",&dv[8]);
  printf("             at X= 7: ");
  scanf("%d",&dv[7]);
        
  for(k=1;k<8;k++){
    du[7-k]=du[7]-(du[7]-du[8])*k/15;
    du[8+k]=du[8]+(du[7]-du[8])*k/15;
    dv[7-k]=dv[7]-(dv[7]-dv[8])*k/15;
    dv[8+k]=dv[8]+(dv[7]-dv[8])*k/15;
  } 
  for(k=-8;k<0;k++){
    printf(" x=%d offset U:%d V:%d\n",k,du[k+16],dv[k+16]);
  }
  for(k=0;k<8;k++){
    printf(" x=%d offset U:%d V:%d\n",k,du[k],dv[k]);
  } 
}

/*#getDataFromWindow ---- get data (1bit) from window (=seppen)*/
short getDataFromWindow(x1,x2)
  short		x1,x2;
{
  int  d;
  d = x1*x1+x2*x2;
  if(d>=r2) return 1;
  return 0;
}

/*#getData---- get data to load from X1 and X2 */
short getData(u3,v3,x)
  short		u3,v3,x;
{
  short  u0,v0;
  u0=u3-du[x];
  v0=v3-dv[x];
  if(getDataFromWindow(u0, v0))
    return 1;
  else
    return 0;
}


/*#main--- main function */
int main( argc, argv )
  short           argc;
  unsigned char   *argv[];
{
  long  	i;
  short         u, v, j;

  unsigned long  C,N,A,F;
  unsigned long  Q,X;
  unsigned long	data;

  int status = 0;
  unsigned short *mem;

  if(argc!=3){
    printf( "Usage:%s crate station \n", argv[0] );
    printf( "  crate  .... Crate number\n" );
    printf( "  station ... Station number of 4299 or RDTM\n" );
    exit(-1);
  }

  C = atoi(argv[1]);
  N = atoi(argv[2]);

  printf("MLU Load Program\n");

  initData();
  
  printf( "Load MLU Memory at Crate= %ld Station=%ld.\n", C, N );
  printf( "r*r = %d\n", r2 );


  if((mem=camOpen())==NULL) {
    fprintf(stderr, "Error in camOpen().\n");
    exit(-1);
  }

  /* Inhibit */
  //data = ModeInhibit|Dim;
  data = ModeInhibit|Dim12;  // Dim12 is required. See the manual.
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

  printf("Loading ...\n");

  F = 16;
  A = 0;
  //printf("  ---> U4-U3\n");
  //printf("\n V4-V3\nV\n  ");
  for(i=0;i<0x1000;i++){
    u=i&0x03f;
    v=(i>>6)&0x3f;
    if(u>=0x20) u -= 0x40;
    if(v>=0x20) v -= 0x40;
    data = 0;
    for(j=0;j<16;j++){
      if(getData(u,v,j))
	data |= 1<<j;
    }
    printf("%lx", (data & 0x100)>>8 );
    if((i & 0x3f)==0x3f) printf("\n");

    //    data = 0x0000;

    if(DEBUG) printf(" data =%.4lX\n", data);
    status = CNAF(mem, C, N, F, A, &data, &Q, &X);
    if(DEBUG) printf(" Q=%1ld, X=%1ld\n\n", Q, X);
    if(status){
      fprintf(stderr, "\nError on writing to the MLU Memory.\n");
      break;
    }
    usleep(10000);
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

  printf("Finished\n ");

  camClose(mem);

  return 0;
}


