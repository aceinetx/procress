#define _GNU_SOURCE
#include "procress.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <threads.h>
#include <unistd.h>

// #region sequence

void procressSequence(ProcressSeqState state, int progress) {
  printf("\x1b]9;4;%d;%d\a", state, progress);
  fflush(stdout);
}

// #endregion

// #region ProcressNode

struct ProcressNode {
  mtx_t mutex;
  atomic_size_t items;
  atomic_size_t estimated_items;
  char *name;
  struct ProcressNode *parent;
  size_t children_count;
  struct ProcressNode **children;
};

static void ProcessNode_addChild(ProcressNode *node, ProcressNode *child) {
  assert(node != child);
  mtx_lock(&node->mutex);

  child->parent = node;

  for (size_t i = 0; i < node->children_count; i++) {
    if (node->children[i] == NULL) {
      node->children[i] = child;
      mtx_unlock(&node->mutex);
      return;
    }
  }

  node->children = realloc(node->children,
                           sizeof *node->children * (node->children_count + 1));
  node->children[node->children_count++] = child;
  mtx_unlock(&node->mutex);
}

static bool ProcessNode_removeChild(ProcressNode *node, ProcressNode *child) {
  assert(node != child);
  mtx_lock(&node->mutex);

  for (size_t i = 0; i < node->children_count; i++) {
    if (node->children[i] == child) {
      mtx_lock(&child->mutex);
      child->parent = NULL;
      mtx_unlock(&child->mutex);

      node->children[i] = NULL;

      mtx_unlock(&node->mutex);
      return true;
    }
  }

  mtx_unlock(&node->mutex);
  return false;
}

static void ProcressNode_printSequence(ProcressNode *node) {
  if (node->parent)
    return;

  if (node->estimated_items != 0) {
    float progress = (float)node->items / (float)node->estimated_items * 100.0f;
    procressSequence(PROCRESS_SEQ_STATE_NORMAL, (int)progress);
  } else {
    procressSequence(PROCRESS_SEQ_STATE_INTERMEDIATE, 0);
  }
}

ProcressNode *ProcressNode_start(ProcressNode *parent, char *name) {
  if (parent == NULL) {
    ProcressNode *node = calloc(1, sizeof *node);

    mtx_init(&node->mutex, mtx_recursive);
    ProcressNode_setName(node, name);

    ProcressNode_printSequence(node);

    return node;
  } else {
    ProcressNode *node = calloc(1, sizeof *node);

    mtx_init(&node->mutex, mtx_recursive);
    ProcressNode_setName(node, name);

    ProcessNode_addChild(parent, node);
    return node;
  }
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
  node->estimated_items += items;

  ProcressNode_printSequence(node);
}

size_t ProcressNode_getEstimatedItems(ProcressNode *node) {
  return node->estimated_items;
}

static int ProcressNode_print(ProcressNode *node, FILE *fp, int indent) {
  bool is_root = node->parent == NULL;

  if (is_root) {
    fprintf(fp, "\x1b[2K");
    fputc(10, fp);
  }

  fprintf(fp, "\x1b[2K");

  for (int i = 0; i < indent; i++)
    fputc('\t', fp);

  if (!is_root) {
    fprintf(fp, "- ");
  }
  bool has_estimated = node->estimated_items != 0;
  if (has_estimated) {
    fprintf(fp, "[%zu/%zu] ", node->items, node->estimated_items);
  } else {
    fprintf(fp, "[%zu] ", node->items);
  }
  fprintf(fp, "%s\n", node->name);

  int lines = 1;

  mtx_lock(&node->mutex);

  for (size_t i = 0; i < node->children_count; i++) {
    if (node->children[i]) {
      lines += ProcressNode_print(node->children[i], fp, indent + 1);
    }
  }

  mtx_unlock(&node->mutex);

  if (is_root) {
    fprintf(fp, "\x1b[2K");
    fprintf(fp, "\x1b[%dF", lines + 1);
    fflush(fp);
  }

  return lines;
}

static FILE *displayfp;
static mtx_t displayfp_mtx;

void ProcressNode_advance(ProcressNode *node, int times) {
  node->items += times;

  ProcressNode_printSequence(node);

  mtx_lock(&node->mutex);
  ProcressNode *n = node;
  while (n->parent)
    n = n->parent;
  mtx_unlock(&node->mutex);

  mtx_lock(&displayfp_mtx);

  static char buf[0x1000];
  displayfp = fmemopen(buf, sizeof buf, "w");

  ProcressNode_print(n, displayfp, 0);

  write(STDOUT_FILENO, buf, ftell(displayfp));

  fclose(displayfp);

  mtx_unlock(&displayfp_mtx);
}

size_t ProcressNode_getItems(ProcressNode *node) { return node->items; }

void ProcressNode_end(ProcressNode *node) {
  mtx_lock(&node->mutex);

  for (size_t i = 0; i < node->children_count; i++) {
    if (node->children[i])
      ProcessNode_removeChild(node, node->children[i]);
  }

  if (!node->parent) {
    procressSequence(PROCRESS_SEQ_STATE_STOP, 0);
  } else {
    ProcessNode_removeChild(node->parent, node);
  }

  ProcressNode_setName(node, NULL);
  free(node->children);

  mtx_unlock(&node->mutex);

  mtx_destroy(&node->mutex);
  free(node);
}
// #endregion

void procressGlobalInit() { mtx_init(&displayfp_mtx, mtx_plain); }
void procressGlobalDeinit() { mtx_destroy(&displayfp_mtx); }
