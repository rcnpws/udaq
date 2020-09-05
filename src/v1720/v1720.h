#ifndef V1720_REG_H
#define V1720_REG_H

/*
 *Definitions fo v1720
 *
 *Version 0.10 1-AUG-2013 by oiwa
 */

//Size of I/O space

#define V1720_MAX_N_MODULES 32  //not sure
#define V1720_N_CH_PER_MODULE 8
#define V1720_BASE_INI 0xDD000000
#define V1720_BASE_INC 0x00010000
#define V1720_BASE(module_number) ((V1720_BASE_INI)+(V1720_BASE_INC)*(module_number))

#define V1720_SIZE  sizeof(V1720_t)

/* VME addressing mode */
#if 1
#define V1720_MODE  VME_A32UD       /* 32bit Extended Non-Prvileged Data Accesss  */
#else
#define V1720_MODE  VME_A32SD       /* 32bit Extended Prvileged Data Accesss  */
#endif

/*
 * V1720 Memory Register
 */

typedef struct V1720{
unsigned long EVENT_READOUT_BUFFER[0x1000/4];
unsigned char Channel_0_reserved_1[0x024];
unsigned long Channel_0_ZS_THRES;
unsigned long Channel_0_ZS_NSAMP;
unsigned char Channel_0_reserved_2[4];
unsigned char Channel_0_reserved_3[0x50];
unsigned long Channel_0_THRESHOLD;
unsigned long Channel_0_TIME_OVER_UNDER_THRESHOLD;
unsigned long Channel_0_STATUS;
unsigned char Channel_0_reserved_4[4];
unsigned long Channel_0_AMCFPGA_FIRMWARE_REVISION;
unsigned char Channel_0_reserved_5[4];
unsigned long Channel_0_BUFFER_OCCUPANCY;
unsigned long Channel_0_DAC;
unsigned long Channel_0_ADC_CONFIGURATION;
unsigned char Channel_0_reserved_6[12];
unsigned char Channel_0_reserved_7[0x50];
unsigned char Channel_1_reserved_1[0x024];
unsigned long Channel_1_ZS_THRES;
unsigned long Channel_1_ZS_NSAMP;
unsigned char Channel_1_reserved_2[4];
unsigned char Channel_1_reserved_3[0x50];
unsigned long Channel_1_THRESHOLD;
unsigned long Channel_1_TIME_OVER_UNDER_THRESHOLD;
unsigned long Channel_1_STATUS;
unsigned char Channel_1_reserved_4[4];
unsigned long Channel_1_AMCFPGA_FIRMWARE_REVISION;
unsigned char Channel_1_reserved_5[4];
unsigned long Channel_1_BUFFER_OCCUPANCY;
unsigned long Channel_1_DAC;
unsigned long Channel_1_ADC_CONFIGURATION;
unsigned char Channel_1_reserved_6[12];
unsigned char Channel_1_reserved_7[0x50];
unsigned char Channel_2_reserved_1[0x024];
unsigned long Channel_2_ZS_THRES;
unsigned long Channel_2_ZS_NSAMP;
unsigned char Channel_2_reserved_2[4];
unsigned char Channel_2_reserved_3[0x50];
unsigned long Channel_2_THRESHOLD;
unsigned long Channel_2_TIME_OVER_UNDER_THRESHOLD;
unsigned long Channel_2_STATUS;
unsigned char Channel_2_reserved_4[4];
unsigned long Channel_2_AMCFPGA_FIRMWARE_REVISION;
unsigned char Channel_2_reserved_5[4];
unsigned long Channel_2_BUFFER_OCCUPANCY;
unsigned long Channel_2_DAC;
unsigned long Channel_2_ADC_CONFIGURATION;
unsigned char Channel_2_reserved_6[12];
unsigned char Channel_2_reserved_7[0x50];
unsigned char Channel_3_reserved_1[0x024];
unsigned long Channel_3_ZS_THRES;
unsigned long Channel_3_ZS_NSAMP;
unsigned char Channel_3_reserved_2[4];
unsigned char Channel_3_reserved_3[0x50];
unsigned long Channel_3_THRESHOLD;
unsigned long Channel_3_TIME_OVER_UNDER_THRESHOLD;
unsigned long Channel_3_STATUS;
unsigned char Channel_3_reserved_4[4];
unsigned long Channel_3_AMCFPGA_FIRMWARE_REVISION;
unsigned char Channel_3_reserved_5[4];
unsigned long Channel_3_BUFFER_OCCUPANCY;
unsigned long Channel_3_DAC;
unsigned long Channel_3_ADC_CONFIGURATION;
unsigned char Channel_3_reserved_6[12];
unsigned char Channel_3_reserved_7[0x50];
unsigned char Channel_4_reserved_1[0x024];
unsigned long Channel_4_ZS_THRES;
unsigned long Channel_4_ZS_NSAMP;
unsigned char Channel_4_reserved_2[4];
unsigned char Channel_4_reserved_3[0x50];
unsigned long Channel_4_THRESHOLD;
unsigned long Channel_4_TIME_OVER_UNDER_THRESHOLD;
unsigned long Channel_4_STATUS;
unsigned char Channel_4_reserved_4[4];
unsigned long Channel_4_AMCFPGA_FIRMWARE_REVISION;
unsigned char Channel_4_reserved_5[4];
unsigned long Channel_4_BUFFER_OCCUPANCY;
unsigned long Channel_4_DAC;
unsigned long Channel_4_ADC_CONFIGURATION;
unsigned char Channel_4_reserved_6[12];
unsigned char Channel_4_reserved_7[0x50];
unsigned char Channel_5_reserved_1[0x024];
unsigned long Channel_5_ZS_THRES;
unsigned long Channel_5_ZS_NSAMP;
unsigned char Channel_5_reserved_2[4];
unsigned char Channel_5_reserved_3[0x50];
unsigned long Channel_5_THRESHOLD;
unsigned long Channel_5_TIME_OVER_UNDER_THRESHOLD;
unsigned long Channel_5_STATUS;
unsigned char Channel_5_reserved_4[4];
unsigned long Channel_5_AMCFPGA_FIRMWARE_REVISION;
unsigned char Channel_5_reserved_5[4];
unsigned long Channel_5_BUFFER_OCCUPANCY;
unsigned long Channel_5_DAC;
unsigned long Channel_5_ADC_CONFIGURATION;
unsigned char Channel_5_reserved_6[12];
unsigned char Channel_5_reserved_7[0x50];
unsigned char Channel_6_reserved_1[0x024];
unsigned long Channel_6_ZS_THRES;
unsigned long Channel_6_ZS_NSAMP;
unsigned char Channel_6_reserved_2[4];
unsigned char Channel_6_reserved_3[0x50];
unsigned long Channel_6_THRESHOLD;
unsigned long Channel_6_TIME_OVER_UNDER_THRESHOLD;
unsigned long Channel_6_STATUS;
unsigned char Channel_6_reserved_4[4];
unsigned long Channel_6_AMCFPGA_FIRMWARE_REVISION;
unsigned char Channel_6_reserved_5[4];
unsigned long Channel_6_BUFFER_OCCUPANCY;
unsigned long Channel_6_DAC;
unsigned long Channel_6_ADC_CONFIGURATION;
unsigned char Channel_6_reserved_6[12];
unsigned char Channel_6_reserved_7[0x50];
unsigned char Channel_7_reserved_1[0x024];
unsigned long Channel_7_ZS_THRES;
unsigned long Channel_7_ZS_NSAMP;
unsigned char Channel_7_reserved_2[4];
unsigned char Channel_7_reserved_3[0x50];
unsigned long Channel_7_THRESHOLD;
unsigned long Channel_7_TIME_OVER_UNDER_THRESHOLD;
unsigned long Channel_7_STATUS;
unsigned char Channel_7_reserved_4[4];
unsigned long Channel_7_AMCFPGA_FIRMWARE_REVISION;
unsigned char Channel_7_reserved_5[4];
unsigned long Channel_7_BUFFER_OCCUPANCY;
unsigned long Channel_7_DAC;
unsigned long Channel_7_ADC_CONFIGURATION;
unsigned char Channel_7_reserved_6[12];
unsigned char Channel_7_reserved_7[0x50];
unsigned char res0[0x6800];
unsigned long CHANNEL_CONFIGURATION;
unsigned long CHANNEL_CONFIGURATION_BIT_SET;
unsigned long CHANNEL_CONFIGURATION_BIT_CLEAR;
unsigned long BUFFER_ORGANIZATION;
unsigned long BUFFER_FREE;
unsigned char res1[12];
unsigned long CUSTOM_SIZE;
unsigned char res2[12];
unsigned char res3[0xD0];
unsigned long ACQUISITION_CONTROL;
unsigned long ACQUISITION_STATUS;
unsigned long SW_TRIGGER;
unsigned long TRIGGER_SOURCE_ENABLE_MASK;
unsigned long FRONT_PANEL_TRIGGER_OUT_ENABLE_MASK;
unsigned long POST_TRIGGER_SETTING;
unsigned long FRONT_PANEL_IO_DATA;
unsigned long FRONT_PANEL_IO_CONTROL;
unsigned long CHANNEL_ENABLE_MASK;
unsigned long ROC_FPGA_FIRMWARE_REVISION;
unsigned char res4[4];
unsigned long EVENT_STORED;
unsigned char res5[8];
unsigned long SET_MONITOR_DAC;
unsigned char res6[4];
unsigned long BOARD_INFO;
unsigned long MONITOR_MODE;
unsigned char res7[4];
unsigned long EVENT_SIZE;
unsigned char res8[0x6DAE];
unsigned long VME_CONTROL;
unsigned long VME_STATUS;
unsigned long BOARD_ID;
unsigned long MULTICAST_BASE_ADDRESS_CONTROL;
unsigned long RELOCATION_ADDRESS;
unsigned long INTERRUPT_STATUS_ID;
unsigned long INTERRUPT_EVENT_NUMBER;
unsigned long BLT_EVENT_NUMBER;
unsigned long SCRATCH;
unsigned long SW_RESET;
unsigned long SW_CLEAR;
unsigned long FLASH_ENABLE;
unsigned long FLASH_DATA;
//unsigned long CONFIGURATION_RELOAD;
//unsigned long CONFIGURATION_ROM;
} V1720_t, *V1720_p;



