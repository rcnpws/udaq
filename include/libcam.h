/* camlib.h ---- camac function library for K2917-K3922                    */
/*								           */
/*  Version 1.00        2008-08-11      by M.Uchida (For Linux2.6)         */

#include "k2917f.h"

typedef struct {
  int n;                        /* Station number */
  int f;                        /* Function       */
  int a;                        /* Sub address    */
  int c;                        /* Crate number   */
  unsigned long  data;          /* data           */
} nfa_t;

/*unsigned short *camOpen(vme_bus_handle_t bus_handle);  */
unsigned short *camOpen(void ); 
/*struct K_REG *camOpen(vme_bus_handle_t bus_handle);*/
/*int camClose(unsigned short * mem,vme_bus_handle_t bus_handle);  */
int camClose(unsigned short * mem);  
/*int camClose(struct K_REG *k2917,vme_bus_handle_t bus_handle);  */
int CNAF(unsigned short *mem, unsigned long C,  unsigned long N,
	   unsigned long A,  unsigned long F, unsigned long* dat,
	   unsigned long* Q, unsigned long* X);
int cSetClear(unsigned short *mem, unsigned long C);

/*int CNAF(struct K_REG *k2917, unsigned long C,  unsigned long N,
	   unsigned long A,  unsigned long F, unsigned long* dat,
	   unsigned long* Q, unsigned long* X);
int cSetClear(struct K_REG *k2917, unsigned long C);*/

#define W_SHIFT(a)  (( (a) >> 16)&0xffff)
