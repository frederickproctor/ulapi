/*
  xeno_rtapi.c

  Implementations of RT API functions declared in rtapi.h, for Xenomai.
*/

#include <stddef.h>		/* NULL, sizeof */
#include <stdio.h>		/* vprintf */
#include <stdarg.h>		/* va_start */
#include <stdlib.h>		/* strtoul */
#include <unistd.h>		/* pause */
#include <ctype.h>
#include <sys/ipc.h>		/* IPC_* */
#include <sys/shm.h>		/* shmget() */
#include <sys/mman.h>
#include <alchemy/task.h>
#include <alchemy/timer.h>
#include <alchemy/heap.h>
#include <sys/io.h>

#include "config.h"
#include "rtapi.h"

#define THE_HEAP_SIZE 1048576 	/* 1 MB */
/*
  Set this application global before calling rtapi_app_init if you
  want a different size
*/
int rtapi_heap_size = THE_HEAP_SIZE;

static RT_HEAP the_heap;

char *rtapi_strncpy(char *dest, const char *src, rtapi_integer n)
{
  char *ptr;
  rtapi_integer i;

  for (ptr = dest, i = 0; i < n; ptr++, src++, i++) {
    *ptr = *src;
    if (0 == *src) break;
  }

  return dest;
}

/* can't do this in a real real-time system */
rtapi_result rtapi_system(const char *prog, rtapi_integer *result)
{
  *result = 0;

  return RTAPI_ERROR;
}

enum {
  XENOMAI_PRIO_LOWEST = 0,
  XENOMAI_PRIO_HIGHEST = 99
};

rtapi_prio rtapi_prio_highest(void)
{
  return XENOMAI_PRIO_HIGHEST;
}


rtapi_prio rtapi_prio_lowest(void)
{
  return XENOMAI_PRIO_LOWEST;
}

rtapi_prio rtapi_prio_next_higher(rtapi_prio prio)
{
  if (prio == rtapi_prio_highest())
    return prio;

  return prio + 1;
}

rtapi_prio rtapi_prio_next_lower(rtapi_prio prio)
{
  if (prio == rtapi_prio_lowest())
    return prio;

  return prio - 1;
}

/*
  The resolution of the Alchemy clock can be set using the
  --alchemy-clock-resolution=<nsec> option when starting the
  application process (defaults to 1 nanosecond). If your application
  sets it, e.g., it was ported from another real-time system that
  allows setting the clock period, you can force the Alchemy period
  when launching it to prevent the error.
 */
rtapi_result rtapi_clock_set_period(rtapi_integer nsecs)
{
  RT_TIMER_INFO info;

  rt_timer_inquire(&info);

  if (nsecs != info.period) return RTAPI_ERROR;
  return RTAPI_OK;
}

rtapi_result rtapi_clock_get_time(rtapi_integer *secs, rtapi_integer *nsecs)
{
  SRTIME ns;

  ns = rt_timer_ticks2ns(rt_timer_read());

  *secs = ns / 1000000000;
  *nsecs = ns % 1000000000;

  return RTAPI_OK;
}

rtapi_result rtapi_clock_get_interval(rtapi_integer start_secs,
				      rtapi_integer start_nsecs,
				      rtapi_integer end_secs,
				      rtapi_integer end_nsecs,
				      rtapi_integer *diff_secs,
				      rtapi_integer *diff_nsecs)
{
  if (end_nsecs < start_nsecs) {
    if (end_secs < start_secs) {
      /* 1.1 - 9.9 */
      *diff_nsecs = start_nsecs - end_nsecs;
      *diff_secs = start_secs - end_secs;
    } else {
      /* 9.1 - 1.9 */
      *diff_nsecs = 1000000000 - start_nsecs + end_nsecs;
      *diff_secs = end_secs - start_secs - 1;
    }
  } else {
    if (end_secs < start_secs) {
      /* 1.9 - 9.1 */
      *diff_nsecs = 1000000000 - end_nsecs + start_nsecs;
      *diff_secs = start_secs - end_secs - 1;
    } else {
      /* 9.9 - 1.1 */
      *diff_nsecs = end_nsecs - start_nsecs;
      *diff_secs = end_secs - start_secs;
    }
  }

  return RTAPI_OK;
}

