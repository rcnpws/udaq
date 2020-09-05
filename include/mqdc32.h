#ifndef MQDC32_REG_H
#define MQDC32_REG_H

/*
 * Definitions for MQDC32
 *
 * Version 0.10  04-SEP-2020  by T. Furuno
 */


/* Size of I/O space */

#define MQDC32_MAX_N_MODULES     32          /* Max number of MQDC32 Modules */
#define MQDC32_BASE_INI   0x34000000         /* VME base address start      */
#define MQDC32_BASE_INC   0x00010000         /* VME base address increment  */
#define MQDC32_BASE(module_number)  ((MQDC32_BASE_INI)+(MQDC32_BASE_INC)*(module_number))

#define MQDC32_SIZE   0x00010000             /* Memory Mapping Size */

/* VME addressing mode */
#define MQDC32_MODE  VME_A32UD               /* 32bit Extended Non-Prvileged Data Accesss  */

#define MQDC32_NUM_CHANNELS 32               /* number of channels */


#define MQDC32_IRQ_THRESHOLD_DEFAULT 6000    /* IRQ Threshold max=8120  */


#define MQDC32_MARKING_TYPE_EVENT_COUNTER        0
#define MQDC32_MARKING_TYPE_TIME_STAMP           1
#define MQDC32_MARKING_TYPE_EXTENDED_TIME_STAMP  3

#define MQDC32_DATA_LEN_FORMAT_8BIT              0
#define MQDC32_DATA_LEN_FORMAT_16BIT             1
#define MQDC32_DATA_LEN_FORMAT_32BIT             2
#define MQDC32_DATA_LEN_FORMAT_64BIT             3

#define MQDC32_BANK_OPERATION_BANKS_CONNECTED    0
#define MQDC32_BANK_OPERATION_BANKS_INDEPENDENT  1
//#define MADC32_BANK_OPERATION_TOGGLE_MODE        3

#define MQDC32_ADC_RESOLUTION_4K                 0   /*  always 4k resolution */
//#define MADC32_ADC_RESOLUTION_4K_HIGH_RES        2   /*  3.2 micro sec */
//#define MADC32_ADC_RESOLUTION_8K                 3   /*  6.4 micro sec */
//#define MADC32_ADC_RESOLUTION_8K_HIGH_RES        4   /* 12.5 micro sec */

//#define MADC32_DELAY0_25NS                       0   /* 25 ns   */
//#define MADC32_DELAY1_25NS                       0   /* 25 ns   */
//#define MADC32_WIDTH0_2US                       40   /*  2 us   */
//#define MADC32_WIDTH1_2US                       40   /*  2 us   */
//#define MADC32_USE_GG_GG0                        1   /* use GG0 */
//#define MADC32_USE_GG_GG1                        2   /* use GG1 */

//#define MADC32_INPUT_RANGE_4V                    0
//#define MADC32_INPUT_RANGE_10V                   1
//#define MADC32_INPUT_RANGE_8V                    2

#define MQDC32_INPUT_COUPLING_B0AC_B1AC          0  /* bank0:AC, bank1:AC */
#define MQDC32_INPUT_COUPLING_B0DC_B1AC          1  /* bank0:AC, bank1:AC */
#define MQDC32_INPUT_COUPLING_B0AC_B1DC          2  /* bank0:AC, bank1:AC */
#define MQDC32_INPUT_COUPLING_B0DC_B1DC          3  /* bank0:AC, bank1:AC */

#define MQDC32_GATE_SELECT_NIM                   0  /* gate 0 and 1 from NIM-inputs */
#define MQDC32_GATE_SELECT_ECL                   1  /* gate 0 and 1 from ECL-inputs */

#define MQDC32_NIM_GAT1_OSC_GATE1_INP            0  /* gate 1 input     */
#define MQDC32_NIM_GAT1_OSC_TIME                 1  /* oscillator input */

#define MQDC32_NIM_FC_RESET_FAST_CLEAR           0  /* fast clear input    */
#define MQDC32_NIM_FC_RESET_TIME_STAMP           1  /* reset time stamp    */
#define MQDC32_NIM_FC_RESET_EXP_TRIG             2  /* exp. trigger input  */

