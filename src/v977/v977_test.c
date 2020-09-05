/* v977test.c ---- access test to V977A                                  */
/*								           */
/*  Version 1.00        2012-12-23      by A. Tamii (For Linux2.6)GE-FANUC */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vme/vme.h>
#include <vme/vme_api.h>

#include "v977.h"

int main(){
	int ret;
	unsigned short cr;
	int module_number = 0;
	unsigned short *ptr;

	V977_p v977=(V977_p)NULL;

	fprintf(stderr, "open V977A\n");
	ret = v977_open();
	if(ret!=0){
		fprintf(stderr, "Error in v977_open()\n");
		exit(-1);
	}

	fprintf(stderr, "map V977A(module_number=%d)\n", module_number);
	v977 = v977_map(module_number);
	if(v977==(V977_p)NULL){
		fprintf(stderr, "Error in v977_map()\n");
		exit(-1);
	}

	printf("v977 = 0x%.8lx\n", (long)v977);

	printf("size of *v977 = %d\n", sizeof(*v977));
	ptr = (unsigned short*)v977;
	printf("dummy = %d\n", ptr[0x002a/2]);


	v977->control_register = 0x0002; 
	cr = v977->control_register;
	printf("control register = 0x%.4x\n", cr);


	/*
	v977->software_reset = 0x00;
	printf("Software Reset \n");
	 */

	fprintf(stderr, "unmap V977A\n");

	fprintf(stderr, "close V977A\n");
	ret = v977_unmap(module_number);
	if(ret!=0){
		fprintf(stderr, "Error in v977_unmap()\n");
	}


	ret = v977_close();
	if(ret!=0){
		fprintf(stderr, "Error in v977_close()\n");
		exit(-1);
	}
	return 0; 
}