rtapi_task_struct *rtapi_task_new(void)
{
  RT_TASK *task;
  
  task = rtapi_new(sizeof(RT_TASK));

  return task;
}

rtapi_result rtapi_task_delete(rtapi_task_struct *task)
{
  if (0 == task) {
    return RTAPI_ERROR;
  }

  rt_task_delete(task);

  rtapi_free(task);
  
  return RTAPI_OK;
}

rtapi_integer rtapi_task_stack_check(rtapi_task_struct *task)
{
  return -1;
}

rtapi_result rtapi_task_start(rtapi_task_struct *task,
			      void (*taskcode)(void *),
			      void *taskarg,
			      rtapi_prio prio,
			      rtapi_integer stacksize,
			      rtapi_integer period_nsec,
			      rtapi_flag uses_fp)
{
  int retval;
  
  retval = rt_task_create(
    task,
    NULL,			/* optional name string */
    stacksize,
    prio,
    0);			      /* mode, could be T_joinable | T_LOCK */
  if (0 != retval) return RTAPI_ERROR;

  retval = rt_task_set_periodic(task, TM_NOW, rt_timer_ns2ticks(period_nsec));
  if (0 != retval) {
    rt_task_delete(task);
    return RTAPI_ERROR;
  }

  retval = rt_task_start(task, taskcode, taskarg);
  if (0 != retval) {
    rt_task_delete(task);
    return RTAPI_ERROR;
  }

  return RTAPI_OK;
}

rtapi_result rtapi_task_stop(rtapi_task_struct *task)
{
  return (rt_task_delete(task) == 0) ? (RTAPI_OK) : (RTAPI_ERROR);
}

rtapi_result rtapi_task_pause(rtapi_task_struct *task)
{
  return (rt_task_suspend(task) == 0) ? (RTAPI_OK) : (RTAPI_ERROR);
}

rtapi_result rtapi_task_resume(rtapi_task_struct *task)
{
  return (rt_task_resume(task) == 0) ? (RTAPI_OK) : (RTAPI_ERROR);
}

rtapi_result rtapi_task_set_period(rtapi_task_struct *task, rtapi_integer period_nsec)
{
  rt_task_set_periodic(NULL, TM_NOW, period_nsec);

  return RTAPI_OK;
}

rtapi_result rtapi_task_init(rtapi_task_struct *task)
{
  return RTAPI_OK;		/* nothing needs to be done */
}

rtapi_result rtapi_self_set_period(rtapi_integer period_nsec)
{
  return (rt_task_set_periodic(NULL, TM_NOW, rt_timer_ns2ticks(period_nsec)) == 0) ? (RTAPI_OK) : (RTAPI_ERROR);
}

rtapi_result rtapi_wait(rtapi_integer period_nsec)
{
  /* the period is already set in the task, so ignore it here */
  rt_task_wait_period(NULL);

  return RTAPI_OK;
}

rtapi_result rtapi_task_exit(void)
{
  return RTAPI_OK;		/* nothing need be done */
}

/*
  The rtapi_shm_ functions are for user-space processes to connect to
  RT shared memory. The rtapi_rtm_ functions are for RT processes to
  connect to user-space processes. In Xenomai, they are the same.
*/

typedef struct {
  rtapi_id key;
  rtapi_integer size;
  rtapi_id id;
  void * addr;
} shm_struct;

