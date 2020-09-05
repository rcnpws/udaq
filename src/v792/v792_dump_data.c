/* v792test.c ---- access test to V792                                  */
/*								           */
/*  Version 1.00        2013-07-31      by oiwa (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v792.h"

int main(){
	int ret,i;
	unsigned long *buff,ldata;
	V792_DATA_t data;
	int module_number = 0;

	V792_p v792=(V792_p)NULL;

	fprintf(stderr, "open V792\n");
	ret = v792_open();
	if(ret!=0){
		fprintf(stderr, "Error in v792_open()\n");
		exit(-1);
	}

	fprintf(stderr, "map V792(module_number=%d)\n", module_number);
	v792 = v792_map(module_number);
	if(v792==(V792_p)NULL){
		fprintf(stderr, "Error in v792_map()\n");
		exit(-1);
	}

	buff = v792->output_buffer;
	int endflag = 0;
	for(i=0; !endflag;i++){
		ldata = *buff;
		printf("output buffer  = 0x%.8lx\n", ldata);

		*(unsigned long *)&data = ldata;
		switch(data.header.id){


			case V792_HEADER_ID_H:
				printf("---------------------------------------\n");
				printf(" Header \n");
				printf(" Converted channels: %d \n",data.header.converted_ch);
				printf(" Crate number      : %d \n",data.header.crate_num);
				printf(" geo               : %d \n",data.header.geo);
				printf("---------------------------------------\n");
				break;
			case V792_HEADER_ID_DW:
				printf(" QDC Data \n");
				printf(" Channel           : %d \n",data.data_word.channel);
				printf(" Value             : %d \n",data.data_word.adc);
				printf(" Overflow(Y=1)     : %d \n",data.data_word.overflow);
				printf(" Under Thre(Y=1)   : %d \n",data.data_word.under_thr);
				break;
			case V792_HEADER_ID_EB:
				printf("---------------------------------------\n");
				printf(" End of Block \n");
				printf(" Event Count       : %d \n",data.endof_block.event_count);
				break;
			case V792_HEADER_ID_NV:
				printf(" The Buffer is empty \n");
				endflag = 1; 
				break;

			default:
			printf("  Unkown Header id:  %.2x(%d)\n", data.header.id, data.header.id);
			endflag = 1;
			break;
		}
	}


	fprintf(stderr, "unmap V792\n");

	fprintf(stderr, "close V792\n");
	ret = v792_unmap(module_number);
	if(ret!=0){
		fprintf(stderr, "Error in v792_unmap()\n");
	}


	ret = v792_close();
	if(ret!=0){
		fprintf(stderr, "Error in v792_close()\n");
		exit(-1);
	}
	return 0; 
}


