#ifndef V792_REG_H
#define V792_REG_H

/*
 *Definitions fo v792
 *
 *Version 0.10 30-JUL-2013 by oiwa
 */

//Size of I/O space

#define V792_MAX_N_MODULES 32  //not sure
#define V792_N_CH_PER_MODULE 32
#define V792_BASE_INI 0x22220000
#define V792_BASE_INC 0x00010000
#define V792_BASE(module_number) ((V792_BASE_INI)+(V792_BASE_INC)*(module_number))

#define V792_SIZE  sizeof(V792_t)

/* VME addressing mode */
#if 1
#define V792_MODE  VME_A32UD       /* 32bit Extended Non-Prvileged Data Accesss  */
#else
#define V792_MODE  VME_A32SD       /* 32bit Extended Prvileged Data Accesss  */
#endif

/*
 * V792 Memory Register
 */

typedef struct V792{
	unsigned long output_buffer[0x1000/4]; // need check!!!!!!!!!!!!!
	unsigned short firmware_revision;
	unsigned short geo_address;
	unsigned short mcst_cblt_address;
	unsigned short bit_set_1;
	unsigned short bit_clear_1;
	unsigned short interrupt_level;
	unsigned short interrupt_vector;
	unsigned short status_register_1;
	unsigned short control_register_1;
	unsigned short ader_high;
	unsigned short ader_low;
	unsigned short single_shot_reset;
	unsigned char  res1[2];
	unsigned short mcst_cblt_ctrl;
	unsigned char  res2[4];
	unsigned short event_trigger_register;
	unsigned short status_register_2;
	unsigned short event_counter_l;
	unsigned short event_counter_h;
	unsigned short increment_event;
	unsigned short increment_offset;
	unsigned short load_test_register;
	unsigned short fclr_window;
	unsigned char  res3[2];
	unsigned short bit_set_2;
	unsigned short bit_clear_2;
	unsigned short w_memory_test_address;
	unsigned short memory_test_word_high;
	unsigned short memory_test_word_low;
	unsigned short create_select;
	unsigned short test_event_write;
	unsigned short event_counter_reset;
	unsigned char  res4[0x1E]; 
	unsigned short iped;
	unsigned char  res5[2];
	unsigned short r_test_address;
	unsigned char  res6[2];
	unsigned short sw_comm;
	unsigned char  res7[6];
	unsigned short aad;
	unsigned short bad;
	unsigned char  res8[12];
	unsigned short threshold_0;
	unsigned short threshold_1;
	unsigned short threshold_2;
	unsigned short threshold_3;
	unsigned short threshold_4;
	unsigned short threshold_5;
	unsigned short threshold_6;
	unsigned short threshold_7;
	unsigned short threshold_8;
	unsigned short threshold_9;
	unsigned short threshold_10;
	unsigned short threshold_11;
	unsigned short threshold_12;
	unsigned short threshold_13;
	unsigned short threshold_14;
	unsigned short threshold_15;
	unsigned short threshold_16;
	unsigned short threshold_17;
	unsigned short threshold_18;
	unsigned short threshold_19;
	unsigned short threshold_20;
	unsigned short threshold_21;
	unsigned short threshold_22;
	unsigned short threshold_23;
	unsigned short threshold_24;
	unsigned short threshold_25;
	unsigned short threshold_26;
	unsigned short threshold_27;
	unsigned short threshold_28;
	unsigned short threshold_29;
	unsigned short threshold_30;
	unsigned short threshold_31;
} V792_t, *V792_p;



/*
 * register map
 */

