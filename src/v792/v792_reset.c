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
	unsigned short bs1r;
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


	bs1r = v792->bit_set_1;

	//software reset
	printf("software reset\n");
	v792->bit_set_1 = bs1r | 0x80;
	v792->bit_clear_1 = 0x80;
	bs1r = v792->bit_set_1;
	
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


