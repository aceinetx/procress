/*
const std = @import("std");
const c = @cImport(@cInclude("stdlib.h"));

pub fn main(init: std.process.Init) !void {
    const root_node = std.Progress.start(init.io, .{
        .root_name = "preparing assets",
        .estimated_total_items = 100,
    });
    defer root_node.end();

    const sub_node = root_node.start("reticulating splines", 100);
    defer sub_node.end();

    for (0..100) |_| {
        sub_node.completeOne();
        root_node.completeOne();
        _ = c.system("sleep 0.1");
    }
}
 */

#include "procress.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>

// #region sequence

void procressSequence(ProcressSeqState state, int progress) {
  printf("\x1b]9;4;%d;%d\a", state, progress);
  fflush(stdout);
}

struct ProcressNode {
  mtx_t mutex;
  size_t items;
  size_t estimated_items;
  char *name;
  struct ProcressNode *parent;
};

// #endregion

// #region ProcressNode

ProcressNode *ProcressNode_start(ProcressNode *node, char *name) {
  if (node == NULL) {
    ProcressNode *node = calloc(1, sizeof *node);

    mtx_init(&node->mutex, mtx_recursive);
    ProcressNode_setName(node, name);
    return node;
  }
  abort();
}

void ProcressNode_setName(ProcressNode *node, char *name) {
  mtx_lock(&node->mutex);

  if (node->name) {
    free(node->name);
    node->name = NULL;
  }

  if (name) {
    size_t len = strlen(name) + 1;
    node->name = malloc(len);
    memcpy(node->name, name, len);
  }

  mtx_unlock(&node->mutex);
}

void ProcressNode_setEstimatedItems(ProcressNode *node, size_t items) {
  mtx_lock(&node->mutex);

  node->estimated_items = items;

  mtx_unlock(&node->mutex);
}

static void ProcressNode_print(ProcressNode *node) {
  assert(0 && "not implemented");
}

void ProcressNode_advance(ProcressNode *node, int times) {
  mtx_lock(&node->mutex);

  node->items += times;
  if (node->estimated_items != 0) {
    float progress = (float)node->items / (float)node->estimated_items * 100.0f;
    procressSequence(PROCRESS_SEQ_STATE_NORMAL, (int)progress);
  } else {
    procressSequence(PROCRESS_SEQ_STATE_INTERMEDIATE, 0);
  }

  mtx_unlock(&node->mutex);
}

void ProcressNode_end(ProcressNode *node) {
  mtx_lock(&node->mutex);

  if (!node->parent)
    procressSequence(PROCRESS_SEQ_STATE_STOP, 0);

  ProcressNode_setName(node, NULL);

  mtx_unlock(&node->mutex);

  mtx_destroy(&node->mutex);
  free(node);
}
// #endregion
