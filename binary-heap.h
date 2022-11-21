/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#ifndef _BINARY_HEAP_H_
#define _BINARY_HEAP_H_

#include <stdbool.h>
#include <stdlib.h>

struct bheap_node {
    void *data;
    union {
        uint32_t u32;
        long long int lli;
    } priority;
    bool invalid;
};

typedef int (*bheap_cmp)(struct bheap_node *a, struct bheap_node *b);

struct bheap {
    bheap_cmp cmp;
    struct bheap_node *entries;
    size_t capacity;
    size_t n;
};

#define BHEAP_INITIALIZER(CMP) { \
    .cmp = CMP, .entries = NULL, \
    .capacity = 0, .n = 0, \
}

static inline void
bheap_swap(struct bheap *h, size_t i, size_t j)
{
    if (i != j) {
        struct bheap_node tmp = h->entries[i];

        h->entries[i] = h->entries[j];
        h->entries[j] = tmp;
    }
}

static inline bool
bheap_cmp_entries(struct bheap *h, size_t a, size_t b)
{
    return (h->cmp(&h->entries[a], &h->entries[b]) < 0);
}

static inline void
bheap_up(struct bheap *h, size_t i)
{
    size_t parent;

    while ((parent = (i - 1) / 2), i > 0 &&
           bheap_cmp_entries(h, i, parent)) {
        bheap_swap(h, i, parent);
        i = parent;
    }
}

static inline void
bheap_down(struct bheap *h, size_t i, size_t size)
{
    size_t next;

    while ((next = (2 * i + 1)), next < size) {
        next += (next + 1 < size && bheap_cmp_entries(h, next + 1, next));
        if (bheap_cmp_entries(h, i, next)) {
            break;
        }
        bheap_swap(h, i, next);
        i = next;
    }
}

static inline bool
bheap_realloc(struct bheap *h, size_t n)
{
    size_t c = h->capacity;
    void *p;

    if (n > c) {
        size_t delta = (1.75 * c) - c;
        c += delta ? delta : 1;
        p = realloc(h->entries, c * sizeof(h->entries[0]));
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

static inline void
bheap_pop_(struct bheap *h)
{
    if (bheap_is_empty(h)) {
        return;
    }

    h->n -= 1;
    bheap_swap(h, 0, h->n);
    bheap_down(h, 0, h->n);

    if (h->n == 0) {
        free(h->entries);
        h->entries = NULL;
        h->capacity = 0;
    }
}

static inline void *
bheap_peek(struct bheap *h)
{
    while (!bheap_is_empty(h) &&
           h->entries[0].invalid) {
        bheap_pop_(h);
    }

    return bheap_is_empty(h) ? NULL : h->entries[0].data;
}

static inline void *
bheap_pop(struct bheap *h)
{
    void *top = bheap_peek(h);;

    if (top == NULL) {
        return NULL;
    }

    bheap_pop_(h);
    return top;
}

static inline void
bheap_insert(struct bheap *h, struct bheap_node n)
{
    if (!bheap_realloc(h, h->n + 1)) {
        return;
    }

    n.invalid = false;
    h->entries[h->n] = n;
    bheap_up(h, h->n);
    h->n += 1;
}

static inline void
bheap_update_key(struct bheap *h, struct bheap_node new_key)
{
    void *data = new_key.data;

    for (size_t i = 0; i < h->n; i++) {
        if (h->entries[i].data == data &&
            h->entries[i].invalid == false) {
            h->entries[i].invalid = true;
            break;
        }
    }
    bheap_insert(h, new_key);
}

#endif /* _BINARY_HEAP_H_ */
