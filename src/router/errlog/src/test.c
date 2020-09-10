#include "errlog.h"

main(argc, argv)
		 int    argc;
		 char   *argv[];
{
	errlog_open(argv[0]);
	errlog_msg(ErrFatal,"This is a test string from test.c .");
	errlog_close();
}
