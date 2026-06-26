/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: radix.h
 * CREATED BY: mimalloc
 * MODIFIED BY: --
 *
 */

#ifndef RADIX_H
#define RADIX_H

#include <types.h>

#define RADIX_BITS   8
#define RADIX_SIZE   256
#define RADIX_MASK   0xFF
#define RADIX_LEVELS 8

typedef struct radix_node {
    u64 *slots[RADIX_SIZE];
} radix_node_t;

typedef struct radix_tree {
    radix_node_t *root;
} radix_tree_t;

typedef void (*radix_walk_fn)(uint64_t ident, void *value, void *ctx);

radix_tree_t *radix_alloc();
void radix_free(radix_tree_t *tree);

void *radix_lookup(radix_tree_t *tree, uint64_t ident);
int radix_insert(radix_tree_t *tree, uint64_t ident, void *value);
void *radix_remove(radix_tree_t *tree, uint64_t ident);
void radix_walk(radix_tree_t *tree, radix_walk_fn callback, void *ctx);

#endif /* RADIX_H */