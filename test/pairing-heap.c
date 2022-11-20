/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#include <assert.h>

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
    (void) v;
    pheap_reinsert(heap, &e->hnode);
}

static void
pheap_node_validate(struct pheap_node *parent, struct pheap_node *n,
                    pheap_cmp cmp)
{
    struct pheap_node *c;

    assert("Only the root node has no precedent." && n->prev != NULL);
    assert("Heap invariant not respected between parent / child." &&
            cmp(parent, n) <= 0);
    PHEAP_NODE_FOREACH_CHILD(c, n) {
        pheap_node_validate(n, c, cmp);
    }
}

static void
pairing_heap_validate(void *_h)
{
    struct pheap *h = _h;
    struct pheap_node *c;

    if (!pheap_is_empty(h)) {
        assert("Root node should have no siblings." && h->root->next == NULL);
        PHEAP_NODE_FOREACH_CHILD(c, h->root) {
            pheap_node_validate(h->root, c, h->cmp);
        }
    }
}

static struct pheap heap;

struct heap min_pairing_heap = {
    .heap = &heap,
    .cmp = min_node_cmp,
    .init = pairing_heap_init,
    .is_empty = pairing_heap_is_empty,
    .insert = pairing_heap_insert,
    .peek = pairing_heap_peek,
    .pop = pairing_heap_pop,
    .update = pairing_heap_update,
    .validate = pairing_heap_validate,
    .desc = "min-pairing-heap",
};

struct heap max_pairing_heap = {
    .heap = &heap,
    .cmp = max_node_cmp,
    .init = pairing_heap_init,
    .is_empty = pairing_heap_is_empty,
    .insert = pairing_heap_insert,
    .peek = pairing_heap_peek,
    .pop = pairing_heap_pop,
    .update = pairing_heap_update,
    .validate = pairing_heap_validate,
    .desc = "max-pairing-heap",
};
