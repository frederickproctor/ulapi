/*!
  \file ultest.c

  \brief Ad-hoc test program for ulapi functions. Stick stuff in here
  for testing.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>		/* tmpnam */
#include <stddef.h>		/* NULL, sizeof */
#include <stdlib.h>		/* malloc */
#include <math.h>		/* fabs */
#include "ulapi.h"		/* these decls */

static ulapi_integer count;

enum {SHM_SIZE = 1024};

typedef struct {
  char array[SHM_SIZE];
} shm_struct;

static ulapi_result test_shm(void)
{
  void *mutex;
  void *shm;
  ulapi_id mutex_key = 101;
  ulapi_id shm_key = 102;

  mutex = ulapi_mutex_new(mutex_key);
  if (NULL == mutex) {
    ulapi_print("can't allocate mutex\n");
    return ULAPI_ERROR;
  }

  shm = ulapi_shm_new(shm_key, sizeof(shm_struct));
  if (NULL == shm) {
    ulapi_print("can't allocate shared memory\n");
    return ULAPI_ERROR;
  }

  ulapi_shm_delete(shm);
  ulapi_mutex_delete(mutex);

  return ULAPI_OK;
}

typedef struct {
  void *mutex;
  void *cond;
  ulapi_integer id;
} cond_wait_args;

static void cond_wait_code(void *args)
{
  void *mutex;
  void *cond;
  ulapi_integer id;

  mutex =  ((cond_wait_args *) args)->mutex;
  cond =  ((cond_wait_args *) args)->cond;
  id = ((cond_wait_args *) args)->id;
  free(args);

  ulapi_print("waiter %d waiting for condition...\n", id);
  ulapi_mutex_take(mutex);
  if (count > 0) {
    ulapi_cond_wait(cond, mutex);
  }
  ulapi_mutex_give(mutex);
  ulapi_print("waiter %d condition met\n", id);

  ulapi_task_exit(id);
}

typedef struct {
  void *mutex;
  void *cond;
} cond_signal_args;

#define NUM_WAITERS 3

#define COND_SIGNAL_RETVAL 17

void cond_signal_code(void *args)
{
  void *mutex;
  void *cond;
  ulapi_integer done;

  mutex =  ((cond_signal_args *) args)->mutex;
  cond =  ((cond_signal_args *) args)->cond;
  free(args);

  for (done = 0; done == 0; ulapi_wait(1000000000)) {
    ulapi_print("working on condition...\n");
    ulapi_mutex_take(mutex);
    if (count-- <= 0) {
      done = 1;
      if (NUM_WAITERS > 1) {
	ulapi_cond_broadcast(cond);
	ulapi_print("broadcasting condition\n");
      } else {
	ulapi_cond_signal(cond);
	ulapi_print("signaling condition\n");
      }
    }
    ulapi_mutex_give(mutex);
  }

  ulapi_task_exit(COND_SIGNAL_RETVAL);
}

static ulapi_result test_cond(ulapi_integer c)
{
  ulapi_integer t;
  void *cond_wait_task[NUM_WAITERS];
  ulapi_task_struct cond_signal_task;
  void *mutex;
  void *cond;
  cond_wait_args *cond_wait_args_ptr;
  cond_signal_args *cond_signal_args_ptr;
  ulapi_id mutex_key = 101;
  ulapi_id cond_key = 102;
  ulapi_integer ret;

  count = c;

  mutex = ulapi_mutex_new(mutex_key);
  if (NULL == mutex) {
    ulapi_print("can't allocate mutex\n");
    return ULAPI_ERROR;
  }

  cond = ulapi_cond_new(cond_key);
  if (NULL == cond) {
    ulapi_print("can't allocate condition variable\n");
    return ULAPI_ERROR;
  }

  for (t = 0; t < NUM_WAITERS; t++) {
    cond_wait_task[t] = ulapi_task_new();
    if (NULL == cond_wait_task[t]) {
      ulapi_print("can't allocate wait task\n");
      return ULAPI_ERROR;
    }
  }

  for (t = 0; t < NUM_WAITERS; t++) {
    cond_wait_args_ptr = malloc(sizeof(cond_wait_args));
    cond_wait_args_ptr->mutex = mutex;
    cond_wait_args_ptr->cond = cond;
    cond_wait_args_ptr->id = t;
    ulapi_task_start(cond_wait_task[t], cond_wait_code, cond_wait_args_ptr, ulapi_prio_lowest(), 0);
  }

  if (ULAPI_OK != ulapi_task_init(&cond_signal_task)) {
    ulapi_print("can't initialize signal task\n");
    return ULAPI_ERROR;    
  }

  cond_signal_args_ptr = malloc(sizeof(cond_signal_args));
  cond_signal_args_ptr->mutex = mutex;
  cond_signal_args_ptr->cond = cond;
  ulapi_task_start(&cond_signal_task, cond_signal_code, cond_signal_args_ptr, ulapi_prio_lowest(), 0);

  ulapi_task_join(&cond_signal_task, &ret);
  if (ret != COND_SIGNAL_RETVAL) ulapi_print("cond_signal_task value incorrect: %d != %d\n", (int) ret, COND_SIGNAL_RETVAL);
  ulapi_task_clear(&cond_signal_task);

  for (t = 0; t < NUM_WAITERS; t++) {
    ulapi_task_join(cond_wait_task[t], &ret);
    if (ret != t) ulapi_print("cond_wait_task value incorrect: %d != %d\n", (int) ret, (int) t);
    ulapi_task_delete(cond_wait_task[t]);
  }

  ulapi_cond_delete(cond);
  ulapi_mutex_delete(mutex);

  return ULAPI_OK;
}

