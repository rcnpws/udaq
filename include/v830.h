#ifndef V830_REG_H
#define V830_REG_H
/*
 *
 * v830.h ... CAEN v260 100MHz scaler module access program header
 * Version 0.01 on 2009.03.18 by A. Tamii
 * Version 1.00 on 2014.11.02 by A. Tamii
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#define V830_MAX_N_MODULES    64          /* Max number of V830 Modules */
#if 1
#define V830_MODE        VME_A32UD   // VME Addressing Mode = 32bit Addr. non-privileged data access
#define V830_BASE_INI    0xC0C00000  // V830 Scaler Address Base
#else
#define V830_MODE        VME_A24UD   // VME Addressing Mode = 24bit Addr. non-privileged data access
#define V830_BASE_INI    0x00C00000  // V830 Scaler Address Base
#endif

#define V830_BASE_INC    0x00010000  // V830 Scaler Address Increment
#define V830_BASE(module_number)      ((V830_BASE_INI)+(V830_BASE_INC)*(module_number))
#define V830_SIZE        0x001200    // UIO Address Size (in byte)

typedef struct V830{
  unsigned long meb[0x400];
  unsigned long counter[32];
  unsigned long testreg;
  unsigned char res01[12];
  unsigned short testlcntl;
  unsigned char res02[2];
  unsigned short testlcnth;
  unsigned char res03[10];
  unsigned short testhcntl;
  unsigned char res04[2];
  unsigned short testhcnth;
  unsigned char res05[10];
  unsigned char res06[0x50];
  unsigned long channelEnableRegister;
  unsigned long dwellTime;
  unsigned short controlRegister;
  unsigned short bitSetRegister;
  unsigned short bitClearRegister;
  unsigned short statusRegister;
  unsigned short geoRegister;
  unsigned short interruptLevel;
  unsigned short interruptVector;
  unsigned short ADER_32;
  unsigned short ADER_24;
  unsigned short enableADER;
  unsigned short MCSTBaseAddress;
  unsigned short MCSTControl;
  unsigned short moduleReset;
  unsigned short softwareClear;
  unsigned short softwareTrigger;
  unsigned short res07;
  unsigned short triggerCounter;
  unsigned short res08;
  unsigned short almostFullLevel;
  unsigned short res09;
  unsigned short BLTEventNumber;
  unsigned short firmware;
  unsigned short MEBEventNumber;
} V830_t, *V830_p;


#define V830_CR_ACQMODE_TRIG_DISABLED  0x0000
#define V830_CR_ACQMODE_TRIG_RANDOM    0x0001
#define V830_CR_ACQMODE_TRIG_PERIODIC  0x0002
#define V830_CR_DATA_FORMAT_32         0x0000
#define V830_CR_DATA_FORMAT_24         0x0004
#define V830_CR_TEST_MODE              0x0008
#define V830_CR_BERR_ENABLE            0x0010
#define V830_CR_HEADER_ENABLE          0x0020
#define V830_CR_CLEAR_MEB              0x0040
#define V830_CR_AUTO_RESET             0x0080

#define V830_SR_DREADY                 0x0001
#define V830_SR_ALMOST_FULL            0x0002
#define V830_SR_FULL                   0x0004
#define V830_SR_GLOBAL_DREADY          0x0008
#define V830_SR_GLOBAL_BUSY            0x0010
#define V830_SR_TERM_ON                0x0020
#define V830_SR_TERM_OFF               0x0040
#define V830_SR_BERR_FLAG              0x0080

#if 1  /* Little Endian Definition for Intel CPU */
typedef struct V830_MEB_HEADER {
  unsigned trigger_number: 16; /* trigger number */
  unsigned ts:           2;    /* trigger selection */
  unsigned n_words:      6;    /* number of words */
  unsigned reserved1:    2;    /* reserved bit */
  unsigned header:       1;    /* header bit */
  unsigned geo:          5;    /* Geographical Address */
} V830_MEB_HEADER_t, *V830_MEB_HEADER_p;
#endif


/*
 * prototype definitions of library functions
 */
int v830_open(void);
V830_p v830_map(int module_number);
int v830_unmap(int module_number);
int v830_close(void);
int v830_clear_counters(V830_p v830);

#endif  /* for ifndef V830_REG_H */
