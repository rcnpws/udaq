#ifndef MYRIAD_REG_H
#define MYRIAD_REG_H

/*
 * Definitions for MyRIAD
 *
 * Version 0.10  05-MAR-2016  by A. Tamii
 * implemented according the MyRIAD manual 2015-04-7
 */


/* Size of I/O space */

#define MYRIAD_BASE  0x550000       /* MyRIAD base address A24  standard (new) one    */
//#define MYRIAD_BASE  0x540000       /* MyRIAD base address A24 older one     */

#define MYRIAD_SIZE  0x010000       /* Memory Size */

#define MYRIAD_BOARD_ID 0xE725      /* Fixed Board ID Word */

#define MYRIAD_HEADER_ID  0xAAAA     /* header ID (unsigned short)*/
/* VME addressing mode */
#define MYRIAD_MODE VME_A24UD       /* 24bit addressing non-Prvileged Data Accesss  */

/*
 * MyRIAD Memory Register
 */

typedef struct MyRIAD{
  unsigned short board_id;         /* Board ID = 0xE725 */
  unsigned short res01;
  unsigned short fifo_status;      /* FIFO Status Register */
  unsigned char  res02[0x001A];
  unsigned short hardware_status;  /* Hardware status Register */
  unsigned char  res03[0x03EA];
  unsigned short pulsed_control;   /* Pulsed Control Register */
  unsigned short fifo_control;     /* FIFO Control Register */
  unsigned short capture_time;     /* Capture Time Register (the address is unclear)*/
  unsigned char  res04[0x1EE];
  unsigned short code_revision;    /* Code Revision */
  unsigned short res05;
  unsigned short code_date[2];     /* Code Date */
  unsigned char  res06[0x0F8];
  unsigned short nim_status;       /* NIM Status Register */
  unsigned short gating_register;  /* Gating Register */
  unsigned short ecl_a_status;     /* ECL-A Input Status (16bits) */
  unsigned short ecl_b_status;     /* ECL-B Input Status (2bits) */
  unsigned short latched_time_stamp[3];   /* Latched Time Stamp MSB, Middle, LSB words*/
  unsigned short seredes_command_format;  /* SerDes command format */
  unsigned short coin_window_delay;       /* Coincidence window delay in 10nsec unit*/
  unsigned short coin_window_width;       /* Coincidence window width in 10nsec unit*/
  unsigned short live_time_stamp[3];      /* Live Time Stamp MSB, Middle, LSB words*/
  unsigned short ts_error_counter_ctrl;   /* Time stamp error counter control */
  unsigned short ts_error_counter_rate;   /* Rate calculation time for time stamp error counter control*/
  unsigned short ts_error_counter[2];     /* Time stamp error couner MSB, LSB words */
  unsigned short ttcl_time_offset;        /* time offset for the trigger propagation delay */
  unsigned short missed_trig_counter;     /* Missed trigger counter */
  unsigned short missed_trig_counter_ts_oe;  /* Missed trigger counter due to time stamp offset error*/
  unsigned short ts_propagation_control;  /* Time stamp propagation control register */
  unsigned char  res07[0x0C2];
  unsigned short fifo_counter;            /* Counter of number of events written in the FIFO */
  unsigned char  res08[0x002];
  unsigned short trig_counter;            /* Counter of number of triggeres received */
  unsigned short user_counter[8];         /* User counters (NIM Input) */
  unsigned char  res09[0x046];
  unsigned short serdes_config;           /* SerDes configuration register */
  unsigned char  res10[0x7B6];
  unsigned short fifo_access;             /* FIFO Access Register */
} MyRIAD_t, *MyRIAD_p;


/*
 *  TDC data format @Output Buffer
 */

/* Little Endian Definition for Intel CPU */
typedef struct MyRIAD_FIFO_status {  /* MyRIAD Status Register */
  unsigned ffir_a:1;
  unsigned paf_a: 1;
  unsigned hf_a:  1;
  unsigned pae_a: 1;
  unsigned ef_a:  1;
  unsigned flag0: 1;
  unsigned flag1: 1;
  unsigned dav:   1;
  unsigned ffir_b:1;
  unsigned paf_b: 1;
  unsigned hf_b:  1;
  unsigned pae_b: 1;
  unsigned ef_b:  1;
  unsigned flag2: 1;
  unsigned flag3: 1;
  unsigned res1:  1;
} MyRIAD_FIFO_status_t, *MyRIAD_FIFO_status_p;