void * rtapi_shm_new(rtapi_id key, rtapi_integer size)
{
  shm_struct *shm;

  /* here we use the Unix 'malloc' function instead of rtapi_new, since
     the caller is a user-level process */
  shm = malloc(sizeof(shm_struct));
  if (NULL == (void *) shm) return NULL;

  shm->id = shmget((key_t) key, (int) size, IPC_CREAT | 0666);
  if (-1 == shm->id) {
    free(shm);			/* likewise, 'free' vs. 'rtapi_free' */
    return NULL;
  }

  shm->addr = shmat(shm->id, NULL, 0);
  if ((void *) -1 == shm->addr) {
    free(shm);
    return NULL;
  }

  return (void *) shm;
}

void * rtapi_shm_addr(void * shm)
{
  return ((shm_struct *) shm)->addr;
}

rtapi_result rtapi_shm_delete(void * shm)
{
  struct shmid_ds d;
  int r1, r2;

  if (NULL == shm) return RTAPI_OK;

  r1 = shmdt(((shm_struct *) shm)->addr);
  r2 = shmctl(IPC_RMID, ((shm_struct *) shm)->id, &d);

  free(shm);

  return (r1 || r2 ? RTAPI_ERROR : RTAPI_OK);
}

void * rtapi_rtm_new(rtapi_id key, rtapi_integer size)
{
  return rtapi_shm_new(key, size);
}

void * rtapi_rtm_addr(void * rtm)
{
  return rtapi_shm_addr(rtm);
}

rtapi_result rtapi_rtm_delete(void * rtm)
{
  return rtapi_shm_delete(rtm);
}

