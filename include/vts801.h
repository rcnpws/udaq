/*
 *
 * vts801.h
 *  Definition for KAIZU V-TS 801 4-gate scaler
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#define VTS801_ADDR                 0x070000    // Scaler Address #1
#define VTS801_MODE                 VME_A32UD   // VME Addressing Mode
                                                // 32bit Addr. non-privileged data access
#define VTS801_SIZE                 0xff        // UIO Address Size (in byte)

typedef struct vts801 {
  unsigned long gate[4][16];
} *Vts801;


Vts801 vts801_map(vme_bus_handle_t bus_handle);
void vts801_reset(Vts801 vts);
int vts801_read(Vts801 vts, unsigned short *sbuf);
