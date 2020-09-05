/* v1720test.c ---- access test to V1720                                  */
/*								           */
/*  Version 1.00        2013-07-31      by oiwa (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v1720.h"

int main(){
	int ret;
	unsigned long bi,bid,aqsr,aqcr;
	int module_number = 0;

	V1720_p v1720=(V1720_p)NULL;

	fprintf(stderr, "open V1720\n");
	ret = v1720_open();
	if(ret!=0){
		fprintf(stderr, "Error in v1720_open()\n");
		exit(-1);
	}

	fprintf(stderr, "map V1720(module_number=%d)\n", module_number);
	v1720 = v1720_map(module_number);
	if(v1720==(V1720_p)NULL){
		fprintf(stderr, "Error in v1720_map()\n");
		exit(-1);
	}

	printf("v1720 = 0x%.8lx\n", (long)v1720);

	printf("size of *v1720 = 0x%.4x\n", sizeof(*v1720));
	
	bi = v1720->BOARD_INFO;
	printf("BOARD INFO is = 0x%.8lx\n",bi);

	bid = v1720->BOARD_ID;
	printf("BOARD ID is = 0x%.8lx\n",bid);
	
	aqsr = v1720->ACQUISITION_STATUS;
	printf("ACQUISITION STATUS is = 0x%.8lx\n",aqsr);

	v1720->ACQUISITION_CONTROL = 0x0000000C ;
	aqcr = v1720->ACQUISITION_CONTROL;
	printf("ACQUISITION CONTROL is = 0x%.8lx\n",aqcr);

	/*
	printf("  Pattern Bit: %s\n", (cr&0x0001)==0 ? "0...I/O REGISTER" : "1...MULTIHIT PATTERN UNIT");
	printf("  Gate From FRONT PANEL signal is: %s\n", (cr&0x0002)==0 ? "0...enabled " : "1...masked");
	printf("  OR and /OR FRONT outputs are: %s\n", (cr&0x0002)==0 ? "0...enabled " : "1...masked");
	//unsigned short dummy,sn,ir;
*/


	/*
	   ir = v1720->input_read;
	   printf("input read  = 0x%.4x\n", ir);

	 */

/*
	fprintf(stderr,"output to 0 ch\n");
	v1720->output_set = 0x0001;
	usleep(0);
	v1720->output_set = 0x0000;
*/

	/*
	   dummy = v1720->dummy_register;
	   printf("dummy register = 0x%.4x\n", dummy);

	 */
	/*
	   sn = v1720 ->serial_number;
	   printf("serial number = 0x%.4x\n", sn);


	   */

	/*
	v1720->software_reset = 0x00;
	printf("Software Reset \n");
	 */

	/*
	   v1720->dummy_register = 0x00AA;

	   dummy = v1720->dummy_register;
	   printf("dummy register = 0x%.4x\n", dummy);
	 */
	fprintf(stderr, "unmap V1720\n");

	fprintf(stderr, "close V1720\n");
	ret = v1720_unmap(module_number);
	if(ret!=0){
		fprintf(stderr, "Error in v1720_unmap()\n");
	}


	ret = v1720_close();
	if(ret!=0){
		fprintf(stderr, "Error in v1720_close()\n");
		exit(-1);
	}
	return 0; 
}


