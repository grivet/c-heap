/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#include <stdio.h>

#include "heap.h"

#include "binary-heap.h"
#include "util.h"

static int
min_node_cmp(struct bheap_node *a, struct bheap_node *b)
{
    return min_priority_cmp(a->priority.lli, b->priority.lli);
}

static int
max_node_cmp(struct bheap_node *a, struct bheap_node *b)
{
    return -min_node_cmp(a, b);
}

static void
binary_heap_init(void *aux, void *cmp)
{
    return bheap_init(aux, cmp);
}

static bool
binary_heap_is_empty(void *aux)
{
    return bheap_is_empty(aux);
}

static void
binary_heap_insert(void *aux, struct element *e)
{
    struct bheap_node n = {
        .data = e,
        .priority.lli = e->priority,
    };

    bheap_insert(aux, n);
}

static struct element *
binary_heap_peek(void *aux)
{
    return bheap_peek(aux);
}

static struct element *
binary_heap_pop(void *aux)
{
    return bheap_pop(aux);
}

static void
binary_heap_update(void *heap, struct element *e, long long int v)
{
    struct bheap_node k = {
        .data = e,
        .priority.lli = v,
    };

    (void) v;
    bheap_update_key(heap, k);
}

#define LEFT(i) (2 * i + 1)
#define RIGHT(i) (2 * i + 2)
#define PARENT(i) ((i - 1) / 2)

static bool
is_heap(struct bheap_node *h, size_t n, bheap_cmp cmp)
{
    size_t i;

    for (i = 0; i < n; i++) {
        if (LEFT(i) >= n)
            break;
        if (cmp(&h[i], &h[LEFT(i)]) > 0) {
            struct element *e[2] = { h[i].data, h[LEFT(i)].data };

            printf("size=%zu, failing on "
                   "node %zu (%lld) > (L) %zu (%lld)\n",
                   n, i, e[0]->priority, LEFT(i), e[1]->priority);
            abort();
        }
        if (RIGHT(i) >= n)
            break;
        if (cmp(&h[i], &h[RIGHT(i)]) > 0) {
            struct element *e[2] = { h[i].data, h[RIGHT(i)].data };

            printf("size=%zu, failing on "
                   "node %zu (%lld) > (R) %zu (%lld)\n",
                   n, i, e[0]->priority, LEFT(i), e[1]->priority);
            abort();
        }
    }
    return true;
}

void
binary_heap_validate(void *_h)
{
    struct bheap *h = _h;

    is_heap(h->entries, h->n, h->cmp);
}

static struct bheap heap;

struct heap min_binary_heap = {
    .heap = &heap,
    .cmp = min_node_cmp,
    .init = binary_heap_init,
    .is_empty = binary_heap_is_empty,
    .insert = binary_heap_insert,
    .peek = binary_heap_peek,
    .pop = binary_heap_pop,
    .update = binary_heap_update,
    .validate = binary_heap_validate,
    .desc = "min-binary-heap",
};

struct heap max_binary_heap = {
    .heap = &heap,
    .cmp = max_node_cmp,
    .init = binary_heap_init,
    .is_empty = binary_heap_is_empty,
    .insert = binary_heap_insert,
    .peek = binary_heap_peek,
    .pop = binary_heap_pop,
    .update = binary_heap_update,
    .validate = binary_heap_validate,
    .desc = "max-binary-heap",
};
