#ifndef VMEDAQ_REG_H
#define VMEDAQ_REG_H

/*
 * Definitions for VMEDAQ
 *
 * Version 0.10  03-NOV-2014  by A. Tamii
 */

#define VMEDAQ_HEADER_ID (0x44454D56)   // 'VMED'
#define VMEDAQ_TYPE_V1190                0x00000001
#define VMEDAQ_TYPE_V830                 0x00000002
#define VMEDAQ_TYPE_COUNTER_OFFSET       0x00000003
#define VMEDAQ_TYPE_MADC32               0x00001000
#define VMEDAQ_TYPE_MYRIAD               0x00024000

typedef struct VMEDAQ_HEADER {
  unsigned long id;
  unsigned long type;
  unsigned long counter;
  unsigned long sizeInBytes;
} VMEDAQ_HEADER_t, *VMEDAQ_HEADER_p;

/*
 * prototype definitions of library functions
 */

#endif  /* for ifndef VMEDAQ_REG_H */