#define MQDC32_NIM_BUSY_BUSY                     0  /* as busy */
//#define MADC32_NIM_BUSY_GG0                      1  /* as gate0 output */
//#define MADC32_NIM_BUSY_GG1                      2  /* as gate1 output */
#define MQDC32_NIM_BUSY_CBUS                     3  /* as Cbus output  */
#define MQDC32_NIM_BUSY_BUFFER_FULL              4  /* buffer full     */
#define MQDC32_NIM_BUSY_DATA_THRESHOLD           8  /* data in buffer above thereshold */
#define MQDC32_NIM_BUSY_EVT_THRESHOLD            9  /* events in buffer above thereshold */

#define MQDC32_TS_SOURCES_VME                    0  /* source:external, disable external reset */  
#define MQDC32_TS_SOURCES_EXT                    1  /* source:external, disable external reset */  
#define MQDC32_TS_SOURCES_EXTERNAL_RESET         2  /* bit for enabling the external reset */  

#define MQDC32_MULTI_EVENT_NO                    0  /* disable multi event mode */  
#define MQDC32_MULTI_EVENT_YES_UNLIMITED         1  /* allow multi event mode, unlimited transfer */
#define MQDC32_MULTI_EVENT_YES_LIMITED           3  /* allow multi event mode, limited data */

/*
 * MQDC32 Memory Register
 */

typedef struct MQDC32 {
  /* Data FIFO */
  unsigned int fifo_read;        /* read FIFO */
  unsigned char  res00[0x3FFC];

  /* Threshold memory from 0x4000 */
  unsigned short threshold[32];  /* threshold */
  unsigned char  res01[0x1FC0];

  /* Registers from 0x6000 */
  unsigned short address_source;
  unsigned short address_reg;
  unsigned short module_id;
  unsigned short fast_vme;
  unsigned short soft_reset;
  unsigned char  res02[0x0004];
  unsigned short firmware_revision;

  /* IRQ(ROACK) from 0x6010 */
  unsigned short irq_level;
  unsigned short irq_vector;
  unsigned short irq_test;
  unsigned short irq_reset;
  unsigned short irq_threshold;
  unsigned short max_transfer_data; // 0x601A
  unsigned short irq_source;
  unsigned short irq_event_threshold;

  /* MCST CBLT from 0x6020 */
  unsigned short cblt_mcst_control;
  unsigned short cblt_address;
  unsigned short mcst_address;
  unsigned char  res03[0x000A];

  /* FIFO handling from 0x6030 */
  unsigned short buffer_data_length;
  unsigned short data_len_format;
  unsigned short readout_reset;
  unsigned short multi_event;
  unsigned short marking_type;
  unsigned short start_acq;
  unsigned short fifo_reset;
  unsigned short data_ready;

  /* operation mode  from 0x6040 */
  unsigned short bank_operation;
  unsigned short adc_resolution;
  unsigned short offset_bank_0;
  unsigned short offset_bank_1;
  unsigned short slc_off;
  unsigned short skip_oorange;
  unsigned short ignore_threshold;
  unsigned char  res04[2];

  /* gate limit from 0x6050 */
  unsigned short limit_bank_0;
  unsigned short limit_bank_1;

  /* experiment trigger from 0x6054 */
  unsigned short exp_trig_delay0;
  unsigned short exp_trig_delay1;
  unsigned char  res05[0x0008];

  /* Inputs, outputs from 0x6060 */
  unsigned short input_coupling;
  unsigned short ecl_term;
  unsigned short ecl_gate1_osc;
  unsigned short ecl_fc_res;
  unsigned short gate_select;
  unsigned short nim_gate1_osc;
  unsigned short nim_fc_reset;
  unsigned short nim_busy;

  /* Test pulser from 0x6070 */
  unsigned short pulser_status;
  unsigned short pulser_dac;
  unsigned char  res06[0x000C];

  /* Mesytec control bus from 0x6080 */
  unsigned short rc_busno;
  unsigned short rc_modnum;
  unsigned short rc_opcode;
  unsigned short rc_adr;
  unsigned short rc_dat;
  unsigned short send_return_status;
  unsigned char  res07[0x0004];

  /* CTRA from 0x6090 */
  unsigned short reset_ctr_ab;
  unsigned short evctr_lo;
  unsigned short evctr_hi;
  unsigned short ts_sources;
  unsigned short ts_divisor;
  unsigned char  res08[0x0002];
  unsigned short ts_counter_lo;
  unsigned short ts_counter_hi;

  /* CTRB from 0x60A0 */
  unsigned short adc_busy_time_lo;
  unsigned short adc_busy_time_hi;
  unsigned short gate1_time_lo;
  unsigned short gate1_time_hi;
  unsigned short time_0;
  unsigned short time_1;
  unsigned short time_2;
  unsigned short stop_ctr;
} MQDC32_t, *MQDC32_p;



