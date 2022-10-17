/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#include "heap.h"

#include "binary-heap.h"
#include "util.h"

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

static struct bheap heap;

struct heap_interface min_binary_heap = {
    .heap = &heap,
    .heap_cmp_fn = bheap_min_cmp_lli,
    .desc = "binary-heap",
    .init = binary_heap_init,
    .is_empty = binary_heap_is_empty,
    .insert = binary_heap_insert,
    .peek = binary_heap_peek,
    .pop = binary_heap_pop,
    .update = NULL,
};

struct heap_interface max_binary_heap = {
    .heap = &heap,
    .heap_cmp_fn = bheap_max_cmp_lli,
    .desc = "binary-heap",
    .init = binary_heap_init,
    .is_empty = binary_heap_is_empty,
    .insert = binary_heap_insert,
    .peek = binary_heap_peek,
    .pop = binary_heap_pop,
    .update = NULL,
};