#define V792_OUT_BUFFER				0x0000				/*!< \brief Output buffer relative address */
#define V792_FW_REV				0x1000				/*!< \brief Firmware Revision register relative address */
#define V792_GEORESS				0x1002				/*!< \brief Geo Address register relative address */
#define V792_MCST_CBLTRESS			0x1004				/*!< \brief MCST/CBLT Address register relative address */
#define V792_BIT_SET_1				0x1006				/*!< \brief Bit Set 1 register relative address */
#define V792_BIT_CLEAR_1			0x1008				/*!< \brief Bit Clear 1 register relative address */
#define V792_INT_LEVEL				0x100A				/*!< \brief Interrupt level register relative address */
#define V792_INT_VECTOR				0x100C				/*!< \brief Interrupt vector register relative address */
#define V792_STATUS_1				0x100E				/*!< \brief Status 1 register relative address */
#define V792_CONTROL_1				0x1010				/*!< \brief Control 1 register relative address */
#define V792_ADER_HIGH				0x1012				/*!< \brief Address decoder HIGH register relative address */
#define V792_ADER_LOW				0x1014				/*!< \brief Address decoder LOW register relative address */
#define V792_SINGLE_SHOT_RESET			0x1016				/*!< \brief Single Shot Reset register relative address */
#define V792_MCST_CBLT_CTRL			0x101A				/*!< \brief MCST/CBLT control register relative address */
#define V792_EVENT_TRG				0x1020				/*!< \brief Event trigger register relative address */
#define V792_STATUS_2				0x1022				/*!< \brief Status 2 register relative address */
#define V792_EVENT_COUNTER_LOW			0x1024				/*!< \brief Event counter low register relative address */
#define V792_EVENT_COUNTER_HIGH			0x1026				/*!< \brief Event counter high register relative address */
#define V792_INC_EVENT				0x1028				/*!< \brief Increment Event register relative address */
#define V792_INC_OFFSET				0x102A				/*!< \brief Increment Offset register relative address */
#define V792_LOAD_TEST				0x102C				/*!< \brief Load Test register relative address */
#define V792_FCLR_WND				0x102E				/*!< \brief FCLR window register relative address */
#define V792_BIT_SET_2				0x1032				/*!< \brief Bit Set 2 register relative address */
#define V792_BIT_CLEAR_2			0x1034				/*!< \brief Bit Clear 2 register relative address */
#define V792_W_MEMORY_TESTRESS			0x1036				/*!< \brief W Memory Test Address register relative address */
#define V792_MEMORY_TEST_WORD_HIGH		0x1038				/*!< \brief Memory Test Word High register relative address */
#define V792_MEMORY_TEST_WORD_LOW		0x103A				/*!< \brief Memory Test Word Low register relative address */
#define V792_CRATE_SELECT			0x103C				/*!< \brief Crate select register relative address */
#define V792_TEST_EVENT_WRITE			0x103E				/*!< \brief Test Event register Write relative address */
#define V792_EVENT_COUNT_RESET			0x1040				/*!< \brief Event Counter Reset register relative address */
#define V792_IPED				0x1060				/*!< \brief Iped register relative address */
#define V792_R_TESTRESS				0x1064				/*!< \brief R Test Address register relative address */
#define V792_SW_COMM				0x1068				/*!< \brief SW Comm register relative address */
#define V792_AAD				0x1070				/*!< \brief AAD register relative address */
#define V792_BAD				0x1072				/*!< \brief BAD register relative address */
#define V792_THRESHOLD_0			0x1080				/*!< \brief Threshold 0 register relative address */
#define V792_THRESHOLD_1			0x1082				/*!< \brief Threshold 1 register relative address */
#define V792_THRESHOLD_2			0x1084				/*!< \brief Threshold 2 register relative address */
#define V792_THRESHOLD_3			0x1086				/*!< \brief Threshold 3 register relative address */
#define V792_THRESHOLD_4			0x1088				/*!< \brief Threshold 4 register relative address */
#define V792_THRESHOLD_5			0x108A				/*!< \brief Threshold 5 register relative address */
#define V792_THRESHOLD_6			0x108C				/*!< \brief Threshold 6 register relative address */
#define V792_THRESHOLD_7			0x108E				/*!< \brief Threshold 7 register relative address */
#define V792_THRESHOLD_8			0x1090				/*!< \brief Threshold 8 register relative address */
#define V792_THRESHOLD_9			0x1092				/*!< \brief Threshold 9 register relative address */
#define V792_THRESHOLD_10			0x1094				/*!< \brief Threshold 10 register relative address */
#define V792_THRESHOLD_11			0x1096				/*!< \brief Threshold 11 register relative address */
#define V792_THRESHOLD_12			0x1098				/*!< \brief Threshold 12 register relative address */
#define V792_THRESHOLD_13			0x109A				/*!< \brief Threshold 13 register relative address */
#define V792_THRESHOLD_14			0x109C				/*!< \brief Threshold 14 register relative address */
#define V792_THRESHOLD_15			0x109E				/*!< \brief Threshold 15 register relative address */
#define V792_THRESHOLD_16			0x10A0				/*!< \brief Threshold 16 register relative address */
#define V792_THRESHOLD_17			0x10A2				/*!< \brief Threshold 17 register relative address */
#define V792_THRESHOLD_18			0x10A4				/*!< \brief Threshold 18 register relative address */
#define V792_THRESHOLD_19			0x10A6				/*!< \brief Threshold 19 register relative address */
#define V792_THRESHOLD_20			0x10A8				/*!< \brief Threshold 20 register relative address */
#define V792_THRESHOLD_21			0x10AA				/*!< \brief Threshold 21 register relative address */
#define V792_THRESHOLD_22			0x10AC				/*!< \brief Threshold 22 register relative address */
#define V792_THRESHOLD_23			0x10AE				/*!< \brief Threshold 23 register relative address */
#define V792_THRESHOLD_24			0x10B0				/*!< \brief Threshold 24 register relative address */
#define V792_THRESHOLD_25			0x10B2				/*!< \brief Threshold 25 register relative address */
#define V792_THRESHOLD_26			0x10B4				/*!< \brief Threshold 26 register relative address */
#define V792_THRESHOLD_27			0x10B6				/*!< \brief Threshold 27 register relative address */
#define V792_THRESHOLD_28			0x10B8				/*!< \brief Threshold 28 register relative address */
#define V792_THRESHOLD_29			0x10BA				/*!< \brief Threshold 29 register relative address */
#define V792_THRESHOLD_30			0x10BC				/*!< \brief Threshold 30 register relative address */
#define V792_THRESHOLD_31			0x10BE				/*!< \brief Threshold 31 register relative address */


