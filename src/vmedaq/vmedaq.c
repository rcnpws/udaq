/* vmedaq.c ---- vme DAQ with V1190, MADC32, MQDC32, V830                   */
/*								            */
/*  Version 1.00        2013-09-05      by A. Tamii (For Linux2.6)GE-FANUC) */
/*  Version 2.00        2016-04-12      by A. Tamii (Use a config file)     */
/*  Version 3.00        2016-06-19      by A. Tamii (Added MyRIAD)          */
/*  Version 3.10        2017-04-13      by A. Tamii (bugfix for no MyRIAD ) */
/*  Version 3.11        2017-04-14      by A. Tamii (mask the V977 output Ch#0) */
/*  Version 3.12        2017-04-20      by A. Tamii (added config for trig. time sub.) */
/*  Version 4.00        2020-09-10      by A. Tamii and T. Furuno (implemended MQDC32 and DMA Transfer) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>
#include <sys/socket.h>
#include <sys/signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>

#include "router.h"
#include "vmedaq.h"
#include "v1190.h"
#include "madc32.h"
#include "mqdc32.h"
#include "v977.h"
#include "v830.h"
#include "myriad.h"
#include "config.h"

#define PERIODIC_CLEAR                    1  /* periodic clear of the V1190 modules */
//#define PERIODIC_CLEAR                    0  /* periodic clear of the V1190 modules */
#define PERIODIC_CLEAR_INTERVAL_IN_SEC   10  /* periodic clear, period in seconds */

long vmedaq_block_counter = 0;

int geometrical_address = -1;

int debug_mode = 0;

#define MAX_DUMP_BUF 0x1000
unsigned long dump_buffer[MAX_DUMP_BUF];

#define V977_MODULE_NUMBER 0
static V977_p v977=(V977_p)NULL;

#define V830_MODULE_NUMBER 0

#define V830_N_CHANNELS 32          // number of scalers in a module
static int v830_n_scalers = 3;      // number of scalers to be recorded
static V830_p v830=(V830_p)NULL;
static long v830_scalers[V830_N_CHANNELS];

#define IOREG_CH_DAQ_INHIBIT     0
#define IOREG_CH_ALMOST_FULL     1
#define IOREG_CH_BLOCK_TRANSFER  2

#define CLEAR_V1190_AFTER_ALMOST_FULL 1
unsigned int counter_offset=0;

int use_dma_transfer = 0;

long long data_size = 0;
int blk_count = 0;
int v1190_n_modules = 0;
int v1190_module_numbers[V1190_MAX_N_MODULES];
int v1190_module_counters[32];
V1190_p v1190[V1190_MAX_N_MODULES];

#define MAX_N_MADC32_MODULE_COUNTERS 0x10000
int madc32_n_modules = 0;
int madc32_module_numbers[MADC32_MAX_N_MODULES];
int madc32_module_id[MADC32_MAX_N_MODULES];
int madc32_module_counters[MAX_N_MADC32_MODULE_COUNTERS];
MADC32_p madc32[MADC32_MAX_N_MODULES];

#define MAX_N_MQDC32_MODULE_COUNTERS 0x10000
int mqdc32_n_modules = 0;
int mqdc32_module_numbers[MQDC32_MAX_N_MODULES];
int mqdc32_module_id[MQDC32_MAX_N_MODULES];
int mqdc32_module_counters[MAX_N_MQDC32_MODULE_COUNTERS];
MQDC32_p mqdc32[MQDC32_MAX_N_MODULES];


int myriad_n_modules = 0;
MyRIAD_p myriad = (MyRIAD_p)NULL;
int myriad_external_timestamp = 1;   /* external time stamp (1) or internal (0) */

static vme_bus_handle_t dma_bus_handle;
static vme_dma_handle_t dma_handle;
static uint8_t *dma_buffer_p;
static long dma_buffer_size = 0x00010000L;       // DMA Buffer Size 64 KBytes (matched to MQDC32)

static int dma_init(){
  if (vme_init(&dma_bus_handle)) {
    perror("dma_init: Error initializing the VMEbus");
    return -1;
  }

  if (vme_dma_buffer_create(dma_bus_handle, &dma_handle, dma_buffer_size, 0, NULL)) {
    perror("dma_init: Error creating the buffer");
    vme_term(dma_bus_handle);
    return -1;
  }

  dma_buffer_p = vme_dma_buffer_map(dma_bus_handle, dma_handle, 0);
  if (!dma_buffer_p) {
    perror("dma_init: Error mapping the buffer");
    vme_dma_buffer_release(dma_bus_handle, dma_handle);
    vme_term(dma_bus_handle);
    return -1;
  }

  return 0;
}

static int v977_init(){
  if(v977_open()){
    fprintf(stderr, "Error in v977_open()\n");
    return -1;
  }
  fprintf(stderr, "Map V977\n");
  v977 = v977_map(V977_MODULE_NUMBER);
  if(v977==(V977_p)NULL){
    fprintf(stderr, "Error in v977_map()\n");
    return -1;
  }

  v977->control_register = 0x0002;  
  // default setting is 0x0002
  // 0x0002 = I/O Register, Gate is masked, Output(from front panel input) is not masked
  v977->input_set = 0;
  v977->input_mask = 0;

  // mask the signal from input Ch#0 to the output of Ch#0, 14-APR-2017
  v977->output_mask = 0x01; 

  v977->clear_output = 0;
  v977->output_set = 1<<IOREG_CH_DAQ_INHIBIT;
  v977->test_control_register = 0;

  data_size = 0;
  
  return 0;
}

static int v977_exit(){
  if(v977_unmap(V977_MODULE_NUMBER)){
    fprintf(stderr, "Error in v977_unmap()\n");
    return -1;
  }
  if(v977_close()){
    fprintf(stderr, "Error in v977_close()\n");
    return -1;
  }
  return 0;
}

static void v830_clear(){
  int i;
  v830_clear_counters(v830);
  for(i=0; i<V830_N_CHANNELS; i++){
    v830_scalers[i] = 0;
  }
}
  
static int v830_init(){
  unsigned int mask;
  int i;
  if(v830_open()){
    fprintf(stderr, "Error in v830_open()\n");
    return -1;
  }
  fprintf(stderr, "Map V830\n");
  v830 = v830_map(V830_MODULE_NUMBER);
  if(v830==(V830_p)NULL){
    fprintf(stderr, "Error in v830_map()\n");
    return -1;
  }
  v830->controlRegister = 
    V830_CR_CLEAR_MEB
    |V830_CR_HEADER_ENABLE
    |V830_CR_DATA_FORMAT_32
    |V830_CR_ACQMODE_TRIG_RANDOM
    ;
  //v830->channelEnableRegister = 0xFFFFFFFF;
  mask = 0;
  for(i=0; i<v830_n_scalers; i++){
    mask |= 1<<i;
  }

  //v830->channelEnableRegister = 0x00000007;
  fprintf(stderr, "V830 Enable Channel = %.8x\n", mask);
  v830->channelEnableRegister = mask;
  v830->geoRegister = 0x00;
  return 0;
}

static int v830_exit(){
  if(v830_unmap(V830_MODULE_NUMBER)){
    fprintf(stderr, "Error in v830_unmap()\n");
    return -1;
  }
  if(v830_close()){
    fprintf(stderr, "Error in v830_close()\n");
    return -1;
  }
  return 0;
}

static void myriad_clear(){
  if(myriad_n_modules==0 || myriad==(MyRIAD_p)NULL) return;
  MyRIAD_pulsed_control_t pulsed_control;
  pulsed_control.raw.sint = 0x0000;
  pulsed_control.bit.dcm_reset = 1;
  pulsed_control.bit.fifo_reset = 1;
  pulsed_control.bit.sm_lost_lock_reset = 1;
  myriad->pulsed_control = pulsed_control.raw.sint;
  pulsed_control.bit.clear_trig_counter = 1;
  if(myriad_external_timestamp){
  }else{
    pulsed_control.bit.local_ts_reset = 1;
  }

  MyRIAD_gating_register_t gating_register;
  *(unsigned short*)&gating_register = 0;
}
  
static int myriad_init(){
  if(myriad_n_modules==0) return -1;
  char *sval;
  if(myriad_open()){
    fprintf(stderr, "Error in myriad_open()\n");
    return -1;
  }
  fprintf(stderr, "Map MyRIAD\n");
  myriad = myriad_map();
  if(v830==(V830_p)NULL){
    fprintf(stderr, "Error in myriad_map()\n");
    return -1;
  }

  MyRIAD_pulsed_control_t pulsed_control;
  pulsed_control.raw.sint = 0x0000;
  pulsed_control.bit.dcm_reset = 1;
  //pulsed_control.bit.local_ts_reset = 1;
  pulsed_control.bit.fifo_reset = 1;
  pulsed_control.bit.clear_trig_counter = 1;
  pulsed_control.bit.sm_lost_lock_reset = 1;
  myriad->pulsed_control = pulsed_control.raw.sint;

  MyRIAD_gating_register_t gating_register;
  *(unsigned short*)&gating_register = 0;

  sval  = config_get_s_value("myriad_trigger_source", 0, "nim");
  fprintf(stderr, "myriad_trigger_source = %s\n", sval);
  if(!strcasecmp(sval, "nim")){
    gating_register.ts_latch_source = MyRIAD_GATING_REGISTER_TS_LATCH_SOURCE_LOCAL;
    gating_register.trig_in_sel = MyRIAD_GATING_REGISTER_TRIG_IN_SEL_NIM;
  }else if(!strcasecmp(sval, "ecl")){
    gating_register.ts_latch_source = MyRIAD_GATING_REGISTER_TS_LATCH_SOURCE_LOCAL;
    gating_register.trig_in_sel = MyRIAD_GATING_REGISTER_TRIG_IN_SEL_ECL;
  }else if(!strcasecmp(sval, "master")){
    gating_register.ts_latch_source = MyRIAD_GATING_REGISTER_TS_LATCH_SOURCE_MASTER;
  }else{
    fprintf(stderr, "Unknown argument for myriad_trigger_source: %s. Default value will be used: nim.\n", sval);
    gating_register.ts_latch_source = MyRIAD_GATING_REGISTER_TS_LATCH_SOURCE_LOCAL;
    gating_register.trig_in_sel = MyRIAD_GATING_REGISTER_TRIG_IN_SEL_NIM;
  }
  myriad->gating_register = *(unsigned short*)&gating_register;

  sval  = config_get_s_value("myriad_timestamp_source", 0, "external");
  fprintf(stderr, "myriad_timestamp_source = %s\n", sval);
  if(!strcasecmp(sval, "internal")){
    myriad_external_timestamp = 0;
    myriad->serdes_config = myriad->serdes_config | 0x8000;
    myriad->ts_propagation_control = myriad->ts_propagation_control & 0xfffe;
  }else if(!strcasecmp(sval, "external")){
    myriad_external_timestamp = 1;
    myriad->serdes_config = myriad->serdes_config & 0x7fff;
    myriad->ts_propagation_control = myriad->ts_propagation_control | 0x0001;
  }else{
    fprintf(stderr, "Unknown argument for myriad_timestamp_source: %s. Default value will be used: external.\n", sval);
    myriad_external_timestamp = 1;
    myriad->serdes_config = myriad->serdes_config & 0x7fff;
    myriad->ts_propagation_control = myriad->ts_propagation_control | 0x0001;
  }
  
  return 0;
}

