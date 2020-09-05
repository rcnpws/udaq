/*
 * libvts801.c   KAIZU VTS801 4mode scaler
 * for GE Fanuc VMIVME-7807RC
 * 2011.1.8  M. Uchida
 *
 */

#include <vme/vme.h>
#include <vme/vme_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "vts801.h"

/*************************************************/
/*                                               */
/*  vts801_map                                   */
/*   Map the VTS801 on VME                       */
/*   Arguments:                                  */
/*      fd: file descript for vmedev             */
/*    addr: VTS801 base address                  */
/*                                               */
/*************************************************/


Vts801 vts801_map(vme_bus_handle_t bus_handle)
{
  vme_master_handle_t window_handle;
  Vts801 scaler;

  if(vme_master_window_create(bus_handle, &window_handle,
                               VTS801_ADDR, VTS801_MODE, VTS801_SIZE,
			       0, NULL)) {
    perror("Error on creating a VME window");
    vme_term(bus_handle);
    exit(1);
  }

  scaler = (Vts801) vme_master_window_map(bus_handle, window_handle, 0);
  if(!scaler){
    perror("Error mapping the window");
    vme_master_window_release(bus_handle, window_handle);
    vme_term(bus_handle);
    exit(1);
  }

  return scaler;

}

void vts801_reset(Vts801 vts)
{
  vts->gate[0][0] = 0;  /* Write 0 to reset a VTS801 */
}

int vts801_read(Vts801 vts, unsigned short *sbuf)
{
  int i,j;
  unsigned long ldata, count = 0;
  unsigned short *sbuf_header, *sbuf_cur;

  sbuf_header = sbuf_cur = sbuf;
  sbuf_cur++;

  for(i=0;i<4;i++){
    for(j=0;j<16;j++){
      ldata = (vts->gate[i][j] & 0xffffff);
      *sbuf_cur++ = (unsigned short) (ldata & 0xffff);
      *sbuf_cur++ = (unsigned short) ((ldata>>16)&0xff);
    }
    *sbuf_header = (unsigned short) (0x6000 | 2*j);
    sbuf_header = sbuf_cur++;
    count += 2*j+1;
  }
  return count;

}
