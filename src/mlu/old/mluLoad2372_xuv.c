/* mluLoad_uv.c	download MLU Memory (Model 2373) dU vs dV Cut   */
/*								*/
/* Copyright (C) 1994, Atsushi Tamii				*/
/* 	This program was made at RCNP.				*/
/*								*/
/*  Version 0.00	1994-07-02	by Atsushi Tamii	*/
/*  Version 0.10	1996-02-25	by Atsushi Tamii	*/
/*  Version 0.20        1996-11-22      by Hidetoshi Akimune    */
/*                       Modified for X, U', V' cut             */

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

static int r;
static int du[16];
static int dv[16];

/*#initData*/
void initData()
{
  int k;
  printf("Input r:");
  scanf("%d",&r);
  r = r*r;
  k=0;
  printf("Input offset U and V\n");

/*  printf("Offset for U at X=%d: ",k);
  scanf("%d",&du[k]);
  printf("           V at X=%d: ",k);
  scanf("%d",&dv[k]);
                
  for(k=1;k<16;k++){
    du[k]=du[0];
    dv[k]=dv[0];
  }*/

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
  if(d>=r) return 1;
  return 0;
}

/*#getData---- get data to load from X1 and X2 */
short getData(u3,v3,x)
  short		u3,v3,x;
{
  short  u0,v0;
  u0=u3-du[x];
  v0=v3-dv[x];
  if(getDataFromWindow((u3-du[x]),(v3-dv[x])))
    return 1;
  else
    return 0;
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
  long		data;
  long          data2;
  long  	i, k;
  short         u, v, j;

  if( argc>1 ){
    printf( "Usage:%s\n", argv[0] );
    printf( "  Crate Number ..... CRATE\n" );
    printf( "  Station Number ... StMLU\n" );
    exit(1);
  }
  crate = atoi(getenv("CRATE"));
  station = atoi(getenv("StMLU"));
  
  printf("MLU Load Program\n");

  initData();
  
  printf( "Load MLU Memory at Crate= %d Station=%d.\n", crate, station );
  printf( "r*r = %d\n", r );


  error  = initK2917();			
  if( error ){
    printf( "Error in initK2917().\n" );
    k3922ShowError( error );
    exit(1);
  }
  k2917SetCrate(crate);			
  error = resetK2917();
  if( error ){
    printf( "Error in resetK3922().\n" );
    k3922ShowError( error );
    exitK2917();
    exit(1);
  }
  error = resetK3922();			
  if( error ){
    printf( "Error in resetK3922().\n" );
    k3922ShowError( error );
    exitK2917();
    exit(1);
  }
  
  error = k3922Write16(station,16,2,ModeInhibit|Dim);
  if( error ){
    printf( "Error in k3922Write16().\n" );
    k3922ShowError( error );
    exitK2917();
    exit(1);
  }

  printf("Loading ...\n");
  error = k3922Write16(station,16,1,0);	
  if( error ){
    printf( "Error in k3922Write16().\n" );
    k3922ShowError( error );
    exitK2917();
    exit(1);
  }

  printf("  ---> U4-U3\n");
  printf("|\n| V4-V3\nV\n  ");
  for(i=0;i<0x1000;i++){
    data = 0;
    data2= 0;
    for(j=0;j<16;j++){
      u=i&0x03f;
      v=(i>>6)&0x3f;
      if(u>=0x20) u -= 0x40;
      if(v>=0x20) v -= 0x40;
      if(getData(u,v,j))
	data |= 1<<j;
    }
    printf("%x", (data & 0x100)>>8 );
    if((i & 0x3f)==0x3f) printf("\n  ");
    error = k3922Write16( station, 16, 0, data );
    if(error){
      printf( "\nError Writing to MLU Memory.\n" );
      k3922ShowError(error);
      break;
    }
  }
  printf("Completed\n ");
  error = k3922Write16(station,16,2,Mode|Dim);
  if( error ){
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


