/*
  This code demonstrates that you need to share mutexes by pointer,
  and that copies of the mutex struct won't share the same mutex.

  With COPY_MUTEX undefined, the thread shares a pointer to the main
  process' mutex, and the execution proceeds correctly, with this output:

  give
  take
  ...
  give
  take
  ...

  With COPY_MUTEX defined, the thread makes a copy of the main process'
  mutex, which is a different mutex. Execution proceeds incorrectly, with
  only one 'take' by the thread. The copied mutex is never given, so the
  thread is perpetually blocked. This is the output in that case:

  give
  take
  ...
  give
  give
  give
  ...

  Always share mutexes by pointers, never copy them!
*/

#include <stdio.h>
#include "ulapi.h"

#undef COPY_MUTEX

void task_code(void *args)
{
#ifdef COPY_MUTEX
  ulapi_mutex_struct mutex = *((ulapi_mutex_struct *) args);
#else
  ulapi_mutex_struct *mutex = ((ulapi_mutex_struct *) args);
#endif
  
  for (;;) {
#ifdef COPY_MUTEX
    ulapi_mutex_take(&mutex);
#else
    ulapi_mutex_take(mutex);
#endif
    printf("take\n");
  }
}

int main(int argc, char *argv[])
{
  ulapi_task_struct task;
  ulapi_mutex_struct mutex;
  
  ulapi_init();

  ulapi_task_init(&task);
  ulapi_mutex_init(&mutex, 1);

  ulapi_task_start(&task, task_code, &mutex, ulapi_prio_lowest(), 0);

  for (;;) {
    ulapi_mutex_give(&mutex);
    printf("give\n");
    ulapi_sleep(1);
  }

  return 0;
}
