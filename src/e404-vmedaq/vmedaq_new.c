/* vmedaq.c ---- vme DAQ with V1190                                         */
/*								            */
/*  Version 1.00        2013-09-05      by A. Tamii (For Linux2.6)GE-FANUC) */
/*  Version 2.00        2016-04-12      by A. Tamii (Use a config file)     */

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
#include "v977.h"
#include "v830.h"
#include "config.h"

#define DEBUG 0   /* 1 = debug mode */

#define PERIODIC_CLEAR                    1  /* periodic clear of the V1190 modules */
//#define PERIODIC_CLEAR                    0  /* periodic clear of the V1190 modules */
#define PERIODIC_CLEAR_INTERVAL_IN_SEC   10  /* periodic clear, period in seconds */

long vmedaq_block_counter = 0;

int geometrical_address = -1;

#define MAX_DUMP_BUF 0x1000
unsigned long dump_buffer[MAX_DUMP_BUF];

#define V977_MODULE_NUMBER 0
static V977_p v977=(V977_p)NULL;

#define V830_MODULE_NUMBER 0
#define v830_n_channels  32
static V830_p v830=(V830_p)NULL;
static long v830_scalers[v830_n_channels];

#define IOREG_CH_DAQ_INHIBIT     0
#define IOREG_CH_ALMOST_FULL     1
#define IOREG_CH_BLOCK_TRANSFER  2

#define CLEAR_V1190_AFTER_ALMOST_FULL 1
unsigned int counter_offset=0;

long long data_size = 0;
int blk_count = 0;
int v1190_n_modules = 0;
int v1190_module_numbers[V1190_MAX_N_MODULES];
int v1190_module_geo[V1190_MAX_N_MODULES];
int v1190_module_counters[32];

#define MAX_N_MADC32_MODULE_COUNTERS 0x10000
int madc32_n_modules = 0;
int madc32_module_numbers[MADC32_MAX_N_MODULES];
int madc32_module_id[MADC32_MAX_N_MODULES];
int madc32_module_counters[MAX_N_MADC32_MODULE_COUNTERS];
int madc32_module_geo[MADC32_MAX_N_MODULES];
V1190_p v1190[V1190_MAX_N_MODULES];
MADC32_p madc32[MADC32_MAX_N_MODULES];


static int v977_init(){
  if(v977_open()){
    fprintf(stderr, "Error in v977_open()\n");
    return -1;
  }
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
  v977->output_mask = 0;
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
  for(i=0; i<v830_n_channels; i++){
    v830_scalers[i] = 0;
  }
}
  
