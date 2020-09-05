/*
 * lr1190.h
 *   definition of registers and memory for LeCroy 1190 Dual Port Memory
 *
 *        2003/08/06    M.Uchida
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <vme/vme.h>
#include <vme/vme_api.h>


#define LRM_MODE  VME_A24SD       /* VME trasfer mode : A24D32        */
#define LRM_ADDR  0x300000        /* VME base address                 */
#define LRM_LEN   0x10000         /* VME memory size in word          */
#if 0
#define LRM_SIZE  0x40000         /* Mapping size in bytes            */
#endif
#define LRM_SIZE  0x100000         /* Mapping size in bytes           */
#define LRM_ADDR_STEP 0x100000     /* LRM Address step                */

#define LRM_FFP_ENA  1            /* Enable FPP  access               */
#define LRM_VME_ENA  0            /* Enable VME access                */

/* Structure LeCroy 1190 Dual port memory */

typedef struct {
  unsigned short mem[LRM_LEN];     /* memory                          */ 
  unsigned short mem_ptr;          /* Address counter                 */
  unsigned short daq_enable;       /* Mode register                   */
} *Lrm;


/* Utility routines */

/* Address assign rule */

#define LRM_ADDR_ASSIGN(n)  (LRM_ADDR+0x00100000*n)
#define DLRM_ADDR_ASSIGN(n) (LRM_ADDR+0x00200000*n)

/* Map LeCroy 1190 */
Lrm lrm_map(vme_bus_handle_t bus_handle);


/* Map LeCroy 1190 array*/
void lrm_nmap(vme_bus_handle_t bus_handle, long addr, Lrm lrm[], int num, int step);

/* Init LeCroy 1190 */
void lrm_init(Lrm lrm);

/* Reset LeCroy 1190 */
void lrm_reset(Lrm );

/* Enable FFP(ECL) DAQ */
#define lrm_ena(a) ((a)->daq_enable = LRM_FFP_ENA)

/* Disable FFP(ECL) DAQ */
#define lrm_dis(a) ((a)->daq_enable = LRM_VME_ENA)

/* Get data from LeCroy 1190 dual port memory */
unsigned short lrm_get(Lrm lrm, unsigned short *buf,unsigned long max_size);
unsigned short lrm_get2(Lrm lrm, unsigned short *buf,unsigned long max_size);








