#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <unistd.h>
#include <netdb.h>
#include <poll.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/msg.h>

#define MAXLINE         4096    /* max text line length       */

/* Define some port number that can be used for client-servers */

#define	SERV_PORT       9877    /* TCP and UDP client-servers */
#define	SERV_PORT_STR  "9877"   /* TCP and UDP client-servers */

/* Following shortens all the type casts of pointer arguments */

#define	SA	struct sockaddr
#define	LISTENQ		1024	/* 2nd argument to listen() */

/* Prototype Declaration */

int      Tcp_connect(const char *, const char *);
int      Tcp_listen(const char *, const char *, socklen_t *);

ssize_t  Readn(int, void *, size_t);
void     Writen(int, void *, size_t);

void Listen(int fd, int backlog);


void	*Calloc(size_t, size_t);
void	 Close(int);

void Getsockopt(int fd, int level, int optname, void *optval, socklen_t *optlenptr);

void Setsockopt(int fd, int level, int optname, const void *optval, socklen_t optlen);

#if 0
void	 Dup2(int, int);
int	 Fcntl(int, int, int);
void	 Gettimeofday(struct timeval *, void *);
int	 Ioctl(int, int, void *);
pid_t	 Fork(void);
void	*Malloc(size_t);
void	 Mktemp(char *);
void	*Mmap(void *, size_t, int, int, int, off_t);
int	 Open(const char *, int, mode_t);

void	 Pipe(int *fds);
ssize_t	 Read(int, void *, size_t);
void	 Sigaddset(sigset_t *, int);
void	 Sigdelset(sigset_t *, int);
void	 Sigemptyset(sigset_t *);
void	 Sigfillset(sigset_t *);
int	 Sigismember(const sigset_t *, int);
void	 Sigpending(sigset_t *);
void	 Sigprocmask(int, const sigset_t *, sigset_t *);
char	*Strdup(const char *);
long	 Sysconf(int);
void	 Sysctl(int *, u_int, void *, size_t *, void *, size_t);
void	 Unlink(const char *);
pid_t	 Wait(int *);
pid_t	 Waitpid(pid_t, int *, int);
void	 Write(int, void *, size_t);
#endif

void     err_dump(const char *, ...);
void     err_msg(const char *, ...);
void     err_quit(const char *, ...);
void     err_ret(const char *, ...);
void     err_sys(const char *, ...);