static int myriad_exit(){
  if(myriad_n_modules==0 || myriad==(MyRIAD_p)NULL) return -1;
  if(myriad_unmap()){
    fprintf(stderr, "Error in myriad_unmap()\n");
    return -1;
  }
  if(myriad_close()){
    fprintf(stderr, "Error in myriad_close()\n");
    return -1;
  }
  myriad = (MyRIAD_p)NULL;
  return 0;
}


void clear_event_counters(){
  int i;
  for(i=0; i<v1190_n_modules; i++){
    v1190[i]->software_clear = 1;   /* software clear, clear the event counter */
  }
  for(i=0; i<madc32_n_modules; i++){
    madc32[i]->reset_ctr_ab = 1;   /* clear the event counter */
  }
  for(i=0; i<mqdc32_n_modules; i++){
    mqdc32[i]->reset_ctr_ab = 1;   /* clear the event counter */
  }
}

void reset_event_counters(){
  fprintf(stderr, "\nReset the event counters.\n");
  clear_event_counters();
  data_size = 0;
  v830_clear();
  myriad_clear();
  vmedaq_block_counter = 0;
  counter_offset = 0;
}

static time_t prev_time = (time_t)0;
static void periodic_clear(){
  time_t  cur_time;
  time(&cur_time);
  if((cur_time - prev_time) >= PERIODIC_CLEAR_INTERVAL_IN_SEC){
    prev_time = cur_time;
    v977->input_set = 1<<IOREG_CH_ALMOST_FULL;
  }
}

static void sighup(){
  reset_event_counters();
}

void show_counter(){
  int i;
  long n=0;
  return; //20200910
  if(data_size==0) return;
  fprintf(stderr, "event:");
  if(v1190_n_modules>0){
    n = v1190[0]->event_counter;
    fprintf(stderr, "%8ld", n+counter_offset);
    for(i=1; i<v1190_n_modules; i++){
      fprintf(stderr, "%5ld", v1190[i]->event_counter-n);
    }
  }
  if(madc32_n_modules>0){
    n = (madc32[0]->evctr_hi<<16)|(madc32[0]->evctr_lo);
    fprintf(stderr, "%8ld", n+counter_offset);
    for(i=1; i<madc32_n_modules; i++){
      fprintf(stderr, "%5ld", ((madc32[i]->evctr_hi<<16)|(madc32[i]->evctr_lo))-n);
    }
  }
  if(mqdc32_n_modules>0){
    n = (mqdc32[0]->evctr_hi<<16)|(mqdc32[0]->evctr_lo);
    fprintf(stderr, "%8ld", n+counter_offset);
    for(i=1; i<mqdc32_n_modules; i++){
      fprintf(stderr, "%5ld", ((mqdc32[i]->evctr_hi<<16)|(mqdc32[i]->evctr_lo))-n);
    }
  }

  fprintf(stderr, " (%5ld,%12ld,%ld blk)", v830_scalers[0]-n-counter_offset, v830_scalers[0], v830_scalers[1]);
  fprintf(stderr, ", size: %6lld MB (%12lld bytes)", data_size/1024/1024, data_size);
  fprintf(stderr, "\n%c%c", 27, 'M');
}


void vmedaq_write_header(VMEDAQ_HEADER_p header, int type, int sizeInBytes){
  header->id = VMEDAQ_HEADER_ID;
  header->type = type;
  header->counter = vmedaq_block_counter++;
  header->sizeInBytes = sizeInBytes;
}


long v830_check_data(V830_p v830, unsigned long *buffer, int buf_size, int *size){
  V830_MEB_HEADER_t   data;
  unsigned long *memory_buffer;
  unsigned long ldata;
  long event_count = -1;
  int i, j, n, n_words;
  VMEDAQ_HEADER_p vmedaq_header;

  vmedaq_header = (VMEDAQ_HEADER_p)buffer;
  *size = sizeof(VMEDAQ_HEADER_t)/sizeof(long);
  memory_buffer = &v830->meb[0];

  n = v830->MEBEventNumber;

  if(n<=0){
    *size = 0;
    return -1;
  }
  for(i=0; i<n; i++){
    if(buf_size-(*size) < (1+V830_N_CHANNELS)) break;
    ldata = *memory_buffer;
    data = *(V830_MEB_HEADER_t*)&ldata;
    for(j=0; j<V830_N_CHANNELS && data.header==0; j++){
      ldata = *memory_buffer;
      data = *(V830_MEB_HEADER_t*)&ldata;
    }
    if(data.header==0) break;
    n_words = data.n_words;
    for(j=0; j<n_words; j++){
      if(j<V830_N_CHANNELS)
	v830_scalers[j] = *memory_buffer;
    }
    event_count++;

    // v830_scalers[0]    // accepted trigger
    // v830_scalers[1]    // buffer change
    // v830_scalers[2]    // accepted clock

    buffer[(*size)++] = v830_n_scalers;
    for(j=0; j<v830_n_scalers; j++){
      buffer[(*size)++] = v830_scalers[j];
    }
  }
  //fprintf(stderr, "\nn=%d event_count=%ld, size=%d\n", n, event_count, *size);

  vmedaq_write_header(vmedaq_header, VMEDAQ_TYPE_V830, (*size)*sizeof(long));

  return event_count;
}

long v1190_check_data(V1190_p v1190, unsigned long *buffer, int buf_size, int *size, uint64_t base_address){
  V1190_DATA_t data;
  unsigned long *output_buffer;
  unsigned long ldata;
  unsigned long *l_dma_p;
  long event_count = -1;
  int i;
  int end_flag = 0;
  int n_event_stored = 0;
  int n_events = 0;
  int n_data_words;
  int n_data_bytes;
  int offset;
  int transfer_size;
  unsigned long ev_fifo;
  VMEDAQ_HEADER_p vmedaq_header;
  
  vmedaq_header = (VMEDAQ_HEADER_p)buffer;
  *size = sizeof(VMEDAQ_HEADER_t)/sizeof(long);
  output_buffer = v1190->output_buffer;

  n_event_stored = v1190->event_stored;

  if(use_dma_transfer){
    n_events = v1190->event_fifo_stored;
    n_data_words = 0;
    for(i=0; i<n_events; i++){
      ev_fifo = v1190->event_fifo;
      //      fprintf(stderr, "Event FIFO: %.8lx\n", ev_fifo);
      n_data_words += (ev_fifo & 0x0000FFFF);
    }
    n_data_bytes = n_data_words * 4;
    fprintf(stderr, "n_events = %d, n_data_bytes = %ld\n", n_events, n_data_bytes);
    if(dma_buffer_size < n_data_bytes){
      fprintf(stderr, "The v1190 data size(%d) is larger than DMA buffer size(%ld).\n", n_data_bytes, dma_buffer_size);
      n_data_bytes = dma_buffer_size;
      }
    uint64_t output_buffer_offset = ((size_t)&v1190->output_buffer - (size_t)v1190);
    if(n_data_bytes>0){
      /* DMA Transfer */
#define DMA_TRANSFER_SIZE 1024000
      for(offset=0; n_data_bytes>0; offset+=DMA_TRANSFER_SIZE){
	transfer_size = n_data_bytes>DMA_TRANSFER_SIZE ? DMA_TRANSFER_SIZE : n_data_bytes;
	fprintf(stderr, "enter vme_dma_read()\n");
	if (vme_dma_read(dma_bus_handle, dma_handle, 
			 offset, // offset in the dma buffer
			 base_address + output_buffer_offset, // VME ADDRESS of the output_buffer;
			 V1190_MODE, //ADDRESS_MODIFIER (VME_A32UD on 2020.9.10)
			 transfer_size, 0)) {
	  fprintf(stderr, "v1190_check_data: error in vme_dma_read()");
	perror("v1190_check_data: error in vme_dma_read()");
	}
	fprintf(stderr, "exit  vme_dma_read()\n");
	n_data_bytes -= transfer_size;
      }
    }
    //    fprintf(stderr, "exit vme_dma_read()\n");
  }
  
  l_dma_p = (unsigned long*)dma_buffer_p;
  while(!end_flag){ 
    if(use_dma_transfer){
      if(n_data_bytes<=0) break;
      ldata = *l_dma_p++;
      n_data_bytes -= 4;
      //      fprintf(stderr, "Data: %.8lx\n", ldata);
    }else{
      ldata = *output_buffer;
    }
    data = *(V1190_DATA_t*)&ldata;
    //memcpy(buffer, output_buffer, MAX_BUF_SIZE*sizeof(unsigned long));
    
    switch(data.global_header.id){
    case V1190_HEADER_ID_FILLER:
      end_flag = 1;
      break;
    case V1190_HEADER_ID_GH: // Global Header
      event_count = data.global_header.event_count;
      v1190_module_counters[data.global_header.geo]=event_count;
      if(event_count%1000==0 && data.global_header.geo==0) show_counter();
      buffer[(*size)++] = ldata;
      break;
    case V1190_HEADER_ID_GT: // Global Trailer
      buffer[(*size)++] = ldata;
      if((--n_event_stored)<=0)
	end_flag = 1;
      break;
    default:
      buffer[(*size)++] = ldata;
      break;
    }
    if(*size>=buf_size) break;
  }
  if(event_count<0){
    *size = 0;
    return event_count;
  }

  vmedaq_write_header(vmedaq_header, VMEDAQ_TYPE_V1190, (*size)*sizeof(long));
  return event_count;
}

