
#include <sys/types.h>
#include <sys/ipc.h>	
#include <sys/msg.h>      /* System V message queues   */
#include <sys/shm.h>      /* System V shared memory    */
#include <sys/sem.h>      /* System V semaphores       */

union semun {             /* define union for semctl() */
  int              val;
  struct semid_ds *buf;
  unsigned short  *array;
};

#define	SVMSG_MODE   (MSG_R | MSG_W | MSG_R>>3 | MSG_R>>6)
#define	SVSEM_MODE   (SEM_R | SEM_A | SEM_R>>3 | SEM_R>>6)
#define	SVSHM_MODE   (SHM_R | SHM_W | SHM_R>>3 | SHM_R>>6)

/* 4System V message queues */
int	 Msgget(key_t key, int flag);
void	 Msgctl(int, int, struct msqid_ds *);
void	 Msgsnd(int, const void *, size_t, int);
ssize_t	 Msgrcv(int, void *, size_t, int, int);

/* 4System V semaphores */
int	 Semget(key_t, int, int);
int	 Semctl(int, int, int, ...);
void	 Semop(int, struct sembuf *, size_t);

/* 4System V shared memory */
int	 Shmget(key_t, size_t, int);
void	*Shmat(int, const void *, int);
void	 Shmdt(const void *);
void	 Shmctl(int, int, struct shmid_ds *);

