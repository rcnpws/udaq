/* mluDump.c	dump MLU Memory			   		*/
/*								*/
/* Copyright (C) 1994, Atsushi Tamii				*/
/* 	This program was made at RCNP.				*/
/*								*/
/*  Version 0.00	1994-07-02	by Atsushi Tamii	*/

# include <stdio.h>
# include <errno.h>
# include "K3922drv.h"
# include "K3922.def"

# define Dim16	0x00	/* Dimension = 16bit */
# define Dim15	0x01	/* Dimension = 15bit */
# define Dim14	0x02	/* Dimension = 14bit */
# define Dim13	0x03	/* Dimension = 13bit */
# define Dim12	0x04	/* Dimension = 12bit */

# define ModeStrobe	0x00	/* Strobe Mode */
# define ModeTrans	0x08	/* Transparent Mode */
# define ModeInhibit	0x10	/* Inhibit Mode */

# define Dim	Dim16
# define Mode	ModeStrobe

/*#main--- main function */
main( argc, argv )
  short           argc;
  unsigned char   *argv[];
{
  short		error;
  short		crate;
  short		station;
  unsigned char	str[256];
  short		x1,x2;
  short		data;
  long  	i;

  if( argc>1 ){
    printf( "Usage:%s\n", argv[0] );
    printf( "  Crate Number ..... CRATE\n" );
    printf( "  Station Number ... StMLU\n" );
    exit(1);
  }
  crate = atoi(getenv("CRATE"));
  station = atoi(getenv("StMLU"));

  printf( "Dump MLU Memory at Crate= %d Station=%d.\n", crate, station );

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

  error = k3922Write16(station,16,1,0);	/* set address register */
  if( error&&error!=k3922NQErr ){
    printf( "Error in k3922Write16().\n" );
    k3922ShowError( error );
    exitK2917();
    exit(1);
  }

  for(x1=0;x1<0x100;x1++){
    for(x2=0;x2<0x100;x2+=8){
      printf("%2x:%2x ",x1,x2);
      for(i=0;i<0x08;i++){
        error = k3922Read16( station, 0, 0, &data );
        if(error){
          printf( "\nError Reading from MLU.\n" );
          k3922ShowError(error);
          x1=x2=0x100;
          break;
        }
        printf("%4x ",data&0x0000FFFF);
      }
      printf("\n");
    }
    printf("\n");
  }
  
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


