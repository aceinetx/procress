#ifndef __PROCRESS_H__
#define __PROCRESS_H__
#include <stddef.h>

// #region sequence

typedef enum ProcressSeqState {
  PROCRESS_SEQ_STATE_STOP = 0,
  PROCRESS_SEQ_STATE_NORMAL = 1,
  PROCRESS_SEQ_STATE_ERROR = 2,
  PROCRESS_SEQ_STATE_INTERMEDIATE = 3,
  PROCRESS_SEQ_STATE_PAUSED = 4,
} ProcressSeqState;

void procressSequence(ProcressSeqState state, int progress);

// #endregion

// #region ProcressNode

typedef struct ProcressNode ProcressNode;

ProcressNode *ProcressNode_start(ProcressNode *node, char *name);
void ProcressNode_setName(ProcressNode *node, char *name);
void ProcressNode_setEstimatedItems(ProcressNode *node, size_t items);
size_t ProcressNode_getEstimatedItems(ProcressNode *node);
void ProcressNode_advance(ProcressNode *node, int times);
size_t ProcressNode_getItems(ProcressNode *node);
void ProcressNode_end(ProcressNode *node);

// #endregion

void procressGlobalInit();
void procressGlobalDeinit();

#endif
