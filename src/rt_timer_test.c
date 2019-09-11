#include <math.h>
#include <rtapi.h>
#include <rtapi_app.h>

#define KEY 101
#define BUFFERSIZE 256

typedef struct {
  rtapi_integer period_nsec;
  char *addr;
  size_t size;
} args_struct;

static void task_code(void *arg)
{
  rtapi_integer period_nsec;
  char *addr;
  size_t size;
  int count = 0;
  double sum = 0;
  rtapi_integer start_secs, start_nsecs;
  rtapi_integer now_secs, now_nsecs;
  rtapi_integer diff_secs, diff_nsecs;

  period_nsec = ((args_struct *) arg)->period_nsec;
  addr = ((args_struct *) arg)->addr;
  size = ((args_struct *) arg)->size;
  
  rtapi_print("starting task with period nsecs %d, buffer size %d\n", (int) period_nsec, (int) size);

  rtapi_clock_get_time(&start_secs, &start_nsecs);
  
  for (;;) {
    count++;
    sum += sin(count);
    addr[0] = addr[size-1] = count;
    rtapi_clock_get_time(&now_secs, &now_nsecs);
    rtapi_clock_get_interval(start_secs, start_nsecs,
			     now_secs, now_nsecs,
			     &diff_secs, &diff_nsecs);
    if (diff_secs > 10) break;
    rtapi_wait(period_nsec);
  }

  rtapi_print("task done, count is %d, sum is %f\n", count, sum);

  exit(0);
}

#define PERIOD_NSEC 10000

int rtapi_app_main(int argc, char **argv)
{
  args_struct args;
  rtapi_task_struct task;
  rtapi_result retval;
  rtapi_integer period_nsec = PERIOD_NSEC;
  void *rtm;

  rtapi_app_init(argc, argv);

  rtapi_task_init(&task);

  rtm = rtapi_rtm_new(KEY, BUFFERSIZE);
  if (NULL == rtm) {
    rtapi_print("can't get rt memory\n");
    return 1;
  }
  
  args.period_nsec = period_nsec;
  args.addr = rtapi_rtm_addr(rtm);
  args.size = BUFFERSIZE;
  
  retval = rtapi_task_start(&task,
			    task_code,
			    &args,
			    rtapi_prio_highest(),
			    0,
			    period_nsec,
			    1);

  if (RTAPI_OK != retval) {
    rtapi_print("can't start task\n");
    return 1;
  }

  rtapi_app_wait();
  
  return 0;
}
