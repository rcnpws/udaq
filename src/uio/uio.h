/*
 *
 * uio.h ... CAEN v262 Universal I/O module access program header
 * Version 0.01 on 2009.03.18 by A. Tamii
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#define UIO_ADDR_1                 0x000000    // UIO Address #1
#define UIO_ADDR_MODE              VME_A24UD   // VME Addressing Mode
                                               // 24bit Addr. non-privileged data access
#define UIO_ADDR_SIZE              0x000010    // UIO Address Size (in byte)

typedef struct uio{
  unsigned short reserved1;
  unsigned short reserved2;
  unsigned short write_ecl_level;
  unsigned short write_nim_level;
  unsigned short write_nim_pulse;
  unsigned short read_nim_level;
} uio_t, *uio_p;

