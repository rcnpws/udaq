/*
	check_format.c ... check the format of buffers
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Researc  Center for Nuclear Physics
  Created:  07-MAY-1995 by A. Tamii
*/

#include <sys/types.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef OSF1
#include <strings.h>
#endif
#include <errno.h>
#include <mtformat.h>

#define BLKSIZE 16384

char buf[BLKSIZE];

extern int base;

main(argc, argv)
		 int    argc;
		 char   *argv[];
{
	int    count;
	char   *data;
	int    size;
	int    i, n;
  int    flag;
	int    fd;
	
	if(argc>2){
		fprintf(stderr, "%s ... check the block format of the target file\n", argv[0]);
		fprintf(stderr, "  Usage: %s [file_name] \n", argv[0]);
    fprintf(stderr, "  file_name ... target file name [defalut=stdin] \n");
		fprintf(stderr, "  Block Size = %d Bytes\n", BLKSIZE);
		exit(1);
	}

  if(argc<=0){
		printf("Error in arguments.\n");
		exit(1);
	}

	if(argc>=2){
		fd = open(argv[1], O_RDONLY, 0666);
		if(fd==-1){
			fprintf(stderr, "Couldn't open file: %s\n", argv[1]);
			perror("open");
			exit(1);
		}
		printf("Check the data format in the file: %s\n", argv[1]);
	}else
		fd = 0;
	
	/* clear counters */
	clear_counters();
	
	while(1){
		count = 0;
		while(count<BLKSIZE){
			n = read(fd, &buf[count], BLKSIZE-count);
			if(n==-1){
				perror("read");
				break;
			}
			if(n==0)
				break;
			count += n;
		}
		if(n<0) break;
		if(count==0)
			break;

		base = (int)buf;
		check_buf(buf, count/2);
		show_counters();

		if(count!=BLKSIZE){
			printf("Unexpected end of file.\n");
			break;
		}
	}
	close(fd);
}

