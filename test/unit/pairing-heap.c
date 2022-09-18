/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#include "heap.h"
#include "pairing-heap.h"
#include "unit.h"

ALLOW_UNDEFINED_BEHAVIOR
static int
min_node_count_cmp(struct pheap_node *a, struct pheap_node *b)
{
    struct element *e[2] = {
        container_of(a, struct element, hnode),
        container_of(b, struct element, hnode),
    };

    n_cmp_inc();
    return min_element_cmp(e[0], e[1]);
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
pheap_validate(void *_h)
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

struct unit_heap_interface unit_min_pairing_heap = {
    .h = &min_pairing_heap,
    .heap_counting_cmp = min_node_count_cmp,
    .validate = pheap_validate,
};
