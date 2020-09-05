/* mluLoadFile.c download MLU Memory (Model 2373) from File	*/
/*								*/
/* Copyright (C) 1994, Atsushi Tamii				*/
/* 	This program was made at RCNP.				*/
/*								*/
/*  Version 0.00	1994-07-02	by Atsushi Tamii	*/
/*  Version 1.00	1996-01-23	by Atsushi Tamii	*/

# include <stdio.h>
# include <modes.h>
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

int     fd=-1;
short   buf[256];
short   buf_x = -1;

/*#getData---- get data to load from X1 and X2 */
short getData(x1,x2)
  short		x1,x2;
{
  int           count;
  if(buf_x!=x2){
    count = read( fd, buf, 256 );
    if( count==-1 ){
      printf("Coundn't read file.\n");
      exit(1);
    }
    buf_x = x2;
  }
  return buf[x1];
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
  short		data;
  long  	i;

  if( argc<2 ){
    printf( "Usage:%s file_name\n", argv[0] );
    printf( "  file_name ........ data to load\n" );
    printf( "  Crate Number ..... CRATE\n" );
    printf( "  Station Number ... StMLU\n" );
    exit(1);
  }

  fd = open( argv[1], S_IREAD );
  if(fd==-1){
    printf( "Error on opening file '%s'.\n", argv[1] );
    exit(1);
  }

  crate = atoi(getenv("CRATE"));
  station = atoi(getenv("StMLU"));

  printf( "Load MLU Memory at Crate= %d Station=%d.\n", crate, station );
  printf( "Data File = '%s'.\n", argv[1] );

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

  for(i=0;i<0x10000;i++){
    data = getData(i&0x00FF,(i>>8)&0x00FF);
    error = k3922Write16( station, 16, 0, data );
    if(error){
      printf( "\nError Writing To MLU Memory.\n" );
      k3922ShowError(error);
      break;
    }
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

  error = close(fd);
  fd = -1;
  if( error==-1 ){
    printf( "Error in close()\n" );
    exit(1);
  }
}


