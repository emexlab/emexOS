/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (c) 2026 emex-foundation
 *
 * FILE: tmpfs.c
 * CREATED BY: emex
 * MODIFIED BY: Offihito
 *
 */

#include "../vfs.h"
#include <kernel/mem/lib/main.h>
#include <kernel/graph/lib/string.h>
#include <kernel/mem/klime/klime.h>

static int tmpfs_open(fs_node *node, fs_file *file) {
    {
        (void)node;(void)file;
    }
    return 0;
}
static int tmpfs_close(fs_file *file) {(void)file; return 0;}


static ssize_t tmpfs_read(fs_file *file, void *buf, size_t cnt) {
    fs_node *node = file->node;
    if (node->type != FS_FILE) return -1;

    tmpfs_data *data = (tmpfs_data*)node->priv;
    if (!data || !data->data) return 0;

    if (file->pos >= node->size) return 0;

    // calc bytes to read
    size_t to_read = cnt;
    if (file->pos + to_read > node->size) {
        to_read = node->size - file->pos;
    }

    memcpy(buf, (u8*)data->data + file->pos, to_read);
    file->pos += to_read;

    return to_read;
}

static ssize_t tmpfs_write(fs_file *file, const void *buf, size_t cnt) {
    fs_node *node = file->node;
    if (node->type != FS_FILE) return -1;

    tmpfs_data *data = (tmpfs_data*)node->priv;
    if (!data) {
        // alloc data struct
        data = (tmpfs_data*)klime_create((klime_t*)fs_klime, sizeof(tmpfs_data));
        if (!data) return -1;
        data->data = NULL;
        data->cap  = 0;
        data->ro   = 0;
        node->priv = data;
    }

    // promote read-only alias to writable copy on first write
    if (data->ro) {
        u64 old_size = node->size;
        u64 new_cap  = (old_size + cnt + 4095) & ~(u64)4095;
        void *copy   = klime_create((klime_t*)fs_klime, new_cap);
        if (!copy) return -1;
        if (data->data && old_size > 0)
            memcpy(copy, data->data, old_size);
        data->data = copy;
        data->cap  = new_cap;
        data->ro   = 0;
    }

    // expand if needed
    u64 needed = file->pos + cnt;
    if (needed > data->cap) {
        u64 new_cap = (needed + 4095) & ~(u64)4095;

        void *new_data = klime_create((klime_t*)fs_klime, new_cap);
        if (!new_data) return -1;

        // copy old data
        if (data->data && node->size > 0) {
            memcpy(new_data, data->data, node->size);
            klime_free((klime_t*)fs_klime, (u64*)data->data);
        }

        data->data = new_data;
        data->cap  = new_cap;
    }

    memcpy((u8*)data->data + file->pos, buf, cnt);
    file->pos += cnt;

    if (file->pos > node->size) {
        node->size = file->pos;
    }

    return cnt;
}

static fs_node* tmpfs_lookup(fs_node *dir, const char *name) {
    if (dir->type != FS_DIR) return NULL;

    // search children
    fs_node *child = dir->children;
    while (child) {
        if (str_equals(child->name, name)) {
            return child;
        }
        child = child->next;
    }

    return NULL;
}

static int tmpfs_create(fs_node *dir, const char *name) {
    if (dir->type != FS_DIR) return -1;
    if (tmpfs_lookup(dir, name)) return -1; // already exists

    fs_node *node = fs_mknode(name, FS_FILE);
    if (!node) return -1;

    node->ops = dir->ops;
    fs_addchild(dir, node);

    return 0;
}

static int tmpfs_mkdir(fs_node *dir, const char *name) {
    if (dir->type != FS_DIR) return -1;
    if (tmpfs_lookup(dir, name)) return -1;

    fs_node *node = fs_mknode(name, FS_DIR);
    if (!node) return -1;

    node->ops = dir->ops;
    fs_addchild(dir, node);

    return 0;
}

static int tmpfs_unlink(fs_node *dir, const char *name) {
    if (dir->type != FS_DIR) return -1;

    fs_node *prev = NULL;
    fs_node *child = dir->children;
    while (child) {
        if (str_equals(child->name, name)) {
            if (prev) prev->next = child->next;
            else dir->children = child->next;
            return 0;
        }
        prev = child;
        child = child->next;
    }

    return -1;
}

static fs_ops tmpfs_ops = {
    .open   = tmpfs_open,
    .close  = tmpfs_close,
    .read   = tmpfs_read,
    .write  = tmpfs_write,
    .lookup = tmpfs_lookup,
    .create = tmpfs_create,
    .mkdir  = tmpfs_mkdir,
    .unlink = tmpfs_unlink,
};

static int tmpfs_mount(const char *src, const char *tgt, fs_mnt *mnt) {
    {
        (void)src;(void)tgt;
    }

    fs_node *root = fs_mknode("/", FS_DIR);
    if (!root) return -1;

    root->ops = &tmpfs_ops;
    mnt->root = root;

    return 0;
}

static fs_type tmpfs = {
    .name = "tmpfs",
    .mount = tmpfs_mount,
    .ops = &tmpfs_ops,
};

/*
 * Zero-copy loader: wire a VFS node directly to an existing read-only
 * memory region (e.g. the cpio image still mapped by Limine).  No heap
 * allocation is made for the file data itself.
 */
int tmpfs_set_ro_data(fs_node *node, const void *ptr, u64 size)
{
    if (!node || !ptr) return -1;

    tmpfs_data *data = (tmpfs_data*)node->priv;
    if (!data) {
        data = (tmpfs_data*)klime_create((klime_t*)fs_klime, sizeof(tmpfs_data));
        if (!data) return -1;
        node->priv = data;
    }

    data->data = (void *)ptr;
    data->cap  = size;
    data->ro   = 1;
    node->size = size;

    return 0;
}

// register tmpfs type directly
void tmpfs_register(void) {
    fs_register(&tmpfs);
}
