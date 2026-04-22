#include "dlmall.h"
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>

#define TRUE 1
#define FALSE 0
#define HEAD (sizeof(struct head))
#define MIN(size) (((size) > 8) ? (size) : 8)
#define ALIGN 8
#define ARENA (64 * 1024)

#define HIDE(block) ((void *)((struct head *)block + 1))
#define MAGIC(mem) ((struct head *)mem - 1)

struct head {
    uint16_t bfree;
    uint16_t bsize;
    uint16_t free;
    uint16_t size;
    struct head *next;
    struct head *prev;
};

struct head *arena = NULL;
struct head *flist = NULL;

/* ================= BASIC NAVIGATION ================= */

struct head *after(struct head *block) {
    return (struct head *)((char *)block + HEAD + block->size);
}

struct head *before(struct head *block) {
    return (struct head *)((char *)block - block->bsize - HEAD);
}

/* ================= CREATE ARENA ================= */

struct head *new() {
    if (arena != NULL) return NULL;

    struct head *new = mmap(NULL, ARENA, PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (new == MAP_FAILED) return NULL;

    int size = ARENA - 2 * HEAD;

    new->bfree = FALSE;
    new->bsize = 0;
    new->free = TRUE;
    new->size = size;
    new->next = NULL;
    new->prev = NULL;

    struct head *sentinel = after(new);
    sentinel->bfree = TRUE;
    sentinel->bsize = size;
    sentinel->free = FALSE;
    sentinel->size = 0;

    arena = new;
    flist = new;

    return new;
}

/* ================= FREE LIST ================= */

void insert(struct head *block) {
    block->next = flist;
    block->prev = NULL;
    if (flist != NULL)
        flist->prev = block;
    flist = block;
}

void detach(struct head *block) {
    if (block->next)
        block->next->prev = block->prev;

    if (block->prev)
        block->prev->next = block->next;
    else
        flist = block->next;
}

/* ================= UTIL ================= */

int adjust(int request) {
    int size = ((request + (ALIGN - 1)) / ALIGN) * ALIGN;
    return MIN(size);
}

/* ================= SPLIT ================= */

struct head *split(struct head *block, int size) {
    int remaining = block->size - size - HEAD;

    struct head *newblock = (struct head *)((char *)block + HEAD + size);

    newblock->size = remaining;
    newblock->free = TRUE;
    newblock->bsize = size;
    newblock->bfree = FALSE;

    struct head *aft = after(newblock);
    aft->bsize = remaining;

    block->size = size;

    return newblock;
}

/* ================= FIND ================= */

struct head *find(int size) {
    struct head *curr = flist;

    while (curr != NULL) {
        if (curr->size >= size)
            return curr;
        curr = curr->next;
    }

    if (arena == NULL) new();
    return flist;
}

/* ================= MERGE ================= */

struct head *merge(struct head *block) {
    struct head *aft = after(block);

    /* merge with next */
    if (aft->free) {
        detach(aft);
        block->size += aft->size + HEAD;

        struct head *newaft = after(block);
        newaft->bsize = block->size;
    }

    /* merge with previous */
    if (block->bfree) {
        struct head *prev = before(block);
        detach(prev);

        prev->size += block->size + HEAD;

        struct head *aft2 = after(prev);
        aft2->bsize = prev->size;

        block = prev;
    }

    return block;
}

/* ================= ALLOC ================= */

void *dalloc(size_t request) {
    if (request <= 0) return NULL;

    int size = adjust(request);
    struct head *block = find(size);

    if (block == NULL) return NULL;

    detach(block);

    /* split if large */
    if (block->size >= size + HEAD + 8) {
        struct head *newblock = split(block, size);
        insert(newblock);
    }

    block->free = FALSE;

    struct head *aft = after(block);
    aft->bfree = FALSE;

    return HIDE(block);
}

/* ================= FREE ================= */

void dfree(void *memory) {
    if (memory == NULL) return;

    struct head *block = MAGIC(memory);

    block->free = TRUE;

    block = merge(block);

    struct head *aft = after(block);
    aft->bfree = TRUE;

    insert(block);
}
