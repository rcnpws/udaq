#include    "tcplib.h"
#include    "ipclib.h"
#include    <stdarg.h>   /* ANSI C header file */

/*  SYSTEM V MESSAGE QUEUES   */

int
Msgget(key_t key, int flag)
{
  int		rc;

  if ( (rc = msgget(key, flag)) == -1)
    err_sys("msgget error");
  return(rc);
}

void
Msgctl(int id, int cmd, struct msqid_ds *buf)
{
  if (msgctl(id, cmd, buf) == -1)
    err_sys("msgctl error");
}

void
Msgsnd(int id, const void *ptr, size_t len, int flag)
{
  if (msgsnd(id, ptr, len, flag) == -1)
    err_sys("msgsnd error");
}

ssize_t
Msgrcv(int id, void *ptr, size_t len, int type, int flag)
{
  ssize_t	rc;

  if ( (rc = msgrcv(id, ptr, len, type, flag)) == -1)
    err_sys("msgrcv error");
  return(rc);
}

/*  SYSTEM V SHARED MEMORY   */

int
Shmget(key_t key, size_t size, int flags)
{
  int		rc;

  if ( (rc = shmget(key, size, flags)) == -1)
    err_sys("shmget error");
  return(rc);
}

void *
Shmat(int id, const void *shmaddr, int flags)
{
  void	*ptr;

  if ( (ptr = shmat(id, shmaddr, flags)) == (void *) -1)
    err_sys("shmat error");
  return(ptr);
}

void
Shmdt(const void *shmaddr)
{
  if (shmdt((char *)shmaddr) == -1)
    err_sys("shmdt error");
}

void
Shmctl(int id, int cmd, struct shmid_ds *buff)
{
  if (shmctl(id, cmd, buff) == -1)
    err_sys("shmctl error");
}

/*  SYSTEM V SEMAPHORES   */

int
Semget(key_t key, int nsems, int flag)
{
  int		rc;

  if ( (rc = semget(key, nsems, flag)) == -1)
    err_sys("semget error");
  return(rc);
}

void
Semop(int id, struct sembuf *opsptr, size_t nops)
{
  if (semop(id, opsptr, nops) == -1)
    err_sys("semctl error");
}

int
Semctl(int id, int semnum, int cmd, ...)
{
  int		rc;
  va_list	ap;
  union semun	arg;

  if (cmd == GETALL || cmd == SETALL || cmd == SETVAL ||
      cmd == IPC_STAT || cmd == IPC_SET) {
    va_start(ap, cmd);		/* init ap to final named argument */
    arg = va_arg(ap, union semun);
    if ( (rc = semctl(id, semnum, cmd, arg)) == -1)
      err_sys("semctl error");
    va_end(ap);
  } else {
    if ( (rc = semctl(id, semnum, cmd)) == -1)
      err_sys("semctl error");
  }
  return(rc);
}
