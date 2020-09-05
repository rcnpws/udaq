/* mluLoad2372.c download MLU Memory (Model 2372)   		*/
/*								*/
/* Copyright (C) 1994, Atsushi Tamii				*/
/* 	This program was made at RCNP.				*/
/*								*/
/*  Version 0.00	1994-07-02	by Atsushi Tamii	*/
/*  Version 1.00	1996-06-20	by Atsushi Tamii	*/

# include <stdio.h>
# include <errno.h>
# include "K3922drv.h"
# include "K3922.def"

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

static int csth;
static int snth;
static int d0;
static int d1;
static int ofs1;
static int ofs2;

/*#initData*/
void initData(th,y0)
  int    th;
  int    y0;
{
  printf("Input sin th *1000:");
  scanf("%d",&snth);
  printf("Input cos th *1000:");
  scanf("%d",&csth);
  printf("Input offset 1:");
  scanf("%d",&ofs1);
  printf("Input offset 2:");
  scanf("%d",&ofs2);
  d0 = abs(csth*y0);
  d1 = abs(csth*(256-y0));
}

/*#getDataFromWindow ---- get data (1bit) from window (=seppen)*/
Boolean getDataFromWindow(x1,x2)
  short		x1,x2;
{
  int  d;
#if 0
  if(x1 & 0x20) return true;
  return false;
#endif
  d = abs(-snth*x1+csth*x2);
  if(d0<=d && d<=d1) return true;
  return false;
}

/*#getData---- get data to load from X1 and X2 */
short getData(x1,x2)
  short		x1,x2;
{
  short		i;
  short		data;
  data = 0;
  x1 = (x1-ofs1) & 0x00FF;
  x2 = (x2-ofs2) & 0x00FF;
  if(x1>=128) x1 -= 256;
  if(x2>=128) x2 -= 256;
  if(getDataFromWindow(x1,x2))
    data = 0xFFFF;
  else
    data = 0;
  return data;
}

/*#main--- main function */
main( argc, argv )
  short           argc;
  unsigned char   *argv[];
{
  short		error;
  short		crate;
  short		station;
  unsigned char	str[256];
  unsigned short  data, data2;
  long  	i, j;
  int           tnth, th, y0;
  int           count;

  if( argc>1 ){
    printf( "Usage:%s\n", argv[0] );
    printf( "  Crate Number ..... CRATE\n" );
    printf( "  Station Number ... StMLU\n" );
    exit(1);
  }
  crate = atoi(getenv("CRATE"));
  station = atoi(getenv("StMLU"));
  
  printf("MLU Load Program\n");

  printf( "Load MLU Memory at Crate= %d Station=%d.\n", crate, station );

#if 0
  ofs1=ofs2=y0=0;
  csth=snth=1;
#else
  printf("Input seppen [0<=seppen<=256]:");
  scanf("%d",&y0);
  initData(th,y0);
  
  printf( "seppen=%d\n", y0 );
  printf( "cos th = %d, sin th =%d\n", csth, snth );
  printf( "offset1= %d, offset2=%d\n", ofs1, ofs2 );
#endif

  error  = initK2917();			/* initialize K2917 and driver */
  if( error ){
    printf( "Error in initK2917().\n" );
    k3922ShowError( error );
    exit(1);
  }
  k2917SetCrate(crate);			/* set crate number */
  error = resetK2917();
  if( error ){
    printf( "Error in resetK3922().\n" );
    k3922ShowError( error );
    exitK2917();
    exit(1);
  }
  error = resetK3922();			/* reset K3922 */
  if( error ){
    printf( "Error in resetK3922().\n" );
    k3922ShowError( error );
    exitK2917();
    exit(1);
  }
  
  error = k3922Write16(station,16,2,ModeInhibit|Dim); /* Inhibit */
  if( error&&error!=k3922NQErr ){
    printf( "Error in k3922Write16().\n" );
    k3922ShowError( error );
    exitK2917();
    exit(1);
  }

  printf("Loading \n");
  error = k3922Write16(station,16,1,0);	/* set address register */
  if( error&&error!=k3922NQErr ){
    printf( "Error in k3922Write16().\n" );
    k3922ShowError( error );
    exitK2917();
    exit(1);
  }
  for(i=0;i<0x1000;i++){
    data = 0;
    for(j=0;j<16;j++){
      if(getData(i&0x00FF,((i>>8)&0x000F)|(j<<4)))
        data |= 1<<j;
    }
    error = k3922Write16( station, 16, 0, data );
    if(error){
      printf( "\nError Writing to MLU Memory.\n" );
      k3922ShowError(error);
      break;
    }
  }
  printf("Completed\n ");
  
#if 0
  printf("\n");
  printf("Verifying\n ");
  error = k3922Write16(station,16,1,0);	/* set address register */
  if( error&&error!=k3922NQErr ){
    printf( "Error in k3922Write16().\n" );
    k3922ShowError( error );
    exitK2917();
    exit(1);
  }
  count=0;
  for(i=0;i<0x1000;i++){
    data = 0;
    for(j=0;j<16;j++){
      if(getData(i&0x00FF,((i>>8)&0x000F)|(j<<4)))
        data |= 1<<j;
    }
    error = k3922Read16( station, 0, 0, &data2 );
    if(error){
      printf( "\nError Reaing from MLU Memory.\n" );
      k3922ShowError(error);
      break;
    }
    if(data != data2){
      printf("Verify Error %x(write)!=%x(read)\n", data, data2);
      count++;
    }
  }
  printf("%d Verify Errors.\n", count);
#endif

  error = k3922Write16(station,16,2,Mode|Dim); /* Reset Inhibit */
  if( error&&error!=k3922NQErr ){
    printf( "Error in k3922Write16().\n" );
    k3922ShowError( error );
    exitK2917();
    exit(1);
  }

  error = exitK2917();
  if( error ){
    printf( "Error in exitK2917().\n" );
    k3922ShowError( error );
    exit(1);
  }
}