static int v830_init(){
  if(v830_open()){
    fprintf(stderr, "Error in v830_open()\n");
    return -1;
  }
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
  v830->channelEnableRegister = 0x00000007;
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

void clear_event_counters(){
  int i;
  for(i=0; i<v1190_n_modules; i++){
    v1190[i]->software_clear = 1;   /* software clear, clear the event counter */
  }
  for(i=0; i<madc32_n_modules; i++){
    madc32[i]->reset_ctr_ab = 1;   /* clear the event counter */
  }
}

void reset_event_counters(){
  fprintf(stderr, "\nReset the event counters.\n");
  clear_event_counters();
  data_size = 0;
  v830_clear();
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
  long counter[v830_n_channels];
  VMEDAQ_HEADER_p vmedaq_header;

  vmedaq_header = (VMEDAQ_HEADER_p)buffer;
  *size = sizeof(VMEDAQ_HEADER_t)/sizeof(long);
  memory_buffer = &v830->meb[0];
  for(i=0; i<v830_n_channels; i++){
    counter[i] = 0;
  }

  n = v830->MEBEventNumber;

  if(n<=0){
    *size = 0;
    return -1;
  }
  for(i=0; i<n; i++){
    if(buf_size-(*size) < (1+v830_n_channels)) break;
    ldata = *memory_buffer;
    data = *(V830_MEB_HEADER_t*)&ldata;
    for(j=0; j<v830_n_channels && data.header==0; j++){
      ldata = *memory_buffer;
      data = *(V830_MEB_HEADER_t*)&ldata;
    }
    if(data.header==0) break;
    n_words = data.n_words;
    for(j=0; j<n_words; j++){
      counter[j] = *memory_buffer;
    }
    event_count++;

    v830_scalers[0] = counter[0];   // accepted trigger
    v830_scalers[1] = counter[1];   // buffer change
    v830_scalers[2] = counter[2];   // accepted clock
    // fprintf(stderr, "\n\n V830 scalers = %12ld %12ld %12ld\n\n", v830_scalers[0],  v830_scalers[1],  v830_scalers[2]);
    buffer[(*size)++] = 3;
    buffer[(*size)++] = v830_scalers[0];
    buffer[(*size)++] = v830_scalers[1];
    buffer[(*size)++] = v830_scalers[2];
  }
  //fprintf(stderr, "\nn=%d event_count=%ld, size=%d\n", n, event_count, *size);

  vmedaq_write_header(vmedaq_header, VMEDAQ_TYPE_V830, (*size)*sizeof(long));

  return event_count;
}

long v1190_check_data(V1190_p v1190, unsigned long *buffer, int buf_size, int *size){
  V1190_DATA_t data;
  unsigned long *output_buffer;
  unsigned long ldata;
  long event_count = -1;
  int end_flag = 0;
  int n_event_stored = 0;
  VMEDAQ_HEADER_p vmedaq_header;
  
  vmedaq_header = (VMEDAQ_HEADER_p)buffer;
  *size = sizeof(VMEDAQ_HEADER_t)/sizeof(long);
  output_buffer = v1190->output_buffer;

  n_event_stored = v1190->event_stored;

  while(!end_flag){ 
    ldata = *output_buffer;
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
	// manually create an end of event data
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
	  header->subheader = MADC32_SUBHEADER_NO_END_OF_EVENT;  /* for the artificially generated end of event */
	}
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
	header->subheader = MADC32_SUBHEADER_NO_HEADER;  /* for the artificially generated header */
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
void send_counter_offset(int sd){
  unsigned int buffer[(VMEDAQ_COUNTER_OFFSET_SIZE)/sizeof(unsigned int)];
  VMEDAQ_HEADER_p header;
  unsigned int *offset;
  header = (VMEDAQ_HEADER_p)buffer;
  offset = (unsigned int *)&header[1];

  counter_offset = *offset = v830_scalers[0];
  if(sd){
    vmedaq_write_header(header, VMEDAQ_TYPE_COUNTER_OFFSET, VMEDAQ_COUNTER_OFFSET_SIZE);
    write_to_sd(sd, (unsigned long*)buffer, VMEDAQ_COUNTER_OFFSET_SIZE/sizeof(unsigned long));
  }
}

#define DATA_BUF_SIZE 16384*16
static unsigned long data_buffer[DATA_BUF_SIZE];

int read_data(int sd, FILE *fd){
  int n_data_modules;
  long event_count;
  int i;
  int v977_val;
  int size;
  int sent_size;
  int written_size;

  n_data_modules = 0;

  v977_val = v977->singlehit_read;

  event_count = v830_check_data(v830, data_buffer, DATA_BUF_SIZE, &size);
  if(sd){
    if(size>0){
      sent_size = write_to_sd(sd, data_buffer, size);
      if(sent_size!=size) return -1;
    }
  }else{
    if(size>0){
      written_size = fwrite(data_buffer, sizeof(unsigned long), size, fd);
      if(written_size != size){
	perror("write");
	return -1;
      }
      fflush(fd);
    }
  }
  data_size += size*sizeof(unsigned long);
  
  if(event_count>=0){
    n_data_modules++;
  }

  for(i=0; i<v1190_n_modules; i++){
    event_count = v1190_check_data(v1190[i], data_buffer, DATA_BUF_SIZE, &size);
    if(sd){
      if(size>0){
	sent_size = write_to_sd(sd, data_buffer, size);
	if(sent_size!=size) return -1;
      }
    }else{
      if(size>0){
	written_size = fwrite(data_buffer, sizeof(unsigned long), size, fd);
	if(written_size != size){
	  perror("write");
	  return -1;
	}
	fflush(fd);
      }
    }
    data_size += size*sizeof(unsigned long);

    if(event_count>=0){
      n_data_modules++;
    }
  }

  for(i=0; i<madc32_n_modules; i++){
    event_count = madc32_check_data(madc32[i], madc32_module_id[i], data_buffer, DATA_BUF_SIZE, &size);
    if(sd){
      if(size>0){
	sent_size = write_to_sd(sd, data_buffer, size);
	if(sent_size!=size) return -1;
      }
    }else{
      if(size>0){
	written_size = fwrite(data_buffer, sizeof(unsigned long), size, fd);
	if(written_size != size){
	  perror("write");
	  return -1;
	}
	fflush(fd);
      }
    }
    data_size += size*sizeof(unsigned long);

    if(event_count>=0){
      n_data_modules++;
    }
  }

  //fprintf(stderr, "n_data_modules = %d\n", n_data_modules);
  if(n_data_modules==0){
    if(v977_val&(1<<IOREG_CH_ALMOST_FULL)){
#if CLEAR_V1190_AFTER_ALMOST_FULL
      send_counter_offset(sd);
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

void usage(){
  fprintf(stdout, "%s ... read v1190 TDC and output to stdout.\n", argv[0]);
  fprintf(stdout, "Usage: %s [-nih] [-c config_file_name] [file_name] \n", argv[0]);
  fprintf(stdout, " -n: (network option) send data to a client via tcp/ip.\n");
  fprintf(stdout, " -h: show this help.\n");
  fprintf(stdout, " -i: initialize V1190 TDCs\n");
  fprintf(stdout, " file_name: file to write when no -n option\n");
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
  int v1190_flag = 1;
  int madc32_flag = 0;
  int geo_flag=0;

  char *program_name;

  int t_clock = 25;
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

  

#if 0 // busy test
  almost_full_level = 100;
#endif

  program_name = argv++;
  argc--;

  while(argc>0){
    if(!strcmp(argv[0],"-i")){
      init_flag = 1;
    }else if(!strcmp(argv[0],"-i")){
      usage();
      exit(0);
    }else if(!strcmp(argv[0],"-n")){
      net_flag = 1;
#if 0
    }else if(!strcmp(argv[0],"-tl")){
      detection = V1190_OPECODE_DETECTION_TRAILING_AND_LEADING; // trailing and leading
      fprintf(stderr, "Edge detection mode = trainling and leading\n");
#endif
    }else if(!strcmp(argv[0],"-c")){
      if(argc<=1){
	fprintf(stderr, "no argument for %s\n", argv[0]);
	usage();
	exit(-1);
      }
      fprintf(stderr, "Reading configration file '%s'...\n", argv[1]);
      config_read_file(argv[1]);
      fprintf(stderr, "Done\n");
    }else if(argv[0][0]=='-'){
      fprintf(stderr, "Unknown option %s.\n", argv[0]);
      exit(-1);
    }else{
      fd = fopen(argv[0],"w");
      if(fd==(FILE*)NULL){
        fprintf(stderr, "Could not open file '%s'.\n", argv[0]);
        exit(-1);
      }
      break;
    }
    argc--;
    argv++;
  }

  /* ------------- Open ------------ */

  /*** --- V977 --- ***/
  if(v977_init()){
    fprintf(stderr, "Error in v1190_init()\n");
    exit(-1);
  }

  /*** --- V830 --- ***/

  if(v830_init()){
    fprintf(stderr, "Error in v830_init()\n");
    exit(-1);
  }

  /*** --- V1190A --- ***/
  if(v1190_open()){
    fprintf(stderr, "Error in v1190_open()\n");
    exit(-1);
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
  if(madc32_open()){
    fprintf(stderr, "Error in madc32_open()\n");
    exit(-1);
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

  /* ------------- Initialization ------------ */
	for(i=0; i<v1190_n_modules; i++){
	  if(init_flag){
	    if(i==0){
	      fprintf(stderr, "Initialization Parameters\n");
	      fprintf(stderr, "  Match window width        = %6d (0x%4x) \n", match_window_width, match_window_width);
	      fprintf(stderr, "  Window offset             = %6d (0x%4x) \n", window_offset, window_offset);
	      fprintf(stderr, "  Extra search window width = %6d (0x%4x) \n", extra_search_window_width, extra_search_window_width);
	      fprintf(stderr, "  Reject margin             = %6d (0x%4x) \n", reject_margin, reject_margin);
	      fprintf(stderr, "  Detection                 = %6d (0x%4x) \n", detection, detection);
	      fprintf(stderr, "  Almost full level         = %6d (0x%4x) \n", almost_full_level, almost_full_level);
	      fprintf(stderr, "  Maximum number of hits    = %6d (=%s) \n", max_number_hits_w, max_number_hits_s[max_number_hits_w&0x0F]);
	      fprintf(stderr, "\n");
	    }
	    
	    switch(i){
	    case 8:
	      v1190_module_numbers[i]=24;
	      break;
	    }
	    fprintf(stderr, "Initializing module: %d\n", v1190_module_numbers[i]);
	    fprintf(stderr, "v1190 = %x\n", (size_t)v1190[i]);
	    v1190[i]->output_prog_control = 2; /* select Almost Full for output */
	    v1190[i]->almost_full_level = almost_full_level;
	    v1190[i]->geo_address_register = v1190_module_geo[i];
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
	    fprintf(stderr, "done\n");
	}
	}
	
	if(init_flag)
	  exit(0);
	
	for(i=0; i<madc32_n_modules; i++){
	  madc32[i]->start_acq = 0;
	  madc32[i]->fifo_reset = 1;  /* FIFO Reset */
	  madc32[i]->start_acq = 1;
	}
	
	clear_event_counters();
	
	if(net_flag){
	  fprintf(stderr, "Creating a socket.\n");
	  ld = get_socket();
	}
	
	signal(SIGPIPE, SIG_IGN);   /* ignore the signal SIGPIPE */
	signal(SIGHUP, sighup);   /* ignore the signal SIGPIPE */

  /* ------------- Start DAQ  ------------ */
  fprintf(stderr, "DAQ started.\n");
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
	  usleep(10000);
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
      if(read_data(sd, fd)<0){
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

  if(v830_exit()){
    fprintf(stderr, "Error in v830_exit()\n");
  }

  if(v977_exit()){
    fprintf(stderr, "Error in v977_exit()\n");
  }

	return 0; 
}

