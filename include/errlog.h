/*
 *
 *  For DAQ Logging system
 *
 */

#define	SERV_PORT_LOG	"9878"	   /* TCP  client-servers for logging */

/* changed by A. Tamii 02-MAR-2006 */

//#define LOG_SERVER      "kasuga1.rcnp.osaka-u.ac.jp"
#define LOG_SERVER      "asagi1.rcnp.osaka-u.ac.jp"

/* end changed 02-MAR-2006 */

struct log_prop {
  char name[50];          /* Process name     */
  char user[20];          /* User name        */
  char ruser[20];         /* Remote user name */
  char host[30];          /* Host name        */
  char comment[200];      /* Comment          */
};


struct log_prop *create_log_prop(char *pname, char *uname);
void delete_log_prop(struct log_prop *lp);
#if 0
int err_open(struct log_prop *prop);
#else
int err_open(char *ename,struct log_prop *prop);
#endif
void err_close(int fd);
int send_err_msg(int fd, struct log_prop *lp);
void Send_err_msg(int fd, struct log_prop *lp);
void rcv_err_msg(int fd, struct log_prop *lp);
int rcv_err(int fd, char *arg);
int send_err(int fd, char *arg);
void Send_err(int fd, char *arg);
