/* Write "n" bytes to a descriptor. */

#include "tcplib.h"

ssize_t writen(int fd, const void *vptr, size_t n)
{
  size_t		nleft;
  ssize_t		nwritten;
  const char	*ptr;

  ptr = vptr;
  nleft = n;

  
  while (nleft > 0) {
#if 0
	printf("enter writen\n");
#endif
    if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
#if 0 /* modified by M.Uchida 2002/05/03 */
      if (errno == EINTR)
		nwritten = 0;		/* and call write() again */
      else
#endif

#if 0
	printf("writen err %d\n",nwritten);
#endif

      return(-1);		        /* error */
    }

    nleft -= nwritten;
    ptr   += nwritten;
  }
  return(n);
}
/* end writen */


void Writen(int fd, void *ptr, size_t nbytes)
{
  if (writen(fd, ptr, nbytes) != nbytes)
    err_sys("writen error");
}