long madc32_check_data(MADC32_p madc32, int mid, unsigned long *l_buffer, int l_buf_size, int *l_size){
  unsigned int data;
  unsigned int buffer_data_length;
  unsigned int *buffer;
  unsigned int buf_size;
  unsigned int *header_pos = (unsigned int*)NULL;
  long event_count = -1;
  int i;
  int module_id=-1;
  int size;
  int eoe_data;
  VMEDAQ_HEADER_p vmedaq_header;
  MADC32_HEADER_SIGNATURE_p    signature;
  MADC32_DATA_HEADER_p         header;
  MADC32_DATA_EVENT_p          event;
  MADC32_EXTENDED_TIME_STAMP_p extended_time_stamp;
  MADC32_FILL_p                fill;
  MADC32_END_OF_EVENT_p        end_of_event;

  int header_flag;
  int end_of_event_flag;
  int no_header_flag;

  buffer = (unsigned int*)l_buffer;
  buf_size = l_buf_size*sizeof(long)/sizeof(int);

  vmedaq_header = (VMEDAQ_HEADER_p)buffer;
  size = sizeof(VMEDAQ_HEADER_t)/sizeof(int);

  buffer_data_length = madc32->buffer_data_length;

  // for fixing automatically the no header error event
  header_flag = 0;
  no_header_flag = 0;
  end_of_event_flag = 0;

  for(i=0; i<buffer_data_length; i++){ 
    data = madc32->fifo_read;
    buffer[size++] = data;
    signature = (MADC32_HEADER_SIGNATURE_p)&data;
    switch(signature->header){
    case MADC32_HEADER_SIGNATURE_HEADER:
      if(header_flag==1 && end_of_event_flag==0){
	// no end_of_event
	// manually generate an end of event data
#if 0 // No need to generate EoE. instead a header is generated in the next event. 13-APR-2016
	end_of_event = (MADC32_END_OF_EVENT_p)&eoe_data;
	end_of_event->header_signature = MADC32_HEADER_SIGNATURE_END_OF_EVENT;
	end_of_event->event_counter = 0;
	if(0<=module_id && module_id<MAX_N_MADC32_MODULE_COUNTERS){
	  end_of_event->event_counter = ++madc32_module_counters[module_id];
	}
	buffer[size] = buffer[size-1];
	buffer[size-1] = eoe_data;
	size++;
	if(header_pos!=(unsigned int*)NULL){
	  header = (MADC32_DATA_HEADER_p)header_pos;
	  header->subheader = MADC32_SUBHEADER_NO_END_OF_EVENT;  /* for the manually generate end of event */
	}
#endif
	header_flag = 0;
	no_header_flag = 0;
	end_of_event_flag = 1;
      }
      header = (MADC32_DATA_HEADER_p)&data;
      module_id = header->module_id;
      header_flag = 1;
      end_of_event_flag = 0;
      break;
    case MADC32_HEADER_SIGNATURE_DATA: 
      if(!header_flag && !no_header_flag){
	no_header_flag = 1;
	header_pos = &buffer[size-1];  // pointer of the header position
	buffer[size++] = data;
      }
      switch(signature->subheader){
      case MADC32_SUBHEADER_EVENT:
	event = (MADC32_DATA_EVENT_p)&data;
	break;
      case MADC32_SUBHEADER_EXTENDED_TIME_STAMP:
	extended_time_stamp = (MADC32_EXTENDED_TIME_STAMP_p)&data;
	break;
      case MADC32_SUBHEADER_FILL:
	fill = (MADC32_FILL_p)&data;
	break;
      default:
	//fprintf(fd, "Unknown subheader = 0x%.3x", signature->subheader);
	break;
      }
      break;
    case MADC32_HEADER_SIGNATURE_END_OF_EVENT:
      end_of_event = (MADC32_END_OF_EVENT_p)&data;
      event_count = end_of_event->event_counter;
      if(0<=module_id && module_id<MAX_N_MADC32_MODULE_COUNTERS){
	module_id = mid;
      }
      madc32_module_counters[module_id]=event_count;
      if(!header_flag && no_header_flag){
	header = (MADC32_DATA_HEADER_p)header_pos;
	header->n_data_words = ((size_t)&buffer[size]-(size_t)header_pos)/sizeof(int)-1;
	header->adc_resolution = madc32->adc_resolution & 0x07;
	header->output_format = 0;
	header->module_id = mid;
	header->subheader = MADC32_SUBHEADER_NO_HEADER;  /* for the manually generated header */
	header->header_signature = 1;
      }
      header_flag = 0;
      no_header_flag = 0;
      end_of_event_flag = 1;
      break;
    default:
      //fprintf(fd, "Unknown header = 0x%.1x", signature->header);
      break;
      /////////
    }
    if(size>=buf_size) break;
  }

  if(header_flag==1 && end_of_event_flag==0){
    // no end_of_event
    // manually create an end of event data
    end_of_event = (MADC32_END_OF_EVENT_p)&eoe_data;
    end_of_event->header_signature = MADC32_HEADER_SIGNATURE_END_OF_EVENT;
    end_of_event->event_counter = ++madc32_module_counters[mid];
    buffer[size++] = eoe_data;
    if(header_pos!=(unsigned int*)NULL){
      header = (MADC32_DATA_HEADER_p)header_pos;
      header->subheader = MADC32_SUBHEADER_NO_END_OF_EVENT;  /* for the artificially generated end of event */
    }
  }
  
  if(event_count<0){
    *l_size = 0;
    return event_count;
  }
  if((size&0x0001)==1){
    buffer[size++] = 0x00; /* Filler for 64bit boundary*/
  }
  vmedaq_write_header(vmedaq_header, VMEDAQ_TYPE_MADC32, size*sizeof(int));
  *l_size = size*sizeof(int)/sizeof(long);
  
  return event_count;
}


long mqdc32_check_data(MQDC32_p mqdc32, int mid, unsigned long *l_buffer, int l_buf_size, int *l_size){
  unsigned int data;
  unsigned int buffer_data_length;
  unsigned int *buffer;
  unsigned int buf_size;
  unsigned int *header_pos = (unsigned int*)NULL;
  long event_count = -1;
  int i;
  int module_id=-1;
  int size;
  int eoe_data;
  VMEDAQ_HEADER_p vmedaq_header;
  MQDC32_HEADER_SIGNATURE_p    signature;
  MQDC32_DATA_HEADER_p         header;
  MQDC32_DATA_EVENT_p          event;
  MQDC32_EXTENDED_TIME_STAMP_p extended_time_stamp;
  MQDC32_FILL_p                fill;
  MQDC32_END_OF_EVENT_p        end_of_event;

  int header_flag;
  int end_of_event_flag;
  int no_header_flag;

  buffer = (unsigned int*)l_buffer;
  buf_size = l_buf_size*sizeof(long)/sizeof(int);

  vmedaq_header = (VMEDAQ_HEADER_p)buffer;
  size = sizeof(VMEDAQ_HEADER_t)/sizeof(int);

  buffer_data_length = mqdc32->buffer_data_length;

  // for fixing automatically the no header error event
  header_flag = 0;
  no_header_flag = 0;
  end_of_event_flag = 0;

  for(i=0; i<buffer_data_length; i++){ 
    data = mqdc32->fifo_read;
    buffer[size++] = data;
    signature = (MQDC32_HEADER_SIGNATURE_p)&data;
    switch(signature->header){
    case MQDC32_HEADER_SIGNATURE_HEADER:
      if(header_flag==1 && end_of_event_flag==0){
	// no end_of_event
	// manually generate an end of event data
#if 0 // No need to generate EoE. instead a header is generated in the next event. 13-APR-2016
	end_of_event = (MQDC32_END_OF_EVENT_p)&eoe_data;
	end_of_event->header_signature = MQDC32_HEADER_SIGNATURE_END_OF_EVENT;
	end_of_event->event_counter = 0;
	if(0<=module_id && module_id<MAX_N_MQDC32_MODULE_COUNTERS){
	  end_of_event->event_counter = ++mqdc32_module_counters[module_id];
	}
	buffer[size] = buffer[size-1];
	buffer[size-1] = eoe_data;
	size++;
	if(header_pos!=(unsigned int*)NULL){
	  header = (MQDC32_DATA_HEADER_p)header_pos;
	  header->subheader = MQDC32_SUBHEADER_NO_END_OF_EVENT;  /* for the manually generate end of event */
	}
#endif
	header_flag = 0;
	no_header_flag = 0;
	end_of_event_flag = 1;
      }
      header = (MQDC32_DATA_HEADER_p)&data;
      module_id = header->module_id;
      header_flag = 1;
      end_of_event_flag = 0;
      break;
    case MQDC32_HEADER_SIGNATURE_DATA: 
      if(!header_flag && !no_header_flag){
	no_header_flag = 1;
	header_pos = &buffer[size-1];  // pointer of the header position
	buffer[size++] = data;
      }
      switch(signature->subheader){
      case MQDC32_SUBHEADER_EVENT:
	event = (MQDC32_DATA_EVENT_p)&data;
	break;
      case MQDC32_SUBHEADER_EXTENDED_TIME_STAMP:
	extended_time_stamp = (MQDC32_EXTENDED_TIME_STAMP_p)&data;
	break;
      case MQDC32_SUBHEADER_FILL:
	fill = (MQDC32_FILL_p)&data;
	break;
      default:
	//fprintf(fd, "Unknown subheader = 0x%.3x", signature->subheader);
	break;
      }
      break;
    case MQDC32_HEADER_SIGNATURE_END_OF_EVENT:
      end_of_event = (MQDC32_END_OF_EVENT_p)&data;
      event_count = end_of_event->event_counter;
      if(0<=module_id && module_id<MAX_N_MQDC32_MODULE_COUNTERS){
	module_id = mid;
      }
      mqdc32_module_counters[module_id]=event_count;
      if(!header_flag && no_header_flag){
	header = (MQDC32_DATA_HEADER_p)header_pos;
	header->n_data_words = ((size_t)&buffer[size]-(size_t)header_pos)/sizeof(int)-1;
	header->adc_resolution = mqdc32->adc_resolution & 0x07;
	header->output_format = 0;
	header->module_id = mid;
	header->subheader = MQDC32_SUBHEADER_NO_HEADER;  /* for the manually generated header */
	header->header_signature = 1;
      }
      header_flag = 0;
      no_header_flag = 0;
      end_of_event_flag = 1;
      break;
    default:
      //fprintf(fd, "Unknown header = 0x%.1x", signature->header);
      break;
      /////////
    }
    if(size>=buf_size) break;
  }

  if(header_flag==1 && end_of_event_flag==0){
    // no end_of_event
    // manually create an end of event data
    end_of_event = (MQDC32_END_OF_EVENT_p)&eoe_data;
    end_of_event->header_signature = MQDC32_HEADER_SIGNATURE_END_OF_EVENT;
    end_of_event->event_counter = ++mqdc32_module_counters[mid];
    buffer[size++] = eoe_data;
    if(header_pos!=(unsigned int*)NULL){
      header = (MQDC32_DATA_HEADER_p)header_pos;
      header->subheader = MQDC32_SUBHEADER_NO_END_OF_EVENT;  /* for the artificially generated end of event */
    }
  }
  
  if(event_count<0){
    *l_size = 0;
    return event_count;
  }
  if((size&0x0001)==1){
    buffer[size++] = 0x00; /* Filler for 64bit boundary*/
  }

  vmedaq_write_header(vmedaq_header, VMEDAQ_TYPE_MQDC32, size*sizeof(int));
  *l_size = size*sizeof(int)/sizeof(long);
  
  return event_count;
}

long myriad_check_data(MyRIAD_p myriad, unsigned long *buffer, int buf_size, int *size){
  unsigned short data;
  unsigned short data1;
  unsigned short data2;
  unsigned short data3;
  long event_count = 0;
  int i;
  VMEDAQ_HEADER_p vmedaq_header;

  if(myriad_n_modules==0 || myriad==(MyRIAD_p)NULL) return -1;

  vmedaq_header = (VMEDAQ_HEADER_p)buffer;
  *size = sizeof(VMEDAQ_HEADER_t)/sizeof(long);

  while(buf_size-(*size)>=2){
    // find a header
    for(i=0; i<3; i++){
      data = myriad->fifo_access;
      //      fprintf(stderr, "myriad data: 0x%.4x\n", data);
      if(data==MYRIAD_HEADER_ID) break;
    }
    if(i>=3) break;
    //if(i!=0) fprintf(stderr, "myriad_check_data: skipped data for %d words\n", i);
    
    data1 = myriad->fifo_access;
    if(data1==MYRIAD_HEADER_ID){
      fprintf(stderr, "myriad_check_data: skipped data for repeating header ID (0x%.4x)\n", data1);
      break;
    }
  
    data2 = myriad->fifo_access;
    data3 = myriad->fifo_access;
    ((unsigned short *)&buffer[*size])[0] = data;
    ((unsigned short *)&buffer[*size])[1] = data1;
    ((unsigned short *)&buffer[*size])[2] = data2;
    ((unsigned short *)&buffer[*size])[3] = data3;
    *size += 2;

    // fprintf(stderr, "myriad check data: event = %.4x %.4x %.4x %.4x\n", data, data1, data2, data3);

    event_count++;
  }


  if(event_count>0){
    //fprintf(stderr, "myriad check data: event_count = %ld\n", event_count);
    vmedaq_write_header(vmedaq_header, VMEDAQ_TYPE_MYRIAD, (*size)*sizeof(long));
    return event_count;
  }else{
    *size = 0;
    return -1;
  }
}