void rtapi_print(const char *fmt, ...)
{
  va_list args;

  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

void rtapi_outb(char byte, rtapi_id port)
{
  outb(byte, port);
}

char rtapi_inb(rtapi_id port)
{
  return inb(port);
}

rtapi_result rtapi_interrupt_assign_handler(rtapi_id irq,
					    void (*handler) (void))
{
  return RTAPI_ERROR;
}

rtapi_result rtapi_interrupt_free_handler(rtapi_id irq)
{
  return RTAPI_OK;
}

rtapi_result rtapi_interrupt_enable(rtapi_id irq)
{
  return RTAPI_OK;
}

rtapi_result rtapi_interrupt_disable(rtapi_id irq)
{
  return RTAPI_OK;
}

rtapi_mutex_struct *rtapi_mutex_new(rtapi_id key)
{
  return NULL;
}

rtapi_result rtapi_mutex_delete(rtapi_mutex_struct *sem)
{
  return RTAPI_OK;
}

rtapi_result rtapi_mutex_give(rtapi_mutex_struct *sem)
{
  return RTAPI_OK;
}

rtapi_result rtapi_mutex_take(rtapi_mutex_struct *sem)
{
  return RTAPI_OK;
}

void *rtapi_sem_new(rtapi_id key)
{
  return rtapi_mutex_new(key);
}

rtapi_result rtapi_sem_delete(void *sem)
{
  return rtapi_mutex_delete(sem);
}

rtapi_result rtapi_sem_give(void *sem)
{
  return rtapi_mutex_give(sem);
}

rtapi_result rtapi_sem_take(void *sem)
{
  return rtapi_mutex_take(sem);
}

rtapi_result
rtapi_app_init(void)
{
  int retval;

  retval = rt_heap_create(&the_heap, NULL, rtapi_heap_size, 0);

#if HAVE_IOPL
  iopl(3);
#endif

  mlockall(MCL_CURRENT | MCL_FUTURE);  

  return retval ? RTAPI_ERROR : RTAPI_OK;
}

rtapi_result
rtapi_app_wait(void)
{
  pause();

  return RTAPI_OK;
}

void
rtapi_exit(void)
{
  rt_heap_delete(&the_heap);
}

void *
rtapi_new(rtapi_integer size)
{
  int retval;
  void *ptr;

  retval = rt_heap_alloc(&the_heap, size, TM_INFINITE, &ptr);

  if (0 != retval) return NULL;

  return ptr;
}

void 
rtapi_free(void *ptr)
{
  rt_heap_free(&the_heap, ptr);
}

char *
rtapi_arg_get_string(char ** var, char *key)
{
  return *var;
}

int
rtapi_arg_get_int(rtapi_integer *var, char *key)
{
  return *var;
}

rtapi_result
rtapi_string_to_integer(const char *str, rtapi_integer *var)
{
  int i;
  char *endptr;

  i = (int) strtol(str, &endptr, 0);
  if (endptr == str ||
      (! isspace(*endptr) && 0 != *endptr)) {
    *var = 0;
    return RTAPI_ERROR;
  }

  *var = (rtapi_integer) i;
  return RTAPI_OK;
}

const char *
rtapi_string_skipwhite(const char *str)
{
  const char *ptr = str;

  while (isspace(*ptr)) ptr++;

  return ptr;
}

const char *
rtapi_string_skipnonwhite(const char *str)
{
  const char *ptr = str;

  while (! isspace(*ptr) && *ptr != 0) ptr++;

  return ptr;
}

const char *
rtapi_string_skipone(const char *str)
{
  const char *ptr;

  ptr = rtapi_string_skipnonwhite(str);
  return rtapi_string_skipwhite(ptr);
}

char *
rtapi_string_copyone(char *dst, const char *src)
{
  char *dstptr;
  const char *srcptr;

  if (NULL == dst || NULL == src) return dst;

  dstptr = dst, srcptr = rtapi_string_skipwhite(src);
  while (!isspace(*srcptr) && 0 != *srcptr) {
    *dstptr++ = *srcptr++;
  }
  *dstptr = 0;

  return dst;
}

/* we don't have RTAI sockets, so these will all return 'unimplemented' */

rtapi_integer
rtapi_socket_client(rtapi_integer port, const char *host)
{
  return -1;
}

rtapi_integer
rtapi_socket_server(rtapi_integer port)
{
  return -1;
}

rtapi_integer
rtapi_socket_get_client(rtapi_integer id)
{
  return -1;
}

rtapi_result
rtapi_socket_set_nonblocking(rtapi_integer id)
{
  return RTAPI_ERROR;
}

rtapi_result
rtapi_socket_set_blocking(rtapi_integer id)
{
  return RTAPI_ERROR;
}

rtapi_integer
rtapi_socket_read(rtapi_integer id, char *buf, rtapi_integer len)
{
  return -1;
}

rtapi_integer
rtapi_socket_write(rtapi_integer id, const char *buf, rtapi_integer len)
{
  return -1;
}

rtapi_result
rtapi_socket_close(rtapi_integer id)
{
  return RTAPI_ERROR;
}

/*
  At the moment, we don't have RTAI serial ports, so these all return
  errors. There is an RT serial comm interface we should use one day.
*/

static int rtapi_serial_id = -1; /* dummy id */

void *
rtapi_serial_new(void)
{
  return &rtapi_serial_id;
}

rtapi_result
rtapi_serial_delete(void *id)
{
  return RTAPI_OK;
}

rtapi_result
rtapi_serial_open(const char *port, void *id)
{
  return RTAPI_ERROR;
}

rtapi_result
rtapi_serial_baud(void *id, int baud)
{
  return RTAPI_ERROR;
}

rtapi_result
rtapi_serial_set_nonblocking(void *id)
{
  return RTAPI_ERROR;
}

rtapi_result
rtapi_serial_set_blocking(void *id)
{
  return RTAPI_ERROR;
}

rtapi_integer
rtapi_serial_read(void *id,
		  char *buf,
		  rtapi_integer len)
{
  return -1;
}

rtapi_integer
rtapi_serial_write(void *id,
		   const char *buf,
		   rtapi_integer len)
{
  return -1;
}

rtapi_result
rtapi_serial_close(void *id)
{
  return RTAPI_OK;
}
