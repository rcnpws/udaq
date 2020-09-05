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
	int ret;
	unsigned short bs1r,sr1,cr1,sr2,bs2r,ecl,ech;
	unsigned long *buff,data;
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

	printf("v792 = 0x%.8lx\n", (long)v792);

	printf("size of *v792       = %d (0x%.4x)\n", sizeof(*v792),sizeof(*v792));
	bs1r = v792->bit_set_1;
	printf("bit set 1 register  = 0x%.4x\n", bs1r);

	/*
	//software reset
	printf("software reset\n");
	v792->bit_set_1 = bs1r | 0x80;
	v792->bit_clear_1 = 0x80;
	bs1r = v792->bit_set_1;
	printf("bit set 1 register  = 0x%.4x\n", bs1r);
	*/
	sr1 = v792->status_register_1;
	printf("status register 1   = 0x%.4x\n", sr1);
	cr1 = v792->control_register_1;
	printf("control register 1  = 0x%.4x\n", cr1);
	sr2 = v792->status_register_2;
	printf("status register 1   = 0x%.4x\n", sr1);
	bs2r = v792->bit_set_2;
	printf("status register 2   = 0x%.4x\n", sr2);
	ecl = v792->event_counter_l;
	printf("event counter low   = 0x%.4x\n", ecl);
	ech = v792->event_counter_h;
	printf("event counter high  = 0x%.4x\n", ech);

	/*
	v792->threshold_1=0x00ff;
	printf("Threshold_1         = %d (0x%.4x)\n",v792->threshold_1,v792->threshold_1);
	*/

	buff = v792->output_buffer;
	data = *buff;
	printf("output buffer  = 0x%.8lx\n", data);
	

	
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