int write_to_sd(int sd, unsigned long *buffer, int lsize){
  rtr_inet_info3_t  info3;
  int size;
  int count;
  size_t wsize;
  int reply;
  int n;
  unsigned char *ptr;

  blk_count++;
  size = lsize*sizeof(unsigned long);
  // fprintf(stderr, "count = %d, size = %d\n", blk_count, size);
  info3.blk_count = htonl(blk_count);
  info3.blk_size = htonl(size);
  wsize = write(sd, &info3, sizeof(info3));    /* write info3 */
  if(wsize!=sizeof(info3)){
    perror("write");
    fprintf(stderr, "wsize = %d, size = %d", wsize, sizeof(info3));  
    return -1;
  }
  wsize = write(sd, buffer, size);             /* write data via TCP/IP */
  if(wsize!=size){       
    perror("write");
    fprintf(stderr, "write. wsize = %d, size = %d", wsize, size);  
    return -1;
  }
  
  count = 0;
  ptr = (unsigned char*)&reply;
  while(count<sizeof(int)){
    n = read(sd, ptr, sizeof(reply)-count);
    if(n==0){
      fprintf(stderr, "No reply from the client.\n");
      return -1;
    }
    if(n<0){
      perror("read"); // connection is closed
      return -1;
    }
    count += n;
    ptr = &ptr[n];
  }
  reply = ntohl(reply);
  if(reply!=0){
    fprintf(stderr, "Error reply from the client(%d).\n", reply);
  }
  return size/sizeof(unsigned long);
}

#define VMEDAQ_COUNTER_OFFSET_SIZE  (sizeof(VMEDAQ_HEADER_t)+sizeof(unsigned int))
void send_counter_offset(int sd, FILE *fd){
  unsigned int buffer[(VMEDAQ_COUNTER_OFFSET_SIZE)/sizeof(unsigned int)];
  VMEDAQ_HEADER_p header;
  unsigned int *offset;
  int size, written_size;
  header = (VMEDAQ_HEADER_p)buffer;
  offset = (unsigned int *)&header[1];

  counter_offset = *offset = v830_scalers[0];
  if(sd){
    vmedaq_write_header(header, VMEDAQ_TYPE_COUNTER_OFFSET, VMEDAQ_COUNTER_OFFSET_SIZE);
    write_to_sd(sd, (unsigned long*)buffer, VMEDAQ_COUNTER_OFFSET_SIZE/sizeof(unsigned long));
  }else{ // corrected on 2020.09.08
    vmedaq_write_header(header, VMEDAQ_TYPE_COUNTER_OFFSET, VMEDAQ_COUNTER_OFFSET_SIZE);
    size = VMEDAQ_COUNTER_OFFSET_SIZE;
    written_size = fwrite(buffer, sizeof(char), size, fd);
    if(written_size != size){
      perror("write");
      return;
    }
    fflush(fd);
  }
}

#define DATA_BUF_SIZE 16384*16
static unsigned long data_buffer[DATA_BUF_SIZE];


int write_data(int sd, FILE *fd, int size){
  int sent_size;
  int written_size;
  if(size<=0) return 0;
  if(sd){
    sent_size = write_to_sd(sd, data_buffer, size);
    if(sent_size!=size) return -1;
  }else{
    written_size = fwrite(data_buffer, sizeof(unsigned long), size, fd);
    if(written_size != size){
      perror("write");
      return -1;
    }
    fflush(fd);
  }
  data_size += size*sizeof(unsigned long);
  return 0;
}


int read_data(int sd, FILE *fd){
  int n_data_modules;
  long event_count;
  int i;
  int v977_val;
  int size;
  
  n_data_modules = 0;
  
  v977_val = v977->singlehit_read;
  
  event_count = v830_check_data(v830, data_buffer, DATA_BUF_SIZE, &size);
  if(write_data(sd, fd, size)<0) return -1;
  if(event_count>=0) n_data_modules++;

  for(i=0; i<v1190_n_modules; i++){
    event_count = v1190_check_data(v1190[i], data_buffer, DATA_BUF_SIZE, &size, V1190_BASE(v1190_module_numbers[i]));
    if(write_data(sd, fd, size)<0) return -1;
    if(event_count>=0) n_data_modules++;
  }
  
  for(i=0; i<madc32_n_modules; i++){
    event_count = madc32_check_data(madc32[i], madc32_module_id[i], data_buffer, DATA_BUF_SIZE, &size);
    if(write_data(sd, fd, size)<0) return -1;
    if(event_count>=0) n_data_modules++;
  }

  for(i=0; i<mqdc32_n_modules; i++){
    event_count = mqdc32_check_data(mqdc32[i], mqdc32_module_id[i], data_buffer, DATA_BUF_SIZE, &size);
    if(write_data(sd, fd, size)<0) return -1;
    if(event_count>=0) n_data_modules++;
  }

  if(myriad_n_modules==1){
    event_count = myriad_check_data(myriad, data_buffer, DATA_BUF_SIZE, &size);
    if(write_data(sd, fd, size)<0) return -1;
    if(event_count>=0) n_data_modules++;
  }
    
    //fprintf(stderr, "n_data_modules = %d\n", n_data_modules);
  if(n_data_modules==0){
    //fprintf(stderr, "v977_val = %x\n", v977_val);
    if(v977_val&(1<<IOREG_CH_ALMOST_FULL)){
#if CLEAR_V1190_AFTER_ALMOST_FULL
      send_counter_offset(sd, fd);
      clear_event_counters();
#endif
      v977_val=v977->singlehit_read_clear;  // clear V977
    }
    //fprintf(stderr, "val = %d\n", val);
    usleep(10);
  }
  return n_data_modules;
}

int get_socket(){
  struct sockaddr_in name;
  struct servent *serv_p;
  int ld;
  
  /* Create a socket */
  if((ld=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))<0){
    perror("socket");
    exit(1);
  }
  
  /* Get service port */
  if((serv_p=getservbyname(ROUTER_DAQDATA_SERVICE, "tcp"))==NULL){
    fprintf(stderr, "Cannot find service %s. The default port %d is used.\n",
	    ROUTER_DAQDATA_SERVICE, ROUTER_DAQDATA_PORT);
    name.sin_port = htons(ROUTER_DAQDATA_PORT);
  }else{
    name.sin_port = serv_p->s_port;
  }
  name.sin_family = AF_INET;
  name.sin_addr.s_addr = INADDR_ANY;
  
  /* Bind */
  while(1){
    if(bind(ld, (struct sockaddr*)&name, sizeof(name))<0){
      perror("bind");
      name.sin_port = htons(ntohs(name.sin_port)+1);
      continue;
    }
    break;
  }
  
  /* Listen */
  if(listen(ld, 1)<0){
    close(ld);
    perror("listen");
    exit(1);
  }
  
  fprintf(stderr, "Waiting for connections at port %d.\n", ntohs(name.sin_port));
  return ld;
}

int accept_connection(int ld){
  struct sockaddr addr;
  int addrlen;
  int sd;
  rtr_inet_info1_t  info1;
  rtr_inet_info2_t  info2;
  struct hostent  *hp;
  struct sockaddr_in *in_p;
  int   count, n;
  unsigned char  *ptr;
  char  name[256];
  char  str[256];

  addrlen = sizeof(addr);
  if((sd=accept(ld, &addr, (socklen_t*)&addrlen))<0){
    close(ld);
    perror("accept");
    exit(1);
  }
  /* get client host name */
  in_p = (struct sockaddr_in*)&addr;
  hp = gethostbyaddr((char*)&in_p->sin_addr, sizeof(struct in_addr), AF_INET);
  if(hp!=NULL && *hp->h_aliases!=(char*)NULL)
    strcpy(name, *hp->h_aliases);
  else
    strcpy(name, (char*)(long)inet_ntoa(in_p->sin_addr));
  
  fprintf(stderr, "Connection request from %s\n", name);

  /* read request from client */
  count = 0;
  ptr  = (unsigned char*)&info1;
  while(count<sizeof(info1)){
    if((n=read(sd, ptr, sizeof(info1)-count))<0){  /* wait for the reply */
      perror("read");
      return 0;
    }
    if(n==0){
      fprintf(stderr, "Connection was closed from the client.\n");
      return 0;
    }
    count += n;
    ptr = &ptr[n];
  }
  
  fprintf(stderr, "Client: %s@%s:%s\n", info1.user, name, info1.program);
  
  /* check the version */
  if(ntohl(info1.ver) != ROUTER_INET_VERSION_2){
    info2.result = htonl(E_VERSION);
    write(sd, &info2, sizeof(info2));
    fprintf(stderr, "Client version %d may not be supported.\n", htonl(E_VERSION));
  }
  
  if((ptr=(unsigned char*)strchr(name, '.'))!=(unsigned char*)NULL)   /* get the host name */
    *ptr = 0x00;
  strncpy(str, (char*)info1.user, sizeof(info1.user));
  strcat(str, "@");
  strcat(str, name);
  strcat(str, ":");
  strncat(str, (char*)info1.program, sizeof(info1.program));
  
  info2.result = htonl(E_NOERR);
  info2.blk_size = htonl(0);
  info2.res1 = info2.res2 = 0;
  
  /* send reply */
  if(write(sd, &info2, sizeof(info2))<0){
    perror("write");
    return 0;
  }
  return sd;
}

void usage(char *program_name){
  fprintf(stderr, "%s ... read v1190 TDC and output to stdout.\n", program_name);
  fprintf(stderr, "Usage: %s [-ndih] [-c config_file_name] [-o file_name] \n", program_name);
  fprintf(stderr, " -n: (network option) send data to a client via tcp/ip.\n");
  fprintf(stderr, " -c: specify the config file name (default = ~/conf/`hostname -s`.conf)\n");
  fprintf(stderr, " -o: write output into the file when no -n option (default=stdout)\n");
  fprintf(stderr, " -d: debug mode\n");
  fprintf(stderr, " -h: show this help\n");
  fprintf(stderr, " -i: initialize V1190, MADC32, and MQDC32\n");
  fprintf(stderr, " file_name: file to write data without -n option\n");
}

