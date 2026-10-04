#define _DEFAULT_SOURCE
#include "procress.h"
#include <stdlib.h>
#include <threads.h>
#include <unistd.h>

#define THREAD_COUNT 8

ProcressNode *root_node;

int threadA(void *_) {
  ProcressNode *node = ProcressNode_start(root_node, "Progress");
  ProcressNode_setEstimatedItems(node, 5);

  for (int i = 0; i < 5; i++) {
    usleep(10000 * (rand() % 50));
    ProcressNode_advance(node, 1);
  }

  ProcressNode_advance(root_node, 1);

  ProcressNode_end(node);

  return 0;
}

int main(void) {
  procressGlobalInit();

  srand(time(NULL));

  root_node = ProcressNode_start(NULL, "qwe");
  ProcressNode_setEstimatedItems(root_node, THREAD_COUNT);

  thrd_t threads[THREAD_COUNT];

  for (int i = 0; i < THREAD_COUNT; i++) {
    thrd_t thread;
    thrd_create(&thread, threadA, NULL);
    threads[i] = thread;
  }

  for (int i = 0; i < THREAD_COUNT; i++) {
    thrd_join(threads[i], NULL);
  }

  ProcressNode_end(root_node);

  procressGlobalDeinit();
}
