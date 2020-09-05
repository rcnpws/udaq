/*
  test_config.c ---- a program for testing config.c
  Copyright (C) 2016 Research Center for Nuclear Physics, Osaka University
  Author: A. Tamii, Rsearch Center for Nuclear Physics
  Version 1.00 11-APR-2016 by A. Tamii
*/

/*
Local Variables:
mode: C
tab-width: 2
End:
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "config.h"

int main(int argc, char *argv[]){
	config_read_file("test.ini");

	fprintf(stderr, "aaa   = %s\n", config_get_s_value("aaa",0,"aaaa"));
	fprintf(stderr, "a0bb  = %s\n", config_get_s_value("a0bb",0,"aaaa"));
	fprintf(stderr, "a0bc  = %s\n", config_get_s_value("a0bc",0,"aaaa"));
	fprintf(stderr, "val2  = %s\n", config_get_s_value("val2",0,"aaaa"));

	fprintf(stderr, "aaa   = %ld\n", config_get_l_value("aaa",0,1000));
	fprintf(stderr, "a0bb  = %ld\n", config_get_l_value("a0bb",0,1000));
	fprintf(stderr, "a0bc  = %ld\n", config_get_l_value("a0bc",0,1000));
	fprintf(stderr, "val2  = %ld\n", config_get_l_value("val2",0,1000));
	fprintf(stderr, "val2[0]  = %ld\n", config_get_l_value("val2",0,1000));
	fprintf(stderr, "val2[1]  = %ld\n", config_get_l_value("val2",1,1000));
	fprintf(stderr, "val2[50]  = %ld\n", config_get_l_value("val2",50,1000));
	fprintf(stderr, "val2[-50]  = %ld\n", config_get_l_value("val2",-50,1000));
	fprintf(stderr, "val2[-100]  = %ld\n", config_get_l_value("val2",-100,1000));
	fprintf(stderr, "val2  = %lf\n", config_get_d_value("val2",0,123.456));

	config_read_file("e404_gr.conf");

	return 0;
}