void v1190_init(){
  int i;
  int t_clock = 25;
#if 0 // old settings on 12-APR-2016
  //unsigned short match_window_width = 1000/t_clock;     // from 1 to 4095
  //unsigned short match_window_width = 2200/t_clock;     // (used from 2015.6.10 20:15)
  // unsigned short match_window_width = 4000/t_clock;     // from 1 to 4095 (used up to 2015.6.10 20:15)
  unsigned short match_window_width = 5000/t_clock;     // from 1 to 4095 (used up for CAGRA 2015.7.2)
  //  unsigned short match_window_width = 3000/t_clock;     // from 1 to 4095 (for E429)  E423 upto 20150622 11:29
  //unsigned short match_window_width = 4000/t_clock;     // E423 from 20150622 12:01 
  //
  //short window_offset = -100/t_clock;                   // from -2048 to 40
  //short window_offset = -2000/t_clock;                   // (used from 2015.6.10 20:15)
  short window_offset = -4000/t_clock;                   // (used up to 2015.6.10 20:15)
  //short window_offset = -2800/t_clock;                   // from -2048 to 40 (for E429) E423 upto 20150622 11:29
  //short window_offset = -3800/t_clock;                   // (used up to 2015.6.10 20:15)
  unsigned short extra_search_window_width = 0/t_clock; // no greater than 50
  unsigned short reject_margin = 100/t_clock;           // should be >= 1
  unsigned short detection = V1190_OPECODE_DETECTION_ONLY_LEADING; // only leading
  unsigned short almost_full_level = 8192 ;              /* 16384 words = a half of the memory */

  // DANGEROUS!!!
  // This register is used to limit 
  // *** the maximum number of hits per event per TDC (32ch) ***
  // To record 3 hits (usual size of cluster) with leading edge,
  // the maximum number of hits should be larger than 6.
  // Please use the register value larger than 3. (4,5,6,7,8,9).
  unsigned short max_number_hits_w = 9;      /* maximum number of hits of TDC for a trigger used from 2015.6.20 19:52*/
  //  unsigned short max_number_hits_w = 3;     /* maximum number of hits of TDC for a trigger used from 2015.6.10 20:15*/
  //unsigned short max_number_hits_w = 5;       /* maximum number of hits of TDC for a trigger used up to 2015.6.10 200:15 */
  //unsigned short max_number_hits_w = 7;         /* maximum number of hits of TDC for a trigger for CAGRA 2015.7.2 */
  // See Sec. 5.5.4 of the V1190 manual
  // 0 = 0000  =   0 hits
  // 1 = 0001  =   1 hits
  // 2 = 0010  =   2 hits
  // 3 = 0011  =   4 hits
  // 4 = 0100  =   8 hits
  // 5 = 0101  =  16 hits
  // 6 = 0110  =  32 hits
  // 7 = 0111  =  64 hits
  // 8 = 1000  = 128 hits
  // 9 = 1001  =  no limit
#endif
  char *sval;
  int geo;
  unsigned short match_window_width = 5000/t_clock;     // from 1 to 4095 (used up for CAGRA 2015.7.2)
  short window_offset = -4000/t_clock;                  // (used up to 2015.6.10 20:15)
  unsigned short extra_search_window_width = 0/t_clock; // no greater than 50
  unsigned short reject_margin = 100/t_clock;           // should be >= 1
  unsigned short detection = V1190_OPECODE_DETECTION_ONLY_LEADING; // only leading
  unsigned short almost_full_level = 8192 ;             // 16384 words = a half of the memory 
  unsigned short max_number_hits_w = 9;                 // maximum number of hits of TDC for a trigger used from 2015.6.20 19:52
  int trigger_time_subtraction = 0;                     // trigger time subtraction (added on 2017.4.20)

  
  fprintf(stderr, "\n[V1190]\n");
  for(i=0; i<v1190_n_modules; i++){
    geo = config_get_l_value("v1190_geo", i, i);
    match_window_width        = config_get_l_value("v1190_match_window_width",        i,  5000)/t_clock;
    window_offset             = config_get_l_value("v1190_window_offset",             i, -4000)/t_clock;
    extra_search_window_width = config_get_l_value("v1190_extra_search_window_width", i,     0)/t_clock;
    reject_margin             = config_get_l_value("v1190_reject_margin",             i,   100)/t_clock;
    almost_full_level         = config_get_l_value("v1190_almost_full_level",         i,  8192);
    max_number_hits_w         = config_get_l_value("v1190_max_number_hits_w",         i,  9);
    sval                      = config_get_s_value("v1190_detection",                 i,   "leading");
    if(!strcasecmp(sval, "pair")){
      detection = V1190_OPECODE_DETECTION_PAIR_MODE;
    }else if(!strcasecmp(sval, "leading")){
      detection = V1190_OPECODE_DETECTION_ONLY_LEADING;
    }else if(!strcasecmp(sval, "triling")){
      detection = V1190_OPECODE_DETECTION_ONLY_TRAILING;
    }else if(!strcasecmp(sval, "both")){
      detection = V1190_OPECODE_DETECTION_TRAILING_AND_LEADING;
    }else{
      fprintf(stderr, "Unknown argument for v1190_detection[%d]: %s. Default value will be used: leading.\n", i, sval);
      detection = V1190_OPECODE_DETECTION_ONLY_LEADING;
    }

    // added on 2017.04.20
    sval                      = config_get_s_value("v1190_trigger_time_subtraction",  i,   "false");

    if(!strcasecmp(sval, "true")){
      trigger_time_subtraction = 1;
    }else if(!strcasecmp(sval, "false")){
      trigger_time_subtraction = 0;
    }else{
      fprintf(stderr, "Unknown argument for v1190_trigger_time_subtraction[%d]: %s. Default value will be used: false.\n", i, sval);
      trigger_time_subtraction = 0;
    }

    fprintf(stderr, "Initializing module: %d\n", v1190_module_numbers[i]);
    //fprintf(stderr, "v1190 = %x\n", (size_t)v1190[i]);
    //fprintf(stderr, "Initialization Parameters\n");
    fprintf(stderr, "  geometrical address (geo) = %6d\n", geo);
    fprintf(stderr, "  Match window width        = %6d * %2d = %8d nsec\n", 
	    match_window_width, t_clock, match_window_width*t_clock);
    fprintf(stderr, "  Window offset             = %6d * %2d = %8d nsec\n", 
	    window_offset, t_clock, window_offset*t_clock);
    fprintf(stderr, "  Extra search window width = %6d * %2d = %8d nsec\n", 
	    extra_search_window_width, t_clock, extra_search_window_width*t_clock);
    fprintf(stderr, "  Reject margin             = %6d * %2d = %8d nsec\n", 
	    reject_margin, t_clock, reject_margin*t_clock);
    fprintf(stderr, "  Detection                 = %6d (=%s)\n", detection,
	    detection==V1190_OPECODE_DETECTION_PAIR_MODE ? "pair mode" :
	    detection==V1190_OPECODE_DETECTION_ONLY_TRAILING ? "only trailing" :
	    detection==V1190_OPECODE_DETECTION_ONLY_LEADING ? "only leading" :
	    detection==V1190_OPECODE_DETECTION_TRAILING_AND_LEADING ? "leading and trailing" :
	    "Unknown");

    fprintf(stderr, "  Almost full level         = %6d \n", almost_full_level);
    fprintf(stderr, "  Maximum number of hits    = %6d (=%s) \n", max_number_hits_w, max_number_hits_s[max_number_hits_w&0x0F]);
    fprintf(stderr, "  Trigger time subtraction  = %6s \n", trigger_time_subtraction ? "true" : "false");
      
    fprintf(stderr, "  Enable event fifo\n");
    v1190[i]->control_register |= 0x0100;  // enable event fifo;
    v1190[i]->output_prog_control = 2; /* select Almost Full for output */
    v1190[i]->almost_full_level = almost_full_level;
    v1190[i]->geo_address_register = geo;
    v1190_micro_handshake_write(v1190[i], V1190_OPECODE_TRIG_MATCH);
    usleep(700000);
    
    v1190_micro_handshake_write(v1190[i], V1190_OPECODE_SET_WIN_WIDTH);
    usleep(700000);
    v1190_micro_handshake_write(v1190[i], match_window_width);
    usleep(700000);
    
    v1190_micro_handshake_write(v1190[i], V1190_OPECODE_SET_WIN_OFFS);
    usleep(700000);
    v1190_micro_handshake_write(v1190[i], window_offset);
    usleep(700000);
    
    v1190_micro_handshake_write(v1190[i], V1190_OPECODE_SET_SW_MARGIN);
    usleep(700000);
    v1190_micro_handshake_write(v1190[i], extra_search_window_width);
    usleep(700000);
    
    v1190_micro_handshake_write(v1190[i], V1190_OPECODE_SET_REJ_MARGIN);
    usleep(700000);
    v1190_micro_handshake_write(v1190[i], reject_margin);
    usleep(700000);
    v1190_micro_handshake_write(v1190[i], V1190_OPECODE_SET_DETECTION);
    usleep(700000);
    v1190_micro_handshake_write(v1190[i], detection);
    usleep(700000);
    
    v1190_micro_handshake_write(v1190[i], V1190_OPECODE_SET_EVENT_SIZE);
    usleep(700000);
    v1190_micro_handshake_write(v1190[i], max_number_hits_w);
    usleep(700000);

    // added on 2017.4.20
    if(trigger_time_subtraction)
      v1190_micro_handshake_write(v1190[i], V1190_OPECODE_EN_SUB_TRG);
    else
      v1190_micro_handshake_write(v1190[i], V1190_OPECODE_DIS_SUB_TRG);
    usleep(700000);

    fprintf(stderr, "done\n");
  }
}

int madc32_decode_delay(int delay_encoded){
  if(delay_encoded==0) return 25;
  return(150+(delay_encoded-1)*50);
}

int madc32_encode_delay(int delay_in_nsec){
  if(delay_in_nsec<150) return 0;
  return (delay_in_nsec-150)/50+1;
}

int madc32_decode_width(int width_encoded){
  return width_encoded*50;
}

int madc32_encode_width(int width_in_nsec){
  return width_in_nsec/50;
}


