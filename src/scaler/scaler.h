/*
 *
 * scaler.h ... CAEN v260 100MHz scaler module access program header
 * Version 0.01 on 2009.03.18 by A. Tamii
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#define SCALER_ADDR_1                 0x001000    // Scaler Address #1
#define SCALER_ADDR_2                 0x002000    // Scaler Address #2
#define SCALER_ADDR_3                 0x003000    // Scaler Address #3
#define SCALER_ADDR_4                 0x004000    // Scaler Address #4
#define SCALER_ADDR_MODE              VME_A24UD   // VME Addressing Mode
                                               // 24bit Addr. non-privileged data access
#define SCALER_ADDR_SIZE              0x000100    // UIO Address Size (in byte)

typedef struct scaler{
  unsigned short reserved1;
  unsigned short reserved2;
  unsigned short interrupt_vector;
  unsigned short interrupt_level;
  unsigned short enable_vme_interrupt;
  unsigned short disable_vme_interrupt;
  unsigned short clear_vme_interrupt;
  unsigned short reserved3;
  unsigned long counter[16];
  unsigned short clear_scalers;
  unsigned short inhibit_set;
  unsigned short inhibit_reset;
  unsigned short scaler_increase;
  unsigned short interrupt_jumper_status;
  unsigned short reserved4[80];
  unsigned short fixed_code;
  unsigned short manufacturer_and_module_type;
  unsigned short version_and_series;
} scaler_t, *scaler_p;