static ulapi_result test_gethostname(void)
{
  ulapi_integer addr;

  addr = ulapi_get_host_address();

  if (0 == addr) return ULAPI_ERROR;
  return ULAPI_OK;
}

static ulapi_result test_fd_stat(const char *path)
{
  char *tn;
  char *base;
  ulapi_result retval;

  if (NULL == path) {
    tn = tmpnam(NULL);
    /* force the file to be in the current directory */
    base = malloc(strlen(tn) + 2);
    ulapi_basename(tn, base);
	
    if (NULL == fopen(base, "w")) {
      return ULAPI_ERROR;
    }
    retval = ulapi_fd_stat(base);
    remove(base);
  } else {
    retval = ulapi_fd_stat(path);
  }
  
  return retval;
}

#ifdef WIN32
#define WAIT_TEN_PROC "ping -n 10 -w 1000 127.0.0.1"
#define FALSE_PROC "ping -n 1 0.0.0.0"
#else
#define WAIT_TEN_PROC "/bin/ping -c 10 127.0.0.1"
#define FALSE_PROC "/bin/false"
#endif

static ulapi_result test_process(void)
{
  void *ph;
  ulapi_integer is_done;
  ulapi_integer result;
  ulapi_result retval;

  /* start a ten-second process */
  ph = ulapi_process_new();
  if (NULL == ph) return ULAPI_ERROR;
  retval = ulapi_process_start(ph, WAIT_TEN_PROC);
  if (ULAPI_OK != retval) return retval;

  /* wait three seconds and check it, then wait 3 seconds and kill it; 
     you will need to verify this happens by watching the output */
  ulapi_sleep(3);
  is_done = ulapi_process_done(ph, &result);
  if (is_done) {
    printf("process returned with result %d\n", (int) result);
  } else {
    printf("process is still running\n");
  }

  ulapi_sleep(3);
  retval = ulapi_process_stop(ph);
  ulapi_sleep(1);		/* race -- give it some time */
  if (ULAPI_OK != retval) return retval;
  is_done = ulapi_process_done(ph, &result);
  if (is_done) {
    printf("process returned with result %d\n", (int) result);
  } else {
    printf("process is still running\n");
  }
  ulapi_process_delete(ph);

  /* start another ten-second process */
  ph = ulapi_process_new();
  if (NULL == ph) return ULAPI_ERROR;
  retval = ulapi_process_start(ph, WAIT_TEN_PROC);
  if (ULAPI_OK != retval) {
    printf("can't start '%s'\n", WAIT_TEN_PROC);
    return retval;
  }

  /* wait the whole time; you will likewise need to verify this */
  retval = ulapi_process_wait(ph, &result);
  if (ULAPI_OK == retval) {
    printf("process returned with result %d\n", (int) result);
  } else {
    printf("error waiting for the process\n");
  }

  ulapi_process_delete(ph);

  /* check for a non-zero return value */
  ph = ulapi_process_new();
  if (NULL == ph) return ULAPI_ERROR;
  retval = ulapi_process_start(ph, FALSE_PROC);
  if (ULAPI_OK != retval) return retval;
  retval = ulapi_process_wait(ph, &result);
  printf("result is %d\n", (int) result);
  if (ULAPI_OK == retval && 0 != result) {
    retval = ULAPI_OK;
  } else {
    printf("error checking for non-zero result");
    retval = ULAPI_ERROR;
  }

  ulapi_process_delete(ph);

  return retval;
}

static ulapi_result test_sxprintf(void)
{
  size_t buffer_size = 1;
  char *buffer = NULL;

  buffer = realloc(buffer, buffer_size);
  if (NULL == buffer) return ULAPI_ERROR;

  if (ULAPI_OK != ulapi_sxprintf(&buffer, &buffer_size, "%d %s", 123, "this is a test")) {
    return ULAPI_ERROR;
  }

  if (0 != strcmp(buffer, "123 this is a test")) return ULAPI_ERROR;

  return ULAPI_OK;
}

