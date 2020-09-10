/*
	errlog.h ... Definitions for Error Log
  Copyright (C) 1995  A. Tamii
  Author:   A. Tamii
	Facility: Department of Physics, Kyoto University 
	          & Research Center for Nuclear Physics
  Created:  28-JUL-1995 by A. Tamii
*/

#include <time.h>

/*** Definitions of Boolean Constants */
#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

/*** Error Log Key File ***/
#define ERRLOGKEY "/"

/*** constant definitions ***/
#define PNAMLEN      15
#define EMSGLEN      223

/*** error type ***/
#define ErrFatal    100
#define ErrSever     20
#define ErrWarning   10
#define ErrMessage    8
#define ErrComment    7
#define ErrLog        6
#define ErrDebug      3
#define ErrVoid       0

typedef struct err_msg{
  int     mtype;            /* message type */
	int     errtype;          /* error type */
	int     errid;            /* error ID (not used) */
	time_t  time;             /* error time */
  int     pid;              /* process ID */
  char    pnam[PNAMLEN+1];  /* process name */
  char    msg[EMSGLEN+1];   /* error message */
} err_msg_t;


/*** prototypes ***/
int errlog_open();
int errlog_close();
int errlog_msg();
