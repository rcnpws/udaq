/********************************************************************
 * k2917.h   Kinetic K2917 registers                                *
 *                                                                  *
 *   24-MAR-1992 Y.Takeuchi                                         *
 *                                                                  *
 * 13-JUL-1992 Y.Takeuchi    modify K2917_ENABLE,DISABLE            *
 * 30-NOV-1992 Y.Takeuchi    modify                                 *
 *  5-MAR-1993 Y.Takeuchi    modify for unified UNIX camlib         *
 * 25-OCT-1993 Y.Takeuchi    modify for Solaris2.x                  *
 ********************************************************************/


/* Size of I/O space */

#define K2917_BASE   0xFF00              /* VME base address    */
#define K2917_SIZE      256


#define K2917_MODE  VME_A16S             /* VME trasfer mode    */


/***** DMA *****/
#define CC_DMA_DONE        0x8000
#define CC_DMA_ERR         0x1000
#define CC_DMA_ACT         0x0800
#define CC_DOCR_INIT       0x8010
#define CC_DMA_READ        0x0080
#define CC_DMA_WRITE       0x0000
#define CC_DMA_START       0x0080
#define CC_DMA_ABORT       0x0010
#define CC_DMA_INT_ENABLE  0x0008
#define CC_DMA_RESET       0x0000  

/***** Interrupt *****/
#define CC_INTR_INIT       0x0000
#define CC_TEST_FLAG       0x0080
#define CC_FLAG_AUTO_CLEAR 0x0040
#define CC_INT_ENABLE      0x0010
#define CC_INT_AUTO_CLEAR  0x0008
#define CC_IRQ1            0x0001
#define CC_IRQ2            0x0002
#define CC_IRQ3            0x0003
#define CC_IRQ4            0x0004
#define CC_IRQ5            0x0005
#define CC_IRQ6            0x0006
#define CC_IRQ7            0x0007

/***** Main register *****/
#define CC_AMR_INIT        0x007D
#define CC_CMA_INIT        0x0000

#define CC_AD              0x0001
#define CC_INLINE          0x0060
#define CC_BLOCK           0x0020
#define CC_BIT24           0x0000
#define CC_BIT16           0x0002
#define CC_QSTOP           0x0000
#define CC_QIGNO           0x0008
#define CC_QREPE           0x0010
#define CC_QSCAN           0x0018
#define CC_HALT            0x0080

#define CC_ERR             0x8000
#define CC_ABT             0x4000
#define CC_TMO             0x2000
#define CC_RST             0x1000
#define CC_LAM             0x0200
#define CC_RDY             0x0100
#define CC_DONE            0x0080
#define CC_DMA             0x0040
#define CC_WRITE           0x0020
#define CC_NOX             0x0004
#define CC_NOQ             0x0002
#define CC_GO              0x0001



/* CAMAC status (for cc_iosb.status) */
#define CC_STA_OK               0
#define CC_STA_SINGLE_TIMEOUT   -1
#define CC_STA_BLOCK_INVFUNC    -2
#define CC_STA_BLOCK_TIMEOUT    -3
#define CC_STA_LIST_STOP        -10
#define CC_STA_LIST_TIMEOUT     -11
#define CC_STA_LIST_EMPTY       -12
#define CC_STA_LIST_ZERODIV     -13
#define CC_STA_LIST_OVERSTEP    -14
#define CC_STA_KLIST_SAMENAME   -21
#define CC_STA_KLIST_MEMORYFULL -22
#define CC_STA_KLIST_NONAME     -23
/* timeout value */
#define CC_TIMEOUT_LAM          60      /* List LAM: 60sec. */
#define CC_TIMEOUT_SINGLE       1000    /* single action: 1000 loop counts */
#define CC_TIMEOUT_DMA          5       /* DMA done: 5 sec */


struct K_REG {
    unsigned short cser;
    unsigned short dummy02;
    unsigned short docr;
    unsigned short sccr;
    unsigned short dummy05;
    unsigned short mtc;
    unsigned short machi;
    unsigned short maclo;

    unsigned short dummy11;           /* 0x10 - 0x1F */
    unsigned short dummy12;
    unsigned short dummy13;
    unsigned short dummy14;
    unsigned short dummy15;
    unsigned short dummy16;
    unsigned short dummy17;
    unsigned short dummy18;

    unsigned short dummy21;           /* 0x20 - 0x2F */
    unsigned short dummy22;
    unsigned short dummy23;
    unsigned short dummy24;
    unsigned short dummy25;
    unsigned short dummy26;
    unsigned short dummy27;
    unsigned short dummy28;

    unsigned short dummy31;           /* 0x30 - 0x3F */
    unsigned short dummy32;
    unsigned short dummy33;
    unsigned short dummy34;
    unsigned short dummy35;
    unsigned short dummy36;
    unsigned short dummy37;
    unsigned short dummy38;

    unsigned short lamc;
    unsigned short donc;
    unsigned short empc;
    unsigned short aboc;
    unsigned short lamv;
    unsigned short donv;
    unsigned short empv;
    unsigned short abov;

    unsigned short dummy51;           /* 0x50 - 0x5F */
    unsigned short dummy52;
    unsigned short dummy53;
    unsigned short dummy54;
    unsigned short dummy55;
    unsigned short dummy56;
    unsigned short dummy57;
    unsigned short dummy58;

    unsigned short amr;
    unsigned short cmr;
    unsigned short cma;
    unsigned short cwc;
    unsigned short srr;
    unsigned short dlr;
    unsigned short dhr;
    unsigned short csr;
};

