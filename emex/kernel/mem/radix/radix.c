/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: radix.c
 * CREATED BY: mimalloc
 * MODIFIED BY: --
 *
 */

#include "radix.h"
#include <kernel/kernel.h>

radix_tree_t *radix_alloc()
{
    radix_tree_t *tree = (radix_tree_t*)kmalloc(sizeof(radix_tree_t));
    tree->root = NULL;
    return tree;
}

void radix_free(radix_tree_t *tree)
{
    kfree((u64*)tree);
}

static inline int radix_chunk(uint64_t ident,
                              int level)
{
    int shift = (RADIX_LEVELS - 1 - level) * RADIX_BITS;
    return(ident >> shift) & RADIX_MASK;
}

void *radix_lookup(radix_tree_t *tree,
                   uint64_t ident)
{
    radix_node_t *node = tree->root;

    for(int level = 0; level < RADIX_LEVELS - 1; level++)
    {
        if(node == NULL)
        {
            return NULL;
        }

        int chunk = radix_chunk(ident, level);
        node = (radix_node_t *)node->slots[chunk];
    }

    if(node == NULL)
    {
        return NULL;
    }

    int chunk = radix_chunk(ident, RADIX_LEVELS - 1);
    return node->slots[chunk];
}

int radix_insert(radix_tree_t *tree,
                 uint64_t ident,
                 void *value)
{
    if(tree->root == NULL)
    {
        tree->root = (radix_node_t*)kcalloc(sizeof(radix_node_t), 1);
    }

    radix_node_t *node = tree->root;

    for(int level = 0; level < RADIX_LEVELS - 1; level++)
    {
        int chunk = radix_chunk(ident, level);

        if(node->slots[chunk] == NULL)
        {
            node->slots[chunk] = kcalloc(sizeof(radix_node_t), 1);
        }

        node = (radix_node_t *)node->slots[chunk];
    }

    int chunk = radix_chunk(ident, RADIX_LEVELS - 1);
    node->slots[chunk] = value;

    return 0;
}

void *radix_remove(radix_tree_t *tree,
                   uint64_t ident)
{
    radix_node_t *node = tree->root;
    radix_node_t *path[RADIX_LEVELS];
    int chunks[RADIX_LEVELS];

    for(int level = 0; level < RADIX_LEVELS; level++)
    {
        if(node == NULL)
        {
            return NULL;
        }

        path[level] = node;
        chunks[level] = radix_chunk(ident, level);

        if(level < RADIX_LEVELS - 1)
        {
            node = (radix_node_t *)node->slots[chunks[level]];
        }
    }

    void *old = path[RADIX_LEVELS - 1]->slots[chunks[RADIX_LEVELS - 1]];
    path[RADIX_LEVELS - 1]->slots[chunks[RADIX_LEVELS - 1]] = NULL;

    for(int level = RADIX_LEVELS - 2; level >= 0; level--)
    {
        u8 all_null = true;
        for(int i = 0; i < RADIX_SIZE; i++)
        {
            if(path[level]->slots[i] != NULL)
            {
                all_null = false;
                break;
            }
        }
        if(all_null && level > 0)
        {
            kfree((u64*)path[level]);
            path[level - 1]->slots[chunks[level - 1]] = NULL;
        }
        else
        {
            break;
        }
    }

    return old;
}

static void radix_walk_node(radix_node_t *node,
                            int level,
                            uint64_t ident_prefix,
                            radix_walk_fn callback,
                            void *ctx)
{
    if(node == NULL)
    {
        return;
    }

    for(int i = 0; i < RADIX_SIZE; i++)
    {
        if(node->slots[i] == NULL)
        {
            continue;
        }

        uint64_t ident = ident_prefix | (i << ((RADIX_LEVELS - 1 - level) * RADIX_BITS));

        if(level == RADIX_LEVELS - 1)
        {
            callback(ident, node->slots[i], ctx);
        }
        else
        {
            radix_walk_node((radix_node_t *)node->slots[i], level + 1, ident, callback, ctx);
        }
    }
}

void radix_walk(radix_tree_t *tree, radix_walk_fn callback, void *ctx)
{
    radix_walk_node(tree->root, 0, 0, callback, ctx);
}