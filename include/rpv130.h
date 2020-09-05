/*
 * rpv130.h
 *   REPIC  Interrupt & I/O register (Model RPV-130)
 *   Header file
 *                          2001/08/02   M.Uchida
 *                          2001/08/04   M.Uchida
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#if 0
#define W   unsigned short
#endif

#define RPV_MODE  VME_A16S            /* VME trasfer mode    */
#define RPV_SIZE  0x10                /* VME window size     */


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
  unsigned short latch1;    /* LATCH1 read         */
  unsigned short latch2;    /* LATCH2 read         */
  unsigned short ff;	       /* Flipflop read       */
  unsigned short thr;       /* Through read        */
  unsigned short pulse;     /* Pulse write         */
  unsigned short level;     /* Level write         */
  unsigned short csr1;      /* CSR1 Read/write     */
  unsigned short csr2;      /* CSR2 Read/write     */
} *Rpv;

void outpuls(Rpv rpv, unsigned short dat);
void sighdl(int arg);
void outlevel(Rpv rpv, unsigned short dat);
void test_funct(int vmedev);
  
int intr_count;

/* Definition of Input and Output register */

#define RPV_IREG_BUF1 (1<<0)    /* Overflow from buf1 */
#define RPV_IREG_BUF2 (1<<1)    /* Overflow from buf2 */

#if 0
#define RPV_OREG_INIT (1<<0)    /* Init signal        */
#define RPV_OREG_BUF1 (1<<1)    /* Avairable buf1     */
#define RPV_OREG_BUF2 (1<<2)    /* Avairable buf2     */
#define RPV_OREG_VETO (1<<6)    /* VETO signal        */
#else /* modified by M.Uchida 2003/08/24 */
#define RPV_OREG_BUF1 (1<<0)    /* Avairable buf1     */
#define RPV_OREG_BUF2 (1<<1)    /* Avairable buf2     */
#define RPV_OREG_INIT (1<<2)    /* Init signal        */
#define RPV_OREG_VETO (1<<3)    /* VETO signal        */
#endif

Rpv rpv_map(vme_bus_handle_t bus_handle);

void rpv_dump(Rpv rpv);
void rpv_init(Rpv rpv);
void rpv_ena(Rpv rpv);
void rpv_dis(Rpv rpv);

#if 0

void rpv_ena_buf(Rpv rpv,int i_buf,unsigned short *dat);
void rpv_dis_buf(Rpv rpv,int i_buf,unsigned short *dat);

#else

void rpv_ena_buf(Rpv rpv,int i_buf);
void rpv_dis_buf(Rpv rpv,int i_buf);

#endif

/* Some usefule macros                */

/* Generate one pulse from out-reg    */
#define rpv_pulse(rpv,dat) rpv->pulse = dat

/* Generate level output from out-reg */
#define rpv_level(rpv,dat) rpv->level = dat

#if 0
/* External VETO on                   */
#define rpv_dis(rpv,dat) dat |= RPV_OREG_VETO;rpv->level=dat;

/* External VETO off                  */
#define rpv_ena(rpv,dat) dat &= ~RPV_OREG_VETO;rpv->level=dat;
#endif

/* RPV130 clear                       */
/*
  #define rpv_clr(rpv) rpv->csr1 = 0x5b; rpv->csr2 = 0x5b;
*/
#define rpv_clr(rpv) rpv->csr1 = 0x3; rpv->csr2 = 0x3;


