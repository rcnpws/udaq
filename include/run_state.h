typedef struct {
  pthread_mutex_t lock;
  int status;
} run_state_t;

run_state_t *run_state;          /* Run state                      */

#define RUN_STOP   0
#define RUN_START  1
#define RUN_OFF    2
#define RUN_ON     3