/*
 * QDC data format @Output Buffer
 */

#if 1 /*Little Endian Definition for intel CPU*/

typedef struct V792_H{  //Header
	unsigned reserved1:    8; //not used
	unsigned converted_ch: 6; //number of converted channels
	unsigned reserved2:    2; //not used
	unsigned crate_num:    8; //crate number
	unsigned id:           3; //identifier (=2)
	unsigned geo:          5; //geographical address
} V792_H_t, *V792_H_p;

typedef struct V792_DW{  //Data Word format
	unsigned adc:         12; //12-bit converted value
	unsigned overflow:     1; //ADC in overdlow (=1) or not (=0)
	unsigned under_thr:    1; //ADC under threshold (=1) or under (=0)
	unsigned reserved1:    2; //not used
	unsigned channel:      5; //Channel
	unsigned reserved2:    3; //not used
	unsigned id:           3; //identifier (=0)
	unsigned geo:          5; //geographical address
} V792_DW_t, *V792_DW_p;

typedef struct V792_EB{  //End of Block
	unsigned event_count: 24; //24-bit event counter value
	unsigned id:           3; //identifier (=4)
	unsigned geo:          5; //geographical address
} V792_EB_t, *V792_EB_p;

typedef struct V792_NV{  //Not valid datum
	unsigned reserved1:   24;
	unsigned id:           3;
	unsigned reserved2:    5;
} V792_NV_t, *V792_NV_p;

/*buffer type id*/
#define V792_HEADER_ID_H      0x2
#define V792_HEADER_ID_DW     0x0
#define V792_HEADER_ID_EB     0x4
#define V792_HEADER_ID_NV     0x6


typedef union V792_DATA {  /* Output buffer data (union, 32 bit)*/
	V792_H_t   header;   /* Header */
	V792_DW_t  data_word;      /* Data Words */
	V792_EB_t  endof_block; /* End of block */
	V792_NV_t  not_valid; /*not valid datum*/
} V792_DATA_t, *V792_DATA_p;


/*
 * Status Register 1
 */
typedef struct V792_SR1{  //Yes=1, No=0
	unsigned data_ready:         1;
	unsigned global_data_ready:  1; //at least 1 module has data in the chain
	unsigned busy:               1;
	unsigned global_busy:        1; //at least 1 module is busy in the chain
	unsigned amnesia:            1; //no GEO adrress picked up
	unsigned purged:             1; //during CBLT operation, the board is purged
	unsigned term_om:            1;
	unsigned term_off:           1;
	unsigned evrdy:              1; /*the number in event trigger register
					  is greater than or equal to the number of
					  evetns stored in the memory (=1)*/
	unsigned reserved:           7; //not used
} V792_SR1_t, *V792_SR1_p;

/*
 * Control Register 1
 */
typedef struct V792_CR1{
	unsigned reserved1:          2; //not used
	unsigned blkend:             1; /*in block transfer mode,
					  sends all data to the CPU
					  until the first EOB word is reached(=1)
					  or all requested data (=0)*/
	unsigned reserved2:          1; //not used
	unsigned prog_reset:         1; /*front panel "reset" acts 
					  on the module (software reset) (=1),
					  or only on data (data reset, default) (=0)*/
	unsigned berr_enable:        1; //in block transfer only
	unsigned align64:            1; //dummy words added when the number of word odd
	unsigned reserved3:          9; //not used
} V792_CR1_t, *V792_CR1_p;


/*
 * Status Resister 2
 */
typedef struct V792_SR2{
	unsigned reserved1:          1; //not used
	unsigned buff_empty:         1;
	unsigned buff_full:          1;
	unsigned reserved2:          1; //not used
	unsigned dsel0:              1; //indicate the type of piggy-back pugged the board
	unsigned dsel1:              1; //indicate the type of piggy-back pugged the board
	unsigned csel0:              1; //indicate the type of piggy-back pugged the board
	unsigned csel1:              1; //indicate the type of piggy-back pugged the board
} V792_SR2_t, *V792_SR2_p;


#endif //for Little Endian

/*
 * prototype definitions of library functions
 */
int v792_open(void);
V792_p v792_map(int module_number);
int v792_unmap(int module_number);
int v792_close(void);

#endif  /* for ifndef V792_REG_H */