void madc32_init(){
  int i, ch;
  char *sval;
  int module_id;
  int irq_threshold;
  int multi_event;
  int marking_type;
  int adc_resolution;
  int delay0;
  int delay1;
  int width0;
  int width1;
  int use_gg;
  int input_range;
  int nim_gat1_osc;
  int nim_fc_reset;
  int nim_busy;
  int threshold;
  int ts_sources;

  fprintf(stderr, "\n[MADC32]\n");
  for(i=0; i<madc32_n_modules; i++){
    module_id      = config_get_l_value("madc32_module_id",      i, i);
    irq_threshold  = config_get_l_value("madc32_irq_threshold",  i, MADC32_IRQ_THRESHOLD_DEFAULT);

    sval = config_get_s_value("madc32_multi_event",    i, "unlimited");
    if(!strcasecmp(sval, "none")) multi_event = MADC32_MULTI_EVENT_NO;
    else if(!strcasecmp(sval, "unlimited")) multi_event = MADC32_MULTI_EVENT_YES_UNLIMITED;
    else if(!strcasecmp(sval, "limited")) multi_event = MADC32_MULTI_EVENT_YES_LIMITED;
    else{
      fprintf(stderr, "Invalid value for madc32_multi_event: %s\n", sval);
      multi_event = MADC32_MULTI_EVENT_YES_UNLIMITED;
    }

    sval = config_get_s_value("madc32_marking_type",   i, "event_counter");
    if(!strcasecmp(sval, "event_counter")) marking_type = MADC32_MARKING_TYPE_EVENT_COUNTER;
    else if(!strcasecmp(sval, "time_stamp")) marking_type = MADC32_MARKING_TYPE_TIME_STAMP;
    else if(!strcasecmp(sval, "extended_time_stamp")) marking_type = MADC32_MARKING_TYPE_EXTENDED_TIME_STAMP;
    else{
      fprintf(stderr, "Invalid value for madc32_marking_type: %s\n", sval);
      marking_type = MADC32_MARKING_TYPE_EVENT_COUNTER;
    }

    sval = config_get_s_value("madc32_adc_resolution", i, "4khigh");
    if(!strcasecmp(sval, "2k")) adc_resolution = MADC32_ADC_RESOLUTION_2K_800NS;
    else if(!strcasecmp(sval, "4k")) adc_resolution = MADC32_ADC_RESOLUTION_4K_1600NS;
    else if(!strcasecmp(sval, "4khigh")) adc_resolution = MADC32_ADC_RESOLUTION_4K_HIGH_RES;
    else if(!strcasecmp(sval, "8k")) adc_resolution = MADC32_ADC_RESOLUTION_8K;
    else if(!strcasecmp(sval, "8khigh")) adc_resolution = MADC32_ADC_RESOLUTION_8K_HIGH_RES;
    else{
      fprintf(stderr, "Invalid value for madc32_adc_resolution: %s\n", sval);
      adc_resolution = MADC32_ADC_RESOLUTION_4K_HIGH_RES;
    }

    delay0         = config_get_l_value("madc32_delay0",         i, MADC32_DELAY0_25NS);
    delay1         = config_get_l_value("madc32_delay1",         i, MADC32_DELAY1_25NS);
    width0         = config_get_l_value("madc32_width0",         i, MADC32_WIDTH0_2US);
    width1         = config_get_l_value("madc32_width1",         i, MADC32_WIDTH1_2US);

    sval = config_get_s_value("madc32_use_gg", i, "gg0");
    if(!strcasecmp(sval, "gg0")) use_gg = MADC32_USE_GG_GG0;
    else if(!strcasecmp(sval, "gg1"))use_gg = MADC32_USE_GG_GG1; 
    else{
      fprintf(stderr, "Invalid value for madc32_use_gg: %s\n", sval);
      use_gg = MADC32_USE_GG_GG0;
    }
    sval = config_get_s_value("madc32_input_range", i, "10V");
    if(!strcasecmp(sval, "4V")) input_range = MADC32_INPUT_RANGE_4V;
    else if(!strcasecmp(sval, "8V"))input_range = MADC32_INPUT_RANGE_8V;
    else if(!strcasecmp(sval, "10V"))input_range = MADC32_INPUT_RANGE_10V;
    else{
      fprintf(stderr, "Invalid value for madc32_input_range: %s\n", sval);
      input_range = MADC32_INPUT_RANGE_10V;
    }
    sval = config_get_s_value("madc32_nim_gat1_osc", i, "oscillator");
    if(!strcasecmp(sval, "gate1")) nim_gat1_osc = MADC32_NIM_GAT1_OSC_GG1;
    else if(!strcasecmp(sval, "oscillator"))nim_gat1_osc = MADC32_NIM_GAT1_OSC_TIME;
    else{
      fprintf(stderr, "Invalid value for madc32_gat1_osc: %s\n", sval);
      nim_gat1_osc = MADC32_NIM_GAT1_OSC_TIME;
    }
    sval = config_get_s_value("madc32_nim_fc_reset", i, "fast_clear");
    if(!strcasecmp(sval, "fast_clear")) nim_fc_reset = MADC32_NIM_FC_RESET_FAST_CLEAR;
    else if(!strcasecmp(sval, "time_stamp"))nim_fc_reset = MADC32_NIM_FC_RESET_TIME_STAMP;
    else{
      fprintf(stderr, "Invalid value for madc32_nim_fc_reset: %s\n", sval);
      nim_fc_reset = MADC32_NIM_FC_RESET_FAST_CLEAR;
    }
    sval = config_get_s_value("madc32_nim_busy", i, "threshold");
    if(!strcasecmp(sval, "busy")) nim_busy = MADC32_NIM_BUSY_BUSY;
    else if(!strcasecmp(sval, "gg0")) nim_busy = MADC32_NIM_BUSY_GG0;
    else if(!strcasecmp(sval, "gg1")) nim_busy = MADC32_NIM_BUSY_GG1;
    else if(!strcasecmp(sval, "cbus")) nim_busy = MADC32_NIM_BUSY_CBUS;
    else if(!strcasecmp(sval, "full")) nim_busy = MADC32_NIM_BUSY_BUFFER_FULL;
    else if(!strcasecmp(sval, "threshold")) nim_busy = MADC32_NIM_BUSY_DATA_THRESHOLD;
    else{
      fprintf(stderr, "Invalid value for madc32_nim_busy: %s\n", sval);
      nim_fc_reset = MADC32_NIM_BUSY_DATA_THRESHOLD;
    }
    sval = config_get_s_value("madc32_ts_sources", i, "internal");
    if(!strcasecmp(sval, "internal")) ts_sources = MADC32_TS_SOURCES_VME;
    else if(!strcasecmp(sval, "external")) ts_sources = MADC32_TS_SOURCES_EXT;
    else{
      fprintf(stderr, "Invalid value for madc32_ts_sources: %s\n", sval);
      ts_sources = MADC32_TS_SOURCES_VME;
    }
    sval = config_get_s_value("madc32_ts_sources_ext_reset_enable", i, "disable");
    if(!strcasecmp(sval, "disable")) ;
    else if(!strcasecmp(sval, "enable")) ts_sources |= MADC32_TS_SOURCES_EXTERNAL_RESET;
    else{
      fprintf(stderr, "Invalid value for madc32_ts_sources_ext_reset_enable: %s\n", sval);
    }

    threshold      = config_get_l_value("madc32_threshold",      i, 0);
    
    fprintf(stderr, "Initializing module: %d\n", madc32_module_numbers[i]);
    fprintf(stderr, "  module id           = %6d\n", module_id);
    fprintf(stderr, "  irq_threshold       = %6d (%.4x) = %5d bytes\n", irq_threshold, irq_threshold,
	    irq_threshold * 4);
    fprintf(stderr, "  multi_event         = %6d (%.4x) = %s\n", multi_event, multi_event,
	    multi_event == MADC32_MULTI_EVENT_NO ? "none" :
	    multi_event == MADC32_MULTI_EVENT_YES_UNLIMITED ? "unlimited" :
	    multi_event == MADC32_MULTI_EVENT_YES_LIMITED ? "limited" :
	    "Undefined");
    fprintf(stderr, "  marking_type        = %6d (%.4x) = %s\n", marking_type, marking_type,
	    marking_type == MADC32_MARKING_TYPE_EVENT_COUNTER ? "event counter" :
	    marking_type == MADC32_MARKING_TYPE_TIME_STAMP ?    "time stamp" :
	    marking_type == MADC32_MARKING_TYPE_EXTENDED_TIME_STAMP ? "extended time stamp" :
	    "Undefined");
    fprintf(stderr, "  adc_resolution      = %6d (%.4x) = %s\n", adc_resolution, adc_resolution, 
	    adc_resolution == MADC32_ADC_RESOLUTION_2K_800NS    ? "2K 800ns" :
	    adc_resolution == MADC32_ADC_RESOLUTION_4K_1600NS   ? "4K 1.6us" :
	    adc_resolution == MADC32_ADC_RESOLUTION_4K_HIGH_RES ? "4K high res" :
	    adc_resolution == MADC32_ADC_RESOLUTION_8K          ? "8K" :
	    adc_resolution == MADC32_ADC_RESOLUTION_8K_HIGH_RES ? "8K high res" :
	    "Undefined");
    fprintf(stderr, "  delay0              = %6d (%.4x) = %5d nsec\n", delay0 , delay0,
	    madc32_decode_delay(delay0));
    fprintf(stderr, "  delay1              = %6d (%.4x) = %5d nsec\n", delay1 , delay1,
	    madc32_decode_delay(delay1));
    fprintf(stderr, "  width0              = %6d (%.4x) = %5d nsec\n", width0 , width0,
	    madc32_decode_width(width0));
    fprintf(stderr, "  width1              = %6d (%.4x) = %5d nsec\n", width1 , width1,
	    madc32_decode_width(width1));
    fprintf(stderr, "  use_gg              = %6d (%.4x) = %s\n", use_gg, use_gg,
	    use_gg == MADC32_USE_GG_GG0 ? "GG0" :
	    use_gg == MADC32_USE_GG_GG1 ? "GG1" :
	    "Undefined");
    fprintf(stderr, "  input_range         = %6d (%.4x) = %s\n", input_range, input_range,
	    input_range == MADC32_INPUT_RANGE_4V ? "4V" :
	    input_range == MADC32_INPUT_RANGE_8V ? "8V" :
	    input_range == MADC32_INPUT_RANGE_10V ? "10V" :
	    "Undefined");
    fprintf(stderr, "  nim_gat1_osc        = %6d (%.4x) = %s\n", nim_gat1_osc, nim_gat1_osc,
	    nim_gat1_osc == MADC32_NIM_GAT1_OSC_GG1 ?  "gate1" :
	    nim_gat1_osc == MADC32_NIM_GAT1_OSC_TIME ? "oscillator" :
	    "Undefined");
    fprintf(stderr, "  nim_fc_reset        = %6d (%.4x) = %s\n", nim_fc_reset, nim_fc_reset,
	    nim_fc_reset == MADC32_NIM_FC_RESET_FAST_CLEAR ?  "fast clear" :
	    nim_fc_reset == MADC32_NIM_FC_RESET_TIME_STAMP ?  "time stamp" :
	    "Undefined");
    fprintf(stderr, "  nim_busy            = %6d (%.4x) %s\n", nim_busy, nim_busy,
	    nim_busy == MADC32_NIM_BUSY_BUSY ?           "busy" :
	    nim_busy == MADC32_NIM_BUSY_GG0  ?           "GG0" :
	    nim_busy == MADC32_NIM_BUSY_GG1  ?           "GG1" :
	    nim_busy == MADC32_NIM_BUSY_CBUS ?           "cbus" :
	    nim_busy == MADC32_NIM_BUSY_BUFFER_FULL ?    "buffer full" :
	    nim_busy == MADC32_NIM_BUSY_DATA_THRESHOLD ? "above threshold" :
	    "Undefined");
    fprintf(stderr, "  ts_sources          = %6d (%.4x) %s\n", ts_sources, ts_sources,
	    ts_sources & MADC32_TS_SOURCES_EXT ? "external" : "internal");
    fprintf(stderr, "  ts_sources_ext_reset_enable =       %s\n",
	    ts_sources & MADC32_TS_SOURCES_EXTERNAL_RESET ? "enable" : "disable");
    fprintf(stderr, "  threshold           = %6d (%.4x)\n", threshold, threshold);

    /* ------------- initialize MADC32  ------------ */
    madc32[i]->module_id = module_id;
    madc32[i]->irq_threshold = irq_threshold;
    madc32[i]->data_len_format = MADC32_DATA_LEN_FORMAT_32BIT;
    madc32[i]->multi_event = multi_event;
    madc32[i]->marking_type = marking_type;  
    madc32[i]->bank_operation = MADC32_BANK_OPERATION_BANKS_CONNECTED;
    madc32[i]->adc_resolution = adc_resolution;
    madc32[i]->hold_delay0 = delay0;
    madc32[i]->hold_delay1 = delay1;
    madc32[i]->hold_width0 = width0;
    madc32[i]->hold_width1 = width1;
    madc32[i]->use_gg = use_gg;
    madc32[i]->input_range = input_range;
    madc32[i]->nim_gate1_osc = nim_gat1_osc;
    madc32[i]->nim_fc_reset = nim_fc_reset;
    madc32[i]->nim_busy = nim_busy;
    madc32[i]->ts_sources = ts_sources;
    
    for(ch=0; ch<MADC32_NUM_CHANNELS; ch++){
      madc32[i]->threshold[ch] = threshold;
    }
  }
}


