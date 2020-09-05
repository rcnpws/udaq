/*
 *
 * libc256.c
 * Library routine for CAEN c256 scaler
 * via K2917-K3922 interface
 * 2003/08/18    M.Uchida
 * 2009.05.04    M.Uchida
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
#include "libcam.h"
#include "c256.h"


unsigned short *c256_open(vme_bus_handle_t bus_handle,int c){

  unsigned short *mem;


  mem = camOpen(bus_handle);
  if (!mem ) {
    perror("Open error: ");
    exit(1);
  }
  
  /* Set crate number */

  cSetClear(mem,c);

  return mem;
}
  
void c256_close(unsigned short *mem,vme_bus_handle_t bus_handle){
  camClose(mem,bus_handle);
}


void c256_reset(unsigned short *mem,int c){
  unsigned long q,x;
  nfa_t nfa;
  nfa.f = 9;
  nfa.a = 0;
  nfa.n = 3;
  nfa.c = c;
  CNAF(mem,nfa.c,nfa.n,nfa.a,nfa.f,&nfa.data,&q,&x);
  nfa.n = 4;
  CNAF(mem,nfa.c,nfa.n,nfa.a,nfa.f,&nfa.data,&q,&x);
  nfa.n = 7;
  CNAF(mem,nfa.c,nfa.n,nfa.a,nfa.f,&nfa.data,&q,&x);
  nfa.n = 8;
  CNAF(mem,nfa.c,nfa.n,nfa.a,nfa.f,&nfa.data,&q,&x);

}

void c256_read_dbuf_mode(unsigned short *mem, unsigned short sbuf[], int ch) /* ch: 1 or 2 */
{ 
  int i,j,status;
  unsigned long q,x;
  int cur_ptr=0;
  nfa_t nfa;
  int spin_up,spin_down;
  const int scaler_offset=33;
  int data1[2][16],data2[2][16];
  int dum1,dum2,dum3;

  nfa.c=0;

  if(ch == 1){
    spin_up = 3;
    spin_down = 4;
  } else if(ch==2){
    spin_up = 7;
    spin_down = 8;
  } else {
    fprintf(stderr,"read_scaler: undefined buffer.\n");
    exit(1);
  }
  

  for(j=spin_up;j<=spin_down;j++){
    nfa.n=j;
    nfa.f=0;     /* Read scaler */

    for(i=0;i<16;i++){
      nfa.a=i;
      status = CNAF(mem,nfa.c,nfa.n,nfa.a,nfa.f,&nfa.data,&q,&x);
      dum1 = (nfa.data & 0xffffff);
      status = CNAF(mem,nfa.c,nfa.n,nfa.a,nfa.f,&nfa.data,&q,&x);
      dum2 = (nfa.data & 0xffffff);

      if(dum1 == dum2){
	data1[j-spin_up][i] = dum1;
	data2[j-spin_up][i] = dum2;
      } else {
#if 1
	printf("mismatch scaler station %i ch %i %d %d\n",j,i,dum1,dum2);
#endif

	status = CNAF(mem,nfa.c,nfa.n,nfa.a,nfa.f,&nfa.data,&q,&x);
	dum3 = (nfa.data & 0xffffff);
	if(dum1 == dum3){
	  dum2 = dum3;
	} else if(dum2 == dum3) {
	  dum1 = dum3;
	} else {
	  printf("Bad scaler channel %d %d %d %d\n",i,dum1,dum2,dum3);
	  
	}

	data1[j-spin_up][i] = dum1;
	data2[j-spin_up][i] = dum2;

      }

    }

    nfa.f=9;     /* Claer scaler */
    nfa.a=0;
    status = CNAF(mem,nfa.c,nfa.n,nfa.a,nfa.f,&nfa.data,&q,&x);


  }

  for(j=0;j<2;j++){
    sbuf[cur_ptr] = 0x6020;
    for(i=0;i<16;i++){
      sbuf[cur_ptr+2*i+1]=(unsigned short) (data1[j][i]&0xffff);
      sbuf[cur_ptr+2*i+2]=(unsigned short) (W_SHIFT(data1[j][i]) & 0xff);
    }
    cur_ptr += scaler_offset;
  }
  for(j=0;j<2;j++){
    sbuf[cur_ptr] = 0x6020;
    for(i=0;i<16;i++){
      sbuf[cur_ptr+2*i+1]=(unsigned short) (data2[j][i]&0xffff);
      sbuf[cur_ptr+2*i+2]=(unsigned short) (W_SHIFT(data2[j][i]) & 0xff);
    }
    cur_ptr += scaler_offset;
  }

}





