/*
  outb.c

  Writes a value to an x86 I/O port.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <rtapi.h>
#include <rtapi_app.h>

int main(int argc, char * argv[])
{
  int val;
  int port;

  if (argc < 3 ||
      1 != sscanf(argv[1], "%i", &val) ||
      1 != sscanf(argv[2], "%i", &port)) {
    fprintf(stderr, "usage: outb <val> <port>\n");
    return 1;
  }

  if (RTAPI_OK != rtapi_app_init(argc, argv)) {
    fprintf(stderr, "can't init rtapi\n");
    return 1;
  }
  
  (void) rtapi_outb(val, port);

  return 0;
}