typedef struct MyRIAD_hardware_status {  /* MyRIAD Hardware Status Register */
  unsigned stat0:         1;
  unsigned stat1:         1;
  unsigned sd_sm_lst_lk:  1;
  unsigned cntr_err:      1;
  unsigned dcm_status:    4;
  unsigned dcm_lock:      1;
  unsigned res0:          1;
  unsigned sd_lock:       1;
  unsigned sd_unq_sm_lk:  1;
  unsigned isync:         1;
  unsigned sync:          1;
  unsigned res1:          1;
  unsigned sd_sm_lock:    1;
} MyRIAD_hardware_status_t, *MyRIAD_hardware_status_p;

typedef union MyRIAD_pulsed_control {  /* MyRIAD Pulsed Control Register */
  struct raw {
    unsigned short sint;
    unsigned short unused;
  } raw;
  struct bit {
    unsigned res0:               2;
    unsigned sm_lost_lock_reset: 1;
    unsigned clear_trig_counter: 1;
    unsigned local_ts_reset:     1;
    unsigned fifo_reset:         1;
    unsigned res1:               2;
    unsigned phase_dec:          1;
    unsigned phase_inc:          1;
    unsigned res2:               2;
    unsigned tdc_reset:          1;
    unsigned res3:               1;
    unsigned fifo_sd_capture:    1;
    unsigned dcm_reset:          1;
    unsigned res4:              16;
  } bit;
} MyRIAD_pulsed_control_t, *MyRIAD_pulsed_control;

typedef struct MyRIAD_FIFO_control {  /* FIFO Control Register */
  unsigned fwft_si:       1;
  unsigned be_le:         1;
  unsigned res0:          1;
  unsigned res1:          1;
  unsigned res2:          1;
  unsigned res3:          1;
  unsigned res4:          1;
  unsigned res5:          1;
  unsigned res6:          1;
  unsigned res7:          1;
  unsigned res8:          1;
  unsigned res9:          1;
  unsigned flag0:         1;
  unsigned flag1:         1;
  unsigned flag2:         1;
  unsigned flag3:         1;
} MyRIAD_FIFO_control_t, *MyRIAD_FIFO_control_p;

typedef struct MyRIAD_code_revision {  /* Code Revision */
  unsigned code_revision_minor:       4;
  unsigned code_revision_major:       4;
  unsigned firmware_type:             4;
  unsigned PCB_revision:              4;
} MyRIAD_code_revision_t, *MyRIAD_code_revision_p;

typedef struct MyRIAD_gating_register {  /* Gating Register */
  unsigned ts_latch_source:           2;
  unsigned res0:                      10;
  unsigned ila_mux_b:                 1;
  unsigned force_fifo:                1;
  unsigned ila_mux_a:                 1;
  unsigned trig_in_sel:               1;
} MyRIAD_gating_register_t, *MyRIAD_gating_register_p;

#define MyRIAD_GATING_REGISTER_TS_LATCH_SOURCE_LOCAL    1;
#define MyRIAD_GATING_REGISTER_TS_LATCH_SOURCE_MASTER   2;
#define MyRIAD_GATING_REGISTER_TRIG_IN_SEL_NIM          0;
#define MyRIAD_GATING_REGISTER_TRIG_IN_SEL_ECL          1;

typedef struct MyRIAD_TS_error_counter_ctrl {  /* TS error counter countrl register */
  unsigned reset:                     1;
  unsigned mode:                      1;
  unsigned inhibit:                   1;
  unsigned res0:                     13;
} MyRIAD_TS_error_counter_ctrl_t, *MyRIAD_TS_error_counter_ctrl_p;

typedef struct MyRIAD_TS_propagation_control {  /* TS propagation control register */
  unsigned sync:                      1;
  unsigned trig_des:                  8;
  unsigned int_trig_b:                1;
  unsigned int_trig_a:                1;
  unsigned gret_async:                1;
  unsigned sync_capt:                 1;
  unsigned aux_det_cmd:               1;
  unsigned res0:                      2;
} MyRIAD_TS_propagation_control_t, *MyRIAD_TS_propagation_control_p;

typedef struct MyRIAD_serdes_config {  /* SerDes configuration register */
  unsigned den:                       1;
  unsigned ren:                       1;
  unsigned line_le:                   1;
  unsigned loc_le:                    1;
  unsigned sync:                      1;
  unsigned t_pwr:                     1;
  unsigned r_pwr:                     1;
  unsigned sl:                        1;
  unsigned res0:                      7;
  unsigned clk_sel:                   1;
} MyRIAD_serdes_config_t, *MyRIAD_serdes_config_p;



/*
 * prototype definitions of library functions
 */

int myriad_open();
MyRIAD_p myriad_map();
int myriad_unmap();
int myriad_close();

#endif  /* for ifndef MYRIAD_REG_H */
