#include <math.h>
#include <rtapi.h>
#include <rtapi_app.h>

static void task_code(void *arg)
{
  rtapi_integer period_nsec;
  int count = 0;
  double sum = 0;
  rtapi_integer start_secs, start_nsecs;
  rtapi_integer now_secs, now_nsecs;
  rtapi_integer diff_secs, diff_nsecs;

  period_nsec = *((rtapi_integer *) arg);
  
  rtapi_print("starting task with period nsecs %d\n", (int) period_nsec);

  rtapi_clock_get_time(&start_secs, &start_nsecs);
  
  for (;;) {
    count++;
    sum += sin(count);
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
  rtapi_task_struct task;
  rtapi_result retval;
  rtapi_integer period_nsec = PERIOD_NSEC;

  rtapi_app_init(argc, argv);

  rtapi_task_init(&task);

  retval = rtapi_task_start(&task,
			    task_code,
			    &period_nsec,
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