#if 1  /* Little Endian Definition for Intel CPU */

/* MQDC32 Data */
#define MQDC32_HEADER_SIGNATURE_DATA         0
#define MQDC32_HEADER_SIGNATURE_HEADER       1
#define MQDC32_HEADER_SIGNATURE_END_OF_EVENT 3
#define MQDC32_SUBHEADER_EXTENDED_TIME_STAMP 0x0024
#define MQDC32_SUBHEADER_EVENT               0x0020
#define MQDC32_SUBHEADER_FILL                0x0000

#define MQDC32_SUBHEADER_NO_HEADER           0x0001
#define MQDC32_SUBHEADER_NO_END_OF_EVENT     0x0002


typedef struct MQDC32_HEADER_SIGNATURE {  /* Header Signature bits*/
  unsigned res:             21;    /* reserved */
  unsigned subheader:        9;    /* subheader */
  unsigned header:           2;    /* Header Signature */
} MQDC32_HEADER_SIGNATURE_t, *MQDC32_HEADER_SIGNATURE_p;

typedef struct MQDC32_DATA_HEADER {  /* Header Word*/
  unsigned n_data_words:   12;    /* number of data words */
  unsigned adc_resolution:  3;    /* ADC resolution */
  unsigned output_format:   1;    /* Output Format */
  unsigned module_id:       8;    /* Module ID */
  unsigned subheader:       6;    /* Subheader=b00000 */
  unsigned header_signature: 2;   /* Header Signature = b01 */
} MQDC32_DATA_HEADER_t, *MQDC32_DATA_HEADER_p;

typedef struct MQDC32_DATA_EVENT {  /* Data Event Word*/
  unsigned adc_amplitude:   12;   /* ADC amplitude */
  unsigned res0:             3;   /* Reserved */
  unsigned out_of_range:     1;   /* Out of range */
  unsigned channel_number:   5;   /* Channel Number */
  unsigned subheader:        9;   /* Subheader=b000100000 */
  unsigned header_signature: 2;   /* Header Signature = b00 */
} MQDC32_DATA_EVENT_t, *MQDC32_DATA_EVENT_p;

typedef struct MQDC32_EXTENDED_TIME_STAMP {  /* Extended Time Stamp */
  unsigned time_stamp_msb:  16;   /* Most Significant 16bit Time Stamp*/
  unsigned res0:             5;   /* Reserved */
  unsigned subheader:        9;   /* Subheader=b000100100 */
  unsigned header_signature: 2;   /* Header Signature = b00 */
} MQDC32_EXTENDED_TIME_STAMP_t, *MQDC32_EXTENDED_TIME_STAMP_p;

typedef struct MQDC32_FILL {  /* FILL DUMMY (PADDING) */
  unsigned res0:            21;   /* Reserved =0 */
  unsigned subheader:        9;   /* Subheader=b000000000 */
  unsigned header_signature: 2;   /* Header Signature = b00 */
} MQDC32_FILL_t, *MQDC32_FILL_p;

typedef struct MQDC32_END_OF_EVENT {  /* End of Event */
  unsigned event_counter:   30;   /* Event Counter or Time Stamp */
  unsigned header_signature: 2;   /* Header Signature = b11 */
} MQDC32_END_OF_EVENT_t, *MQDC32_END_OF_EVENT_p;

#endif

extern char *max_number_hits_s[];

/*
 * prototype definitions of library functions
 */
int mqdc32_open(void);
MQDC32_p mqdc32_map(int module_number);
int mqdc32_unmap(int module_number);
int mqdc32_close(void);


#endif  /* for ifndef MQDC32_REG_H */
