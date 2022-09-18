/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#include "heap.h"

#include "pairing-heap.h"
#include "util.h"

ALLOW_UNDEFINED_BEHAVIOR
static int
min_node_cmp(struct pheap_node *a, struct pheap_node *b)
{
    struct element *e[2] = {
        container_of(a, struct element, hnode),
        container_of(b, struct element, hnode),
    };

    return min_element_cmp(e[0], e[1]);
}

static int
max_node_cmp(struct pheap_node *a, struct pheap_node *b)
{
    return -min_node_cmp(a, b);
}

static void
pairing_heap_init(void *heap, void *cmp)
{
    return pheap_init(heap, cmp);
}

static bool
pairing_heap_is_empty(void *heap)
{
    return pheap_is_empty(heap);
}

static void
pairing_heap_insert(void *heap, struct element *e)
{
    pheap_insert(heap, &e->hnode);
}

ALLOW_UNDEFINED_BEHAVIOR
static struct element *
pairing_heap_peek(void *heap)
{
    struct pheap_node *pnode;

    pnode = pheap_peek(heap);
    if (pnode != NULL) {
        return container_of(pnode, struct element, hnode);
    } else {
        return NULL;
    }
}

ALLOW_UNDEFINED_BEHAVIOR
static struct element *
pairing_heap_pop(void *heap)
{
    struct pheap_node *pnode;

    pnode = pheap_pop(heap);
    if (pnode != NULL) {
        return container_of(pnode, struct element, hnode);
    } else {
        return NULL;
    }
}

static void
pairing_heap_update(void *heap, struct element *e, long long int v)
{
    e->expiration = v;
    pheap_reinsert(heap, &e->hnode);
}

static struct pheap heap;

struct heap_interface min_pairing_heap = {
    .heap = &heap,
    .heap_cmp_fn = min_node_cmp,
    .desc = "pairing-heap",
    .init = pairing_heap_init,
    .is_empty = pairing_heap_is_empty,
    .insert = pairing_heap_insert,
    .peek = pairing_heap_peek,
    .pop = pairing_heap_pop,
    .update = pairing_heap_update,
};

struct heap_interface max_pairing_heap = {
    .heap = &heap,
    .heap_cmp_fn = max_node_cmp,
    .desc = "pairing-heap",
    .init = pairing_heap_init,
    .is_empty = pairing_heap_is_empty,
    .insert = pairing_heap_insert,
    .peek = pairing_heap_peek,
    .pop = pairing_heap_pop,
    .update = pairing_heap_update,
};
