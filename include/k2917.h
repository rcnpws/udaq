#ifndef K2917_REG_H
#define K2917_REG_H

/*
 * Register definitions for Kinetic 2917 interface to 3922 CAMAC C.C.
 *
 *    N.B. Only registers related to single-transfer are defined.  
 */


/* Size of I/O space */

#define K2917_BASE   0xFF00              /* VME base address    */
#define K2917_SIZE      256

#if 0
#define K2917_MODE  VME_A16S            /* VME trasfer mode    */
#else
#define K2917_MODE  VME_A16U            /* VME trasfer mode    */
#endif
/*
 * Command Memory Register
 */
#define K2917_CMR       (0x0062>>1)
#define K2917_CMR_QSTP  (0<< 3)
#define K2917_CMR_QIGN  (1<< 3)
#define K2917_CMR_QRPT  (2<< 3)
#define K2917_CMR_QSCN  (3<< 3)
#define K2917_CMR_WS16  (1<< 1)
#define K2917_CMR_WS24  (0<< 1)
#define K2917_CMR_AD    1
#define K2917_CMR_HALT  0x0080

/*
 * Command Memory Address Register
 */
#define K2917_CMA       (0x0064>>1)

/*
 * Data Low Register
 */
#define K2917_DLR       (0x006A>>1)

/*
 * Data High Register
 */
#define K2917_DHR       (0x006C>>1)

/*
 * Control Status Register
 */
#define K2917_CSR       (0x006E>>1)
/*        (R) Error */
#define K2917_CSR_ERR   (1<<15)
/*        (R) Abort */
#define K2917_CSR_ABT   (1<<14)
/*        (R) Time Out */
#define K2917_CSR_TMO   (1<<13)
/*        (W) Rest 2917 */
#define K2917_CSR_RST   (1<<12)
/*            Transfer Mode (0=Sngl Xfer, 1=Blk Xfer, 3=Sngl inline Write) */
#define K2917_CSR_TM(b) (b<<10)
/*        (R) LAM */
#define K2917_CSR_LAM   (1<< 9)
/*        (R) Ready */
#define K2917_CSR_RDY   (1<< 8)
/*        (R) Done */
#define K2917_CSR_DONE  (1<< 7)
/*            Direct Memory Access */
#define K2917_CSR_DMA   (1<< 6)
/*            Direction (0=CAMAC read, 1=CAMAC write) */
#define K2917_CSR_DIR   (1<< 5)
/*        (R) Interface ID */
#define K2917_CSR_ID1   (1<< 4)
#define K2917_CSR_ID2   (1<< 3)
/*        (R) No-X */
#define K2917_CSR_NOX   (1<< 2)
/*        (R) No-Q */
#define K2917_CSR_NOQ   (1<< 1)
/*        (W) Go */
#define K2917_CSR_GO    (1    )

#endif /* K2917_REG_H */
