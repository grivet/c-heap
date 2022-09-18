/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#ifndef _BINARY_HEAP_H_
#define _BINARY_HEAP_H_

#include <stdbool.h>
#include <stdlib.h>

typedef int (*bheap_cmp)(const void *a, const void *b);

struct bheap {
    bheap_cmp cmp;
    void **entries;
    size_t capacity;
    size_t n;
};

#define BHEAP_INITIALIZER(CMP) { \
    .cmp = CMP, .entries = NULL, \
    .capacity = 0, .n = 0, \
}

static inline void
bheap_ptr_swap(void **a, void **b)
{
    void *tmp = *a;
    *a = *b;
    *b = tmp;
}

static inline bool
bheap_cmp_entries(struct bheap *h, size_t a, size_t b)
{
    return (h->cmp(h->entries[a], h->entries[b]) < 0);
}

static inline void
bheap_up(struct bheap *h, size_t i)
{
    void **e = h->entries;
    size_t parent;

    while ((parent = (i - 1) / 2), i > 0 && bheap_cmp_entries(h, i, parent)) {
        bheap_ptr_swap(&e[i], &e[parent]);
        i = parent;
    }
}

static inline void
bheap_down(struct bheap *h, size_t i, size_t size)
{
    void **e = h->entries;
    size_t next;

    while ((next = (2 * i + 1)), next < size) {
        next += (next + 1 < size && bheap_cmp_entries(h, next + 1, next));
        if (bheap_cmp_entries(h, i, next)) {
            break;
        }
        bheap_ptr_swap(&e[i], &e[next]);
        i = next;
    }
}

static inline bool
bheap_realloc(struct bheap *h, size_t n)
{
    size_t c = h->capacity;
    void *p;

    if (n > c) {
        c = c == 0 ? 1 : 2 * c;
        p = realloc(h->entries, c * sizeof(void *));
        if (p == NULL) {
            return false;
        }
        h->entries = p;
        h->capacity = c;
    }
    return true;
}

static inline void
bheap_init(struct bheap *h, bheap_cmp cmp)
{
    *h = (struct bheap) BHEAP_INITIALIZER(cmp);
}

static inline bool
bheap_is_empty(struct bheap *h)
{
    return h->n == 0;
}

static inline void *
bheap_peek(struct bheap *h)
{
    return bheap_is_empty(h) ? NULL : h->entries[0];
}

static inline void *
bheap_pop(struct bheap *h)
{
    void *top = bheap_peek(h);;

    if (top == NULL) {
        return NULL;
    }

    h->n -= 1;
    bheap_ptr_swap(&h->entries[0], &h->entries[h->n]);
    bheap_down(h, 0, h->n);

    if (h->n == 0) {
        free(h->entries);
        h->entries = NULL;
        h->capacity = 0;
    }

    return top;
}

static inline void
bheap_insert(struct bheap *h, void *entry)
{
    if (!bheap_realloc(h, h->n + 1)) {
        return;
    }

    h->entries[h->n] = entry;
    bheap_up(h, h->n);
    h->n += 1;
}

#endif /* _BINARY_HEAP_H_ */