void mqdc32_init(){
  int i, ch;
  char *sval;
  int module_id;
  int irq_threshold;
  int multi_event;
  int marking_type;
  int input_coupling;
  int nim_gat1_osc;
  int nim_fc_reset;
  int nim_busy;
  //  int threshold;
  int ts_sources;
  int exp_trig_delay;
  unsigned long use_ch;

  fprintf(stderr, "\n[MQDC32]\n");
  for(i=0; i<mqdc32_n_modules; i++){
    module_id      = config_get_l_value("mqdc32_module_id",      i, i);

    irq_threshold  = config_get_l_value("mqdc32_irq_threshold",  i, MQDC32_IRQ_THRESHOLD_DEFAULT);

    sval = config_get_s_value("mqdc32_multi_event",    i, "unlimited");
    if(!strcasecmp(sval, "none")) multi_event = MQDC32_MULTI_EVENT_NO;
    else if(!strcasecmp(sval, "unlimited")) multi_event = MQDC32_MULTI_EVENT_YES_UNLIMITED;
    else if(!strcasecmp(sval, "limited"))   multi_event = MQDC32_MULTI_EVENT_YES_LIMITED;
    else{
      fprintf(stderr, "Invalid value for mqdc32_multi_event: %s\n", sval);
      multi_event = MQDC32_MULTI_EVENT_YES_UNLIMITED;
    }

    sval = config_get_s_value("mqdc32_marking_type",   i, "event_counter");
    if(!strcasecmp(sval, "event_counter")) marking_type = MQDC32_MARKING_TYPE_EVENT_COUNTER;
    else if(!strcasecmp(sval, "time_stamp")) marking_type = MQDC32_MARKING_TYPE_TIME_STAMP;
    else if(!strcasecmp(sval, "extended_time_stamp")) marking_type = MQDC32_MARKING_TYPE_EXTENDED_TIME_STAMP;
    else{
      fprintf(stderr, "Invalid value for mqdc32_marking_type: %s\n", sval);
      marking_type = MQDC32_MARKING_TYPE_EVENT_COUNTER;
    }

    sval = config_get_s_value("mqdc32_input_coupling", i, "B0AC_B1AC");
    if(!strcasecmp(sval, "B0AC_B1AC")) input_coupling = MQDC32_INPUT_COUPLING_B0AC_B1AC;
    else if(!strcasecmp(sval, "B0DC_B1AC")) input_coupling = MQDC32_INPUT_COUPLING_B0DC_B1AC;
    else if(!strcasecmp(sval, "B0AC_B1DC")) input_coupling = MQDC32_INPUT_COUPLING_B0AC_B1DC;
    else if(!strcasecmp(sval, "B0DC_B1DC")) input_coupling = MQDC32_INPUT_COUPLING_B0DC_B1DC;
    else{
      fprintf(stderr, "Invalid value for mqdc32_input_coupling: %s\n", sval);
      input_coupling = MQDC32_INPUT_COUPLING_B0AC_B1AC;
    }

    sval = config_get_s_value("mqdc32_nim_gat1_osc", i, "oscillator");
    if(!strcasecmp(sval, "gate1")) nim_gat1_osc = MQDC32_NIM_GAT1_OSC_GATE1_INP;
    else if(!strcasecmp(sval, "oscillator")) nim_gat1_osc = MQDC32_NIM_GAT1_OSC_TIME;
    else{
      fprintf(stderr, "Invalid value for mqdc32_gat1_osc: %s\n", sval);
      nim_gat1_osc = MQDC32_NIM_GAT1_OSC_TIME;
    }

    sval = config_get_s_value("mqdc32_nim_fc_reset", i, "fast_clear");
    if(!strcasecmp(sval, "fast_clear")) nim_fc_reset = MQDC32_NIM_FC_RESET_FAST_CLEAR;
    else if(!strcasecmp(sval, "time_stamp")) nim_fc_reset = MQDC32_NIM_FC_RESET_TIME_STAMP;
    else if(!strcasecmp(sval, "exp_trig")) nim_fc_reset = MQDC32_NIM_FC_RESET_EXP_TRIG;
    else{
      fprintf(stderr, "Invalid value for mqdc32_nim_fc_reset: %s\n", sval);
      nim_fc_reset = MQDC32_NIM_FC_RESET_FAST_CLEAR;
    }

    sval = config_get_s_value("mqdc32_nim_busy", i, "data_threshold");
    if(!strcasecmp(sval, "busy")) nim_busy = MQDC32_NIM_BUSY_BUSY;
    else if(!strcasecmp(sval, "cbus")) nim_busy = MQDC32_NIM_BUSY_CBUS;
    else if(!strcasecmp(sval, "full")) nim_busy = MQDC32_NIM_BUSY_BUFFER_FULL;
    else if(!strcasecmp(sval, "data_threshold")) nim_busy = MQDC32_NIM_BUSY_DATA_THRESHOLD;
    else if(!strcasecmp(sval, "evt_threshold"))  nim_busy = MQDC32_NIM_BUSY_EVT_THRESHOLD;
    else{
      fprintf(stderr, "Invalid value for mqdc32_nim_busy: %s\n", sval);
      nim_fc_reset = MQDC32_NIM_BUSY_DATA_THRESHOLD;
    }

    sval = config_get_s_value("mqdc32_ts_sources", i, "external");
    if(!strcasecmp(sval, "internal")) ts_sources = MQDC32_TS_SOURCES_VME;
    else if(!strcasecmp(sval, "external")) ts_sources = MQDC32_TS_SOURCES_EXT;
    else{
      fprintf(stderr, "Invalid value for mqdc32_ts_sources: %s\n", sval);
      ts_sources = MQDC32_TS_SOURCES_VME;
    }

    sval = config_get_s_value("mqdc32_ts_sources_ext_reset_enable", i, "disable");
    if(!strcasecmp(sval, "disable")) ;
    else if(!strcasecmp(sval, "enable")) ts_sources |= MQDC32_TS_SOURCES_EXTERNAL_RESET;
    else{
      fprintf(stderr, "Invalid value for mqdc32_ts_sources_ext_reset_enable: %s\n", sval);
    }

    //    threshold      = config_get_l_value("mqdc32_threshold",      i, 0);
    
    use_ch = config_get_l_value("mqdc32_use_ch",      i, 0xffffffff);
    for(ch=0; ch<MQDC32_NUM_CHANNELS; ch++){
      if((use_ch>>ch)&0x00000001 == 1) fprintf(stderr, "MQDC32 ch%02d: Enable\n", ch);
      else fprintf(stderr, "MQDC32 ch%02d: Disable\n", ch);
    }

    exp_trig_delay = config_get_l_value("mqdc32_exp_trig_delay", i, 0);

    
    fprintf(stderr, "Initializing module: %d\n", mqdc32_module_numbers[i]);
    fprintf(stderr, "  module id           = %6d\n", module_id);
    fprintf(stderr, "  irq_threshold       = %6d (%.4x) = %5d bytes\n", irq_threshold, irq_threshold,
	    irq_threshold * 4);
    fprintf(stderr, "  multi_event         = %6d (%.4x) = %s\n", multi_event, multi_event,
	    multi_event == MQDC32_MULTI_EVENT_NO ? "none" :
	    multi_event == MQDC32_MULTI_EVENT_YES_UNLIMITED ? "unlimited" :
	    multi_event == MQDC32_MULTI_EVENT_YES_LIMITED ? "limited" :
	    "Undefined");
    fprintf(stderr, "  marking_type        = %6d (%.4x) = %s\n", marking_type, marking_type,
	    marking_type == MQDC32_MARKING_TYPE_EVENT_COUNTER ? "event counter" :
	    marking_type == MQDC32_MARKING_TYPE_TIME_STAMP ?    "time stamp" :
	    marking_type == MQDC32_MARKING_TYPE_EXTENDED_TIME_STAMP ? "extended time stamp" :
	    "Undefined");
    fprintf(stderr, "  input_coupling      = %6d (%.4x) = %s\n", input_coupling, input_coupling,
	    input_coupling == MQDC32_INPUT_COUPLING_B0AC_B1AC ? "bank0:AC, bank1:AC" :
	    input_coupling == MQDC32_INPUT_COUPLING_B0DC_B1AC ? "bank0:DC, bank1:AC" :
	    input_coupling == MQDC32_INPUT_COUPLING_B0AC_B1DC ? "bank0:AC, bank1:DC" :
	    input_coupling == MQDC32_INPUT_COUPLING_B0DC_B1DC ? "bank0:DC, bank1:DC" :
	    "Undefined");
    fprintf(stderr, "  nim_gat1_osc        = %6d (%.4x) = %s\n", nim_gat1_osc, nim_gat1_osc,
	    nim_gat1_osc == MQDC32_NIM_GAT1_OSC_GATE1_INP ?  "gate1" :
	    nim_gat1_osc == MQDC32_NIM_GAT1_OSC_TIME ? "oscillator" :
	    "Undefined");
    fprintf(stderr, "  nim_fc_reset        = %6d (%.4x) = %s\n", nim_fc_reset, nim_fc_reset,
	    nim_fc_reset == MQDC32_NIM_FC_RESET_FAST_CLEAR ?  "fast clear" :
	    nim_fc_reset == MQDC32_NIM_FC_RESET_TIME_STAMP ?  "time stamp" :
	    "Undefined");
    fprintf(stderr, "  nim_busy            = %6d (%.4x) %s\n", nim_busy, nim_busy,
	    nim_busy == MQDC32_NIM_BUSY_BUSY ?           "busy" :
	    nim_busy == MQDC32_NIM_BUSY_CBUS ?           "cbus" :
	    nim_busy == MQDC32_NIM_BUSY_BUFFER_FULL ?    "buffer full" :
	    nim_busy == MQDC32_NIM_BUSY_DATA_THRESHOLD ? "data above threshold" :
	    nim_busy == MQDC32_NIM_BUSY_EVT_THRESHOLD ?  "event above threshold" :
	    "Undefined");
    fprintf(stderr, "  ts_sources          = %6d (%.4x) %s\n", ts_sources, ts_sources,
	    ts_sources & MQDC32_TS_SOURCES_EXT ? "external" : "internal");
    fprintf(stderr, "  ts_sources_ext_reset_enable =       %s\n",
	    ts_sources & MQDC32_TS_SOURCES_EXTERNAL_RESET ? "enable" : "disable");
    //    fprintf(stderr, "  threshold           = %6d (%.4x)\n", threshold, threshold);
    fprintf(stderr, "  experiment trigger delay           = %6d (%.4x)\n", exp_trig_delay, exp_trig_delay);

    /* ------------- initialize MQDC32  ------------ */
    mqdc32[i]->module_id = module_id;
    mqdc32[i]->irq_threshold = irq_threshold;
    mqdc32[i]->data_len_format = MQDC32_DATA_LEN_FORMAT_32BIT;
    mqdc32[i]->multi_event = multi_event;
    mqdc32[i]->marking_type = marking_type;  
    mqdc32[i]->bank_operation = MQDC32_BANK_OPERATION_BANKS_CONNECTED;
    mqdc32[i]->input_coupling = input_coupling;
    mqdc32[i]->nim_gate1_osc = nim_gat1_osc;
    mqdc32[i]->nim_fc_reset = nim_fc_reset;
    mqdc32[i]->nim_busy = nim_busy;
    mqdc32[i]->ts_sources = ts_sources;
    
    for(ch=0; ch<MQDC32_NUM_CHANNELS; ch++){
      if((use_ch>>ch)&0x00000001 == 1) mqdc32[i]->threshold[ch] = 0;      // enable
      else mqdc32[i]->threshold[ch] = 0x1fff; // disable
    }
    // 20200910 for DAQ speed test
    for(ch=0; ch<4; ch++){
      //      mqdc32[i]->threshold[ch] = 0;
    }

    mqdc32[i]->exp_trig_delay0 = exp_trig_delay;
  }
}

