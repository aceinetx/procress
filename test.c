#include "procress.h"
#include <stdlib.h>

int main(void) {
  ProcressNode *root_node = ProcressNode_start(NULL, "abc");
  ProcressNode_setEstimatedItems(root_node, 10);

  for (int i = 0; i < 10; i++) {
    ProcressNode_advance(root_node, 1);
    system("sleep 0.1");
  }

  ProcressNode_end(root_node);
}
