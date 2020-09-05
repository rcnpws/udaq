#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/time.h>
#include <sys/resource.h>
     
static void
showlimit(int resource, char* str)
{
  struct rlimit lim;
  if (getrlimit(resource, &lim) != 0) {
    (void)printf("Couldn't retrieve %s limit\n", str);
    return;
  }
  (void)printf("Current/maximum %s limit is \t%lu / %lu\n", 
	       str, lim.rlim_cur, lim.rlim_max);
}
     
/*ARGSUSED*/
int
main(int argc, char*argv[])
{
  showlimit(RLIMIT_DATA,  "data");
  showlimit(RLIMIT_STACK, "stack");
  showlimit(RLIMIT_VMEM,  "vmem");
     
  return 0;
}