static ulapi_result test_time_string(void)
{
  char buffer[] = "1970-01-01T00:00:00Z";
  const char *ptr;
  size_t len = sizeof(buffer);

  ptr = ulapi_time_string(NULL, 0);
  if (NULL == ptr) return ULAPI_ERROR;
  /* some heuristics */
  if (ptr[0] != '2' || ptr[strlen(ptr)-1] != 'Z') return ULAPI_ERROR;

  ptr = ulapi_time_string(buffer, len);
  if (NULL == ptr) return ULAPI_ERROR;
  if (ptr[0] != '2' || ptr[strlen(ptr)-1] != 'Z') return ULAPI_ERROR;
  if (buffer[0] != '2' || buffer[strlen(buffer)-1] != 'Z') return ULAPI_ERROR;

  printf("Time string is %s\n", ptr);

  return ULAPI_OK;
}

int main(int argc, char *argv[])
{
  enum {BUFFERLEN = 80};
  char inbuf[BUFFERLEN];
  char outbuf[BUFFERLEN];
  ulapi_real start, diff;
  ulapi_result retval;

  /* check for specific testing requests */
  if (argc > 1) {
    inbuf[sizeof(inbuf)-1] = 0;
    outbuf[sizeof(inbuf)-1] = 0;
    retval = 0;
    if (! strcmp(argv[1], "basename")) {
      while ((!feof(stdin)) &&
	     (NULL != fgets(inbuf, sizeof(inbuf)-1, stdin))) {
	ulapi_print("%s", ulapi_basename(inbuf, outbuf));
      }
    } else if (! strcmp(argv[1], "dirname")) {
      while ((!feof(stdin)) &&
	     (NULL != fgets(inbuf, sizeof(inbuf)-1, stdin))) {
	ulapi_print("%s\n", ulapi_dirname(inbuf, outbuf));
      }
    } else if (! strcmp(argv[1], "1")) {
      /* add simple local test here */
      retval = test_sxprintf();
      if (ULAPI_OK != retval) {
	ulapi_print("ultest sxprintf test failed\n");
	return 1;
      }
      ulapi_print("ultest sxprintf test passed\n");
      return 0;
    } else if (! strcmp(argv[1], "2")) {
      /* ditto */
      retval = test_time_string();
      if (ULAPI_OK != retval) {
	ulapi_print("ultest time_string test failed\n");
	return 1;
      }
      ulapi_print("ultest time_string test passed\n");
      return 0;
    } else {
      ulapi_print("unknown argument: %s\n", argv[1]);
      retval = 1;
    }
    return retval;
  }

  retval = ulapi_init();
  if (ULAPI_OK != retval) {
    return 1;
  }

  ulapi_set_debug(ULAPI_DEBUG_ALL);

  retval = test_process();
  if (ULAPI_OK != retval) {
    ulapi_print("ultest process test failed\n");
    return 1;
  }
  ulapi_print("ultest process test passed\n");

  start = ulapi_time();
  ulapi_wait(1000000000);	/* one second in nanoseconds */
  diff = ulapi_time() - start;
  if (fabs(diff - 1) > 0.100) {
    ulapi_print("ultest timer failed: %f instead of 1", (double) diff);
    return 1;
  }
  ulapi_print("ultest timer passed\n");

  retval = test_shm();
  if (ULAPI_OK != retval) {
    ulapi_print("ultest shared memory test failed\n");
    return 1;
  }
  ulapi_print("ultest shared memory test passed\n");

  retval = test_cond(3);
  if (ULAPI_OK != retval) {
    ulapi_print("ultest condition variable test failed\n");
    return 1;
  }
  ulapi_print("ultest condition variable test passed\n");

  retval = test_fd_stat(NULL);
  if (ULAPI_OK != retval) {
	  ulapi_print("ultest fd stat test failed on temp file\n");
	  return 1;
  }
  ulapi_print("ultest fd stat test passed\n");
  retval = test_fd_stat("nofile");
  if (ULAPI_OK == retval) {
	  ulapi_print("ultest fd stat test failed on no file\n");
	  return 1;
  }

  retval = test_gethostname();
  if (ULAPI_OK != retval) {
    ulapi_print("ultest gethostname test failed\n");
    return 1;
  }
  ulapi_print("ultest gethostname test passed\n");

  retval = test_sxprintf();
  if (ULAPI_OK != retval) {
    ulapi_print("ultest sxprintf test failed\n");
    return 1;
  }
  ulapi_print("ultest sxprintf test passed\n");

  retval = test_time_string();
  if (ULAPI_OK != retval) {
    ulapi_print("ultest time string test failed\n");
    return 1;
  }
  ulapi_print("ultest time string test passed\n");

  ulapi_print("all tests passed\n");

  retval = ulapi_exit();
  return retval;
}