int main(int argc, char *argv[]){
  FILE *fd = stdout;
  int i;
  int init_flag = 0;
  int net_flag = 0;
  int ld=0;
  int sd=0;
  int n=0;
  long l=0;

  char *program_name;
  char config_file_name[256];
  char homedir[256];
  char hostname[256];
  char hostname_short[256];

#if 0 // busy test
  almost_full_level = 100;
#endif

  program_name = *argv++;
  argc--;

  // default config file name
  strncpy(homedir, getenv("HOME"), 256);
  gethostname(hostname, 256);
  sscanf(hostname, "%256[^.]", hostname_short);
  sprintf(config_file_name, "%s/conf/%s.conf", homedir, hostname_short);

  while(argc>0){
    if(!strcmp(argv[0],"-i")){
      init_flag = 1;
    }else if(!strcmp(argv[0],"-h")){
      usage(program_name);
      exit(0);
    }else if(!strcmp(argv[0],"-d")){
      debug_mode = 1;
    }else if(!strcmp(argv[0],"-n")){
      net_flag = 1;
    }else if(!strcmp(argv[0],"-dma")){
      use_dma_transfer = 1;
    }else if(!strcmp(argv[0],"-o")){
      if(argc<=1){
	fprintf(stderr, "no argument for %s\n", argv[0]);
	usage(program_name);
	exit(-1);
      }
      fd = fopen(argv[1],"w");
      if(fd==(FILE*)NULL){
        fprintf(stderr, "Could not open file '%s'.\n", argv[1]);
        exit(-1);
      }
      argc--;
      argv++;
    }else if(!strcmp(argv[0],"-c")){
      if(argc<=1){
	fprintf(stderr, "no argument for %s\n", argv[0]);
	usage(program_name);
	exit(-1);
      }
      strncpy(config_file_name, argv[1], 256);
      argc--;
      argv++;
    }else if(argv[0][0]=='-'){
      fprintf(stderr, "Unknown option %s.\n", argv[0]);
      exit(-1);
    }else{
      fprintf(stderr, "Undefined option or unexpected arguments %s...\n", argv[0]);
      usage(0);
      exit(-1);
    }
    argc--;
    argv++;
  }

  /*-------------- Read configuration file ----------*/


  fprintf(stderr, "Reading configration file '%s'...\n", config_file_name);
  config_set_debug_mode(debug_mode);
  config_read_file(config_file_name);
  fprintf(stderr, "Done\n");

  /* ------------- Open ------------ */

  /*** --- DMA --- ***/
  if(use_dma_transfer){
    fprintf(stderr, "\n[DMA]\n");
    fprintf(stderr, "Initialize DMA handle.\n");
    if(dma_init()){
      fprintf(stderr, "Error in dma_init()\n");
      exit(-1);
    }
  }

  /*** --- V977 --- ***/
  fprintf(stderr, "\n[V977]\n");
  if(v977_init()){
    fprintf(stderr, "Error in v977_init()\n");
    exit(-1);
  }

  /*** --- V830 --- ***/
  fprintf(stderr, "\n[830]\n");
  v830_n_scalers = config_get_l_value("v830_n_scalers", 0, 3);
  if(v830_n_scalers<0 || V830_N_CHANNELS<v830_n_scalers){
    fprintf(stderr, "Invalid number for v830_n_scalers: %d.\n", v830_n_scalers);
    exit(-1);
  }
  fprintf(stderr, "Number of scalers to read = %d\n", v830_n_scalers);
  if(v830_init()){
    fprintf(stderr, "Error in v830_init()\n");
    exit(-1);
  }

  /*** --- V1190A --- ***/
  fprintf(stderr, "\n[V1190]\n");
  if(v1190_open()){
    fprintf(stderr, "Error in v1190_open()\n");
    exit(-1);
  }

  v1190_n_modules = config_get_l_value("v1190_n_modules", 0, 0);
  if(v1190_n_modules<0 || V1190_MAX_N_MODULES<v1190_n_modules){
    fprintf(stderr, "Invalid number for v1190_n_modules: %d.\n", v1190_n_modules);
    exit(-1);
  }
  fprintf(stderr, "Number of V1190 modules = %d\n", v1190_n_modules);
  for(i=0; i<v1190_n_modules; i++){
    v1190_module_numbers[i] = config_get_l_value("v1190_module", i, i);
    fprintf(stderr, "V1190 module number: %d\n", v1190_module_numbers[i]);
  }

  for(i=0; i<v1190_n_modules; i++){
    fprintf(stderr, "Map V1190A #%d\n", v1190_module_numbers[i]);
    v1190[i] = v1190_map(v1190_module_numbers[i]);
    if(v1190[i]==(V1190_p)NULL){
      fprintf(stderr, "Error in v1190_map()\n");
      v1190_close();
      exit(-1);
    }
  }

  /*** --- MADC32 --- ***/
  fprintf(stderr, "\n[MADC32]\n");
  if(madc32_open()){
    fprintf(stderr, "Error in madc32_open()\n");
    exit(-1);
  }

  madc32_n_modules = config_get_l_value("madc32_n_modules", 0, 0);
  if(madc32_n_modules<0 || MADC32_MAX_N_MODULES<madc32_n_modules){
    fprintf(stderr, "Invalid number for madc32_n_modules: %d.\n", madc32_n_modules);
    exit(-1);
  }
  fprintf(stderr, "Number of MADC32 modules = %d\n", madc32_n_modules);
  for(i=0; i<madc32_n_modules; i++){
    madc32_module_numbers[i] = config_get_l_value("madc32_module", i, i);
    fprintf(stderr, "MADC32 module number: %d\n", madc32_module_numbers[i]);
  }

  for(i=0; i<madc32_n_modules; i++){
    fprintf(stderr, "Map MADC32 #%d\n", madc32_module_numbers[i]);
    madc32[i] = madc32_map(madc32_module_numbers[i]);
    madc32_module_id[i] = madc32[i]->module_id;
    if(madc32[i]==(MADC32_p)NULL){
      fprintf(stderr, "Error in madc32_map()\n");
      madc32_close();
      exit(-1);
    }
  }

  /*** --- MQDC32 --- ***/
  fprintf(stderr, "\n[MQDC32]\n");
  if(mqdc32_open()){
    fprintf(stderr, "Error in mqdc32_open()\n");
    exit(-1);
  }

  mqdc32_n_modules = config_get_l_value("mqdc32_n_modules", 0, 0);
  if(mqdc32_n_modules<0 || MQDC32_MAX_N_MODULES<mqdc32_n_modules){
    fprintf(stderr, "Invalid number for mqdc32_n_modules: %d.\n", mqdc32_n_modules);
    exit(-1);
  }
  fprintf(stderr, "Number of MQDC32 modules = %d\n", mqdc32_n_modules);
  for(i=0; i<mqdc32_n_modules; i++){
    mqdc32_module_numbers[i] = config_get_l_value("mqdc32_module", i, i);
    fprintf(stderr, "MQDC32 module number: %d\n", mqdc32_module_numbers[i]);
  }

  for(i=0; i<mqdc32_n_modules; i++){
    fprintf(stderr, "Map MQDC32 #%d\n", mqdc32_module_numbers[i]);
    mqdc32[i] = mqdc32_map(mqdc32_module_numbers[i]);
    mqdc32_module_id[i] = mqdc32[i]->module_id;
    if(mqdc32[i]==(MQDC32_p)NULL){
      fprintf(stderr, "Error in mqdc32_map()\n");
      mqdc32_close();
      exit(-1);
    }
  }

  /*** --- MyRIAD --- ***/
  fprintf(stderr, "\n[MyRIAD]\n");
  myriad_n_modules = config_get_l_value("myriad_n_modules", 0, 0);
  if(myriad_n_modules!=0 && myriad_n_modules!=1){
    fprintf(stderr, "Invalid number for myriad_n_modules: %d.\n", myriad_n_modules);
    exit(-1);
  }
  fprintf(stderr, "Number of MyRIAD = %d\n", myriad_n_modules);

  /* ------------- Initialization ------------ */
  if(init_flag){
    fprintf(stderr, "\n---- Initialization of the Modules ----\n");
    v1190_init();
    madc32_init();
    mqdc32_init();

    fprintf(stderr, "------------------- Done ----------------\n");
    exit(0);
  }

  for(i=0; i<madc32_n_modules; i++){
    madc32[i]->start_acq = 0;
    madc32[i]->fifo_reset = 1;  /* FIFO Reset */
    madc32[i]->start_acq = 1;
  }

  for(i=0; i<mqdc32_n_modules; i++){
    mqdc32[i]->start_acq = 0;
    mqdc32[i]->fifo_reset = 1;  /* FIFO Reset */
    mqdc32[i]->start_acq = 1;
  }
  
  if(myriad_n_modules>0){
    if(myriad_init()){
      fprintf(stderr, "Error in myriad_init()\n");
      exit(-1);
    }
  }

  clear_event_counters();
  
  if(net_flag){
    fprintf(stderr, "Creating a socket.\n");
    ld = get_socket();
  }
  
  signal(SIGPIPE, SIG_IGN);   /* ignore the signal SIGPIPE */
  signal(SIGHUP, sighup);   /* ignore the signal SIGPIPE */

  /* ------------- Start DAQ  ------------ */
  fprintf(stderr, "\nDAQ started.\n");
  if(net_flag){
    while(1){
      periodic_clear();
      if(sd==0){
	sd = accept_connection(ld);
	reset_event_counters();
	v977->input_set = 0;
	v977->output_set = 0;
      }
      n=read_data(sd, fd);
      if(n>0) continue;
      else if(n==0){ // idling
	if(write_to_sd(sd, &l, 0)>=0){ // send keep alive packet
	  show_counter();
	  //	  usleep(10000);
	  continue;
	}
      }
      // connection loss
      v977->output_set = 1<<IOREG_CH_DAQ_INHIBIT;
      fprintf(stderr, "Close connection to the client.\n");
      close(sd);
      sd = 0;
    }
  }else{
    v977->input_set = 0;
    v977->output_set = 0;
    while(1){
      periodic_clear();
      n= read_data(sd, fd);
      if(n<0){
	break;
      }
    }
    v977->output_set = 1<<IOREG_CH_DAQ_INHIBIT;
  }

  /* ------------- Close ------------ */
  for(i=0; i<v1190_n_modules; i++){
    if(v1190_unmap(v1190_module_numbers[i])){
      fprintf(stderr, "Error in v1190_unmap()\n");
    }
  }

  if(v1190_close()){
    fprintf(stderr, "Error in v1190_close()\n");
  }

  for(i=0; i<madc32_n_modules; i++){
    if(madc32_unmap(madc32_module_numbers[i])){
      fprintf(stderr, "Error in madc32_unmap()\n");
    }
  }

  if(madc32_close()){
    fprintf(stderr, "Error in madc32_close()\n");
  }

  for(i=0; i<mqdc32_n_modules; i++){
    if(mqdc32_unmap(mqdc32_module_numbers[i])){
      fprintf(stderr, "Error in mqdc32_unmap()\n");
    }
  }

  if(mqdc32_close()){
    fprintf(stderr, "Error in mqdc32_close()\n");
  }

  if(v830_exit()){
    fprintf(stderr, "Error in v830_exit()\n");
  }

  if(v977_exit()){
    fprintf(stderr, "Error in v977_exit()\n");
  }

  if(myriad_n_modules>0){
    if(myriad_unmap()){
      fprintf(stderr, "Error in myriad_unmap()\n");
    }
    if(myriad_exit()){
      fprintf(stderr, "Error in myriad_exit()\n");
    }
  }

  return 0; 
}