/*
 * register map
 */
#define  V1720_EVENT_READOUT_BUFFER			 0x0000  /*up to 0x0FFC*/
#define  V1720_Channel_0_ZS_THRES			 0x1024
#define  V1720_Channel_0_ZS_NSAMP			 0x1028
#define  V1720_Channel_0_THRESHOLD			 0x1080
#define  V1720_Channel_0_TIME_OVER_UNDER_THRESHOLD	 0x1084
#define  V1720_Channel_0_STATUS				 0x1088
#define  V1720_Channel_0_AMC_FPGA_FIRMWARE_REVISION	 0x108C
#define  V1720_Channel_0_BUFFER_OCCUPANCY		 0x1094
#define  V1720_Channel_0_DAC			 	 0x1098
#define  V1720_Channel_0_ADC_CONFIGURATION		 0x109C
#define  V1720_Channel_1_ZS_THRES			 0x1124
#define  V1720_Channel_1_ZS_NSAMP			 0x1128
#define  V1720_Channel_1_THRESHOLD			 0x1180
#define  V1720_Channel_1_TIME_OVER_UNDER_THRESHOLD	 0x1184
#define  V1720_Channel_1_STATUS				 0x1188
#define  V1720_Channel_1_AMC_FPGA_FIRMWARE_REVISION	 0x118C
#define  V1720_Channel_1_BUFFER_OCCUPANCY		 0x1194
#define  V1720_Channel_1_DAC			 	 0x1198
#define  V1720_Channel_1_ADC_CONFIGURATION		 0x119C
#define  V1720_Channel_2_ZS_THRES			 0x1224
#define  V1720_Channel_2_ZS_NSAMP			 0x1228
#define  V1720_Channel_2_THRESHOLD			 0x1280
#define  V1720_Channel_2_TIME_OVER_UNDER_THRESHOLD	 0x1284
#define  V1720_Channel_2_STATUS				 0x1288
#define  V1720_Channel_2_AMC_FPGA_FIRMWARE_REVISION	 0x128C
#define  V1720_Channel_2_BUFFER_OCCUPANCY		 0x1294
#define  V1720_Channel_2_DAC			 	 0x1298
#define  V1720_Channel_2_ADC_CONFIGURATION		 0x129C
#define  V1720_Channel_3_ZS_THRES			 0x1324
#define  V1720_Channel_3_ZS_NSAMP			 0x1328
#define  V1720_Channel_3_THRESHOLD			 0x1380
#define  V1720_Channel_3_TIME_OVER_UNDER_THRESHOLD	 0x1384
#define  V1720_Channel_3_STATUS				 0x1388
#define  V1720_Channel_3_AMC_FPGA_FIRMWARE_REVISION	 0x138C
#define  V1720_Channel_3_BUFFER_OCCUPANCY		 0x1394
#define  V1720_Channel_3_DAC			 	 0x1398
#define  V1720_Channel_3_ADC_CONFIGURATION		 0x139C
#define  V1720_Channel_4_ZS_THRES			 0x1424
#define  V1720_Channel_4_ZS_NSAMP			 0x1428
#define  V1720_Channel_4_THRESHOLD			 0x1480
#define  V1720_Channel_4_TIME_OVER_UNDER_THRESHOLD	 0x1484
#define  V1720_Channel_4_STATUS				 0x1488
#define  V1720_Channel_4_AMC_FPGA_FIRMWARE_REVISION	 0x148C
#define  V1720_Channel_4_BUFFER_OCCUPANCY		 0x1494
#define  V1720_Channel_4_DAC			 	 0x1498
#define  V1720_Channel_4_ADC_CONFIGURATION		 0x149C
#define  V1720_Channel_5_ZS_THRES			 0x1524
#define  V1720_Channel_5_ZS_NSAMP			 0x1528
#define  V1720_Channel_5_THRESHOLD			 0x1580
#define  V1720_Channel_5_TIME_OVER_UNDER_THRESHOLD	 0x1584
#define  V1720_Channel_5_STATUS				 0x1588
#define  V1720_Channel_5_AMC_FPGA_FIRMWARE_REVISION	 0x158C
#define  V1720_Channel_5_BUFFER_OCCUPANCY		 0x1594
#define  V1720_Channel_5_DAC			 	 0x1598
#define  V1720_Channel_5_ADC_CONFIGURATION		 0x159C
#define  V1720_Channel_6_ZS_THRES			 0x1624
#define  V1720_Channel_6_ZS_NSAMP			 0x1628
#define  V1720_Channel_6_THRESHOLD			 0x1680
#define  V1720_Channel_6_TIME_OVER_UNDER_THRESHOLD	 0x1684
#define  V1720_Channel_6_STATUS				 0x1688
#define  V1720_Channel_6_AMC_FPGA_FIRMWARE_REVISION	 0x168C
#define  V1720_Channel_6_BUFFER_OCCUPANCY		 0x1694
#define  V1720_Channel_6_DAC			 	 0x1698
#define  V1720_Channel_6_ADC_CONFIGURATION		 0x169C
#define  V1720_Channel_7_ZS_THRES			 0x1724
#define  V1720_Channel_7_ZS_NSAMP			 0x1728
#define  V1720_Channel_7_THRESHOLD			 0x1780
#define  V1720_Channel_7_TIME_OVER_UNDER_THRESHOLD	 0x1784
#define  V1720_Channel_7_STATUS				 0x1788
#define  V1720_Channel_7_AMC_FPGA_FIRMWARE_REVISION	 0x178C
#define  V1720_Channel_7_BUFFER_OCCUPANCY		 0x1794
#define  V1720_Channel_7_DAC			 	 0x1798
#define  V1720_Channel_7_ADC_CONFIGURATION		 0x179C
#define  V1720_CHANNEL_CONFIGURATION			 0x8000
#define  V1720_CHANNEL_CONFIGURATION_BIT_SET		 0x8004
#define  V1720_CHANNEL_CONFIGURATION_BIT_CLEAR	 	 0x8008
#define  V1720_BUFFER_ORGANIZATION			 0x800C
#define  V1720_BUFFER_FREE				 0x8010
#define  V1720_CUSTOM_SIZE				 0x8020
#define  V1720_ACQUISITION_CONTROL			 0x8100
#define  V1720_ACQUISITION_STATUS			 0x8104
#define  V1720_SW_TRIGGER				 0x8108
#define  V1720_TRIGGER_SOURCE_ENABLE_MASK		 0x810C
#define  V1720_FRONT_PANEL_TRIGGER_OUT_ENABLE_MASK	 0x8110
#define  V1720_POST_TRIGGER_SETTING			 0x8114
#define  V1720_FRONT_PANEL_IO_DATA			 0x8118
#define  V1720_FRONT_PANEL_IO_CONTROL			 0x811C
#define  V1720_CHANNEL_ENABLE_MASK			 0x8120
#define  V1720_ROC_FPGA_FIRMWARE_REVISION		 0x8124
#define  V1720_EVENT_STORED			 	 0x812C
#define  V1720_SET_MONITOR_DAC			 	 0x8138
#define  V1720_BOARD_INFO			  	 0x8140
#define  V1720_MONITOR_MODE				 0x8144
#define  V1720_EVENT_SIZE				 0x814C
#define  V1720_VME_CONTROL				 0xEF00
#define  V1720_VME_STATUS				 0xEF04
#define  V1720_BOARD_ID					 0xEF08
#define  V1720_MULTICAST_BASE_ADDRESS_CONTROL 		 0xEF0C
#define  V1720_RELOCATION_ADDRESS			 0xEF10
#define  V1720_INTERRUPT_STATUS_ID			 0xEF14
#define  V1720_INTERRUPT_EVENT_NUMBER			 0xEF18
#define  V1720_BLT_EVENT_NUMBER				 0xEF1C
#define  V1720_SCRATCH					 0xEF20
#define  V1720_SW_RESET					 0xEF24
#define  V1720_SW_CLEAR				 	 0xEF28
#define  V1720_FLASH_ENABLE				 0xEF2C
#define  V1720_FLASH_DATA				 0xEF30
#define  V1720_CONFIGURATION_RELOAD			 0xEF34
#define  V1720_CONFIGURATION_ROM			 0xF000 /*up to 0xF3FC*/


/*
 * QDC data format @Output Buffer
 */

#if 1 /*Little Endian Definition for intel CPU*/



/*
 * Status Register
 */
/*
 * Control Register
 */

#endif //for Little Endian

/*
 * prototype definitions of library functions
 */
int v1720_open(void);
V1720_p v1720_map(int module_number);
int v1720_unmap(int module_number);
int v1720_close(void);

#endif  /* for ifndef V1720_REG_H */
