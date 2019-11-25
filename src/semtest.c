/*!
  \file semtest.c

  \brief Test program for ulapi semaphores.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stddef.h>
#include <signal.h>
#include "ulapi.h"		/* these decls */

#define SEM_KEY 12322

/*
  Syntax: semtest <arg>

  With any arg, is the master, otherwise is the slave.

  Master takes, prints, waits a second, gives, waits a second.

  Slave takes, prints, gives, takes, etc. with no waits.

  Slave should run really fast until there's a master.
*/

static int done = 0;
static void quit(int sig)
{
  done = 1;
}

int main(int argc, char *argv[])
{
  int master;
  ulapi_semaphore_struct *sem;

  if (argc > 1) master = 1;
  else master = 0;

  if (ULAPI_OK != ulapi_init()) {
    fprintf(stderr, "can't init ulapi\n");
    return 1;
  }

  sem = ulapi_semaphore_new(SEM_KEY);

  if (NULL == sem) {
    fprintf(stderr, "can't create semaphore\n");
    return 1;
  }

  signal(SIGINT, quit);

  while (! done) {
    if (master) {
      ulapi_semaphore_give(sem);
      printf("gave it\n");
      ulapi_sleep(1);
    } else {
      printf("trying it\n");
      ulapi_semaphore_take(sem);
      printf("got it\n");
    }
  }

  ulapi_semaphore_delete(sem);

  return 0;
}
