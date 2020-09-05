/*
 *
 * rpv130.h
 *   REPIC  Interrupt & I/O register (Model RPV-130)
 *   Header file
 *                          2001/08/02   M.Uchida
 *                          2001/08/04   M.Uchida
 *   Modified for VMIVME7750
 *                          2005/05/20   M.Uchida
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>


#define W   unsigned short
#if 1
#define RPV_MODE  VME_A16S            /* VME trasfer mode    */
#else
#define RPV_MODE  VME_A16U            /* VME trasfer mode    */
#endif

#if 0
#define RPV_SIZE  0x1000              /* VME window size     */
#else
#define RPV_SIZE  0x10                /* VME window size     */
#endif

/* Definition of RPV130 address map               */

#define RPV_ADDR     0xe800           /* RPV130 base address */
#define RPV_READ_L1  RPV_ADDR         /* LATCH1 read         */
#define RPV_READ_L2  (RPV_ADDR+0x2)   /* LATCH1 read         */
#define RPV_READ_FF  (RPV_ADDR+0x4)   /* Flipflop read       */
#define RPV_READ_TH  (RPV_ADDR+0x6)   /* Through read        */
#define RPV_WRITE_P  (RPV_ADDR+0x8)   /* Pulse write         */
#define RPV_WRITE_L  (RPV_ADDR+0xa)   /* Level write         */
#define RPV_CSR1     (RPV_ADDR+0xc)   /* CSR1 Read/write     */
#define RPV_CSR2     (RPV_ADDR+0xe)   /* CSR2 Read/write     */

typedef struct rpv130 {
  W latch1;    /* LATCH1 read         */
  W latch2;    /* LATCH2 read         */
  W ff;        /* Flipflop read       */
  W thr;       /* Through read        */
  W pulse;     /* Pulse write         */
  W level;     /* Level write         */
  W csr1;      /* CSR1 Read/write     */
  W csr2;      /* CSR2 Read/write     */
} *Rpv;

/* Some usefule macros                */

/* Generate one pulse from out-reg    */
#define rpv_pulse(rpv,dat) rpv->pulse = dat

/* Generate level output from out-reg */
#define rpv_level(rpv,dat) rpv->level = dat
void outpuls( Rpv rpv, unsigned short dat);
void outlevel( Rpv rpv, unsigned short dat);
