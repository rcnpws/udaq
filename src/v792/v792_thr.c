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


	v792->threshold_0=0x00ff;
	printf("Threshold_0         = %d (0x%.4x)\n",v792->threshold_0,v792->threshold_0);
	v792->threshold_1=0x00ff;
	printf("Threshold_1         = %d (0x%.4x)\n",v792->threshold_1,v792->threshold_1);
	v792->threshold_2=0x00ff;
	printf("Threshold_2         = %d (0x%.4x)\n",v792->threshold_2,v792->threshold_2);
	v792->threshold_3=0x00ff;
	printf("Threshold_3         = %d (0x%.4x)\n",v792->threshold_3,v792->threshold_3);
	v792->threshold_4=0x00ff;
	printf("Threshold_4         = %d (0x%.4x)\n",v792->threshold_4,v792->threshold_4);
	v792->threshold_5=0x00ff;
	printf("Threshold_5         = %d (0x%.4x)\n",v792->threshold_5,v792->threshold_5);
	v792->threshold_6=0x00ff;
	printf("Threshold_6         = %d (0x%.4x)\n",v792->threshold_6,v792->threshold_6);
	v792->threshold_7=0x00ff;
	printf("Threshold_7         = %d (0x%.4x)\n",v792->threshold_7,v792->threshold_7);
	v792->threshold_8=0x00ff;
	printf("Threshold_8         = %d (0x%.4x)\n",v792->threshold_8,v792->threshold_8);
	v792->threshold_9=0x00ff;
	printf("Threshold_9         = %d (0x%.4x)\n",v792->threshold_9,v792->threshold_9);
	v792->threshold_10=0x00ff;
	printf("Threshold_10         = %d (0x%.4x)\n",v792->threshold_10,v792->threshold_10);
	v792->threshold_11=0x00ff;
	printf("Threshold_11         = %d (0x%.4x)\n",v792->threshold_11,v792->threshold_11);
	v792->threshold_12=0x00ff;
	printf("Threshold_12         = %d (0x%.4x)\n",v792->threshold_12,v792->threshold_12);
	v792->threshold_13=0x00ff;
	printf("Threshold_13         = %d (0x%.4x)\n",v792->threshold_13,v792->threshold_13);
	v792->threshold_14=0x00ff;
	printf("Threshold_14         = %d (0x%.4x)\n",v792->threshold_14,v792->threshold_14);
	v792->threshold_15=0x00ff;
	printf("Threshold_15         = %d (0x%.4x)\n",v792->threshold_15,v792->threshold_15);
	
	
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


