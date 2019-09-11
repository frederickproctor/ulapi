#include <rtapi.h>

#define KEY 101
#define BUFFERSIZE 256

int main(int argc, char *argv[])
{
  void *shm;
  char *addr;

  shm = rtapi_shm_new(KEY, BUFFERSIZE);
  if (NULL == shm) {
    printf("can't get shared memory\n");
    return 1;
  }

  addr = rtapi_shm_addr(shm);

  printf("%d .. %d\n", (int) addr[0], (int) addr[BUFFERSIZE-1]);
  
  return 0;
}
