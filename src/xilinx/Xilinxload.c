/* XlinxLoad.c ---- load Xlinx program to LeCroy2366               */
/*                                                                 */
/* Copyright (C) 1993, Atsushi Tamii                               */
/*      This program was made at RCNP.                             */
/* Modified      1996, H. Akimune                                  */
/* Modified      2001, H. Akimune ported to cc7000 on Linux        */
/*  Version 0.00        1996-04-15      by H. Akimune              */
/*  Version 1.00        1996-05-09      by T.Baba                  */
/*  Version 1.00.1      2001-04-04      by H. Akimune              */
/*  Version 1.01        2001-04-15      by M.Uchida (For Sun5.6)   */
/*  Version 2.00        2008-08-11      by M.Uchida (For Linux2.6) */
/*  Version 2.10        2008-11-23      by A. Tamii (cleaned-up)   */


#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "camlib.h"

FILE *fp;

#define DEBUG 0

void showError( short error, char *str)
{
  printf( "Error: %s\n", str);
  exit(-1);
}

int main( int argc, char **argv)
{

  short         count;
  int           data_num;
  unsigned char  buf[512];
  short error;
  short crateNo, lr2366Station;
  int cc;
  unsigned short* mem;
  int skip,back;

  short         i;
  char          dummy;
  unsigned long C,N,A,F,data,Q,X;
  
  if( argc<4 ){
    printf( "%s --- download Xlinx program\n", argv[0] );
    printf( "Usage : %s crate# station#  file_name [-s]\n",argv[0] );
    printf( "       -s: silent mode\n");
    exit(-1);
  }

  argv++;
  error= 0;
  crateNo = atoi(*argv++);
  lr2366Station = atoi(*argv++);

  if((mem = camOpen()) == NULL){
    fprintf(stderr,"Can't open camac\n");
    exit(1);
  }

  /* Open Xilinx bitmap file */
  printf("Open bit file: %s\n", *argv);
  if(NULL == (fp = fopen( *argv, "r" ))){
    showError (errno, "opening bit file.");
  }

  printf( "Download Xlinx Program to LeCroy 2366 ( Crate# %d  Station# %d)\n",
          crateNo, lr2366Station);

  /* skip dummy 16bytes and find Top of Data */
  skip=16;
  if(fseek(fp,skip,0)){
    puts("skip error");
    exit(1);
  }
  while((cc=fgetc(fp)) != EOF) {
    if((cc&0xff) == 0xff) break;
    printf("%c",cc&0xff);
  }
  printf("\n");

  back=-1;
  if(fseek(fp,back,1)){
    puts("back error!");
    exit(1);
  }


  /* Xilinx initialize */

  /* Set crate number */
  C =crateNo;
  N=lr2366Station;

  if(DEBUG) fprintf(stderr, "--- Send clear\n");
  cSetClear(mem,C);
  sleep(1);

  /* Disable function */
  if(DEBUG) fprintf(stderr, "--- Disable\n");
  F=9;
  A=0;
  data=1;
  CNAF(mem,C,N,A,F,&data,&Q,&X);
  
  /* Enter programming mode        */
  if(DEBUG) fprintf(stderr, "--- Enter programming mode\n");
  F=30;
  A=0;
  data=1;
  usleep(100);
  CNAF(mem,C,N,A,F,&data,&Q,&X);

  /* Select CAMAC programming mode */
  if(DEBUG) fprintf(stderr, "--- Select CAMAC programming mode\n");
  F=28;
  data=1;
  usleep(100);
  CNAF(mem,C,N,A,F,&data,&Q,&X);

  /* Program Xilinx chip           */
  if(DEBUG) fprintf(stderr, "--- Program Xilinx chip\n");
  F=25;
  A=0;
  data=1;
  usleep(100);
  CNAF(mem,C,N,A,F,&data,&Q,&X);

  if(X==0){
    showError( error, "writing to LeCroy 2366" );
  }

  /* Test Xilinx INIT line        */
  if(DEBUG) fprintf(stderr, "--- Test Xilinx INIT line\n");
  F=14;
  data=1;
  usleep(100);
  CNAF(mem,C,N,A,F,&data,&Q,&X);

  if(argc==4){
    printf("Xilinx initialized--- press anykey to start download. \n");
    dummy=getchar();
  }

  /* Write data */
  if(DEBUG) fprintf(stderr, "--- Start writing data\n");
  data_num=0;
  while((count=fread(buf,sizeof(*buf),sizeof(buf),fp)) > 0){
    for(i=0; i<count; i++){
      data = buf[i];
      data_num++;
      if(((data_num/1000)*1000) == data_num){
        fprintf(stderr,"data 0x%lx ptr %5d\n", data, data_num);
      }

      F=16;
      A=0;
      CNAF(mem,C,N,A,F,&data,&Q,&X);

      data=1;
      F=12;
      A=0;
      CNAF(mem,C,N,A,F,&data,&Q,&X);

    }
  }
 if(count<0) showError( errno, "reading from file" );

 printf( "\n  Loaded data size = %d bytes\n",(data_num));

 //printf( "Lecroy2366 may be 11876bytes.....\n");

  fclose(fp);
  camClose(mem);

  return 0;
}

  
  

  
