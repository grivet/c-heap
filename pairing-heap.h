/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#ifndef _PAIRING_HEAP_H_
#define _PAIRING_HEAP_H_

#include <stdbool.h>
#include <stdlib.h>

/* Pairing heap.
 *
 * This heap is similar to fibonacci heaps but simpler, smaller, and faster.
 * It can be used to implement priority queues of arbitrary order.
 *
 * This is an intrusive implementation, meaning that to insert an element
 * it must contain a 'pheap_node' which is the size of three pointers.
 *
 * To avoid replicating data (and possibly creating synchronization issues),
 * comparison between two nodes is made by comparing their containing type,
 * which must be orderable.
 *
 * No memory is allocated during heap operations.
 */

struct pheap_node {
    union {
        struct pheap_node *parent;
        struct pheap_node *prev;
    };
    struct pheap_node *next;
    struct pheap_node *child;
};

#define PHEAP_NODE_INITIALIZER { \
    .prev = NULL, .next = NULL, \
    .child = NULL, \
}

typedef int (*pheap_cmp)(struct pheap_node *a, struct pheap_node *b);

struct pheap {
    struct pheap_node *root;
    pheap_cmp cmp;
};

#define PHEAP_INITIALIZER(CMP) { \
    .root = NULL, .cmp = CMP, \
}

/* Pairing heap API. */

/* Initialize a heap. Must be called first unless
 * the static PHEAP_INITIALIZER has been used. */
static inline void pheap_init(struct pheap *h, pheap_cmp cmp);

/* Returns 'true' if the heap is empty. */
static inline bool pheap_is_empty(struct pheap *h);

/* Returns the top element of the heap, defined as
 * the min or max one depending on the comparison function
 * used. */
static inline struct pheap_node *pheap_peek(struct pheap *h);

/* Removes the top element from the heap. */
static inline struct pheap_node *pheap_pop(struct pheap *h);

/* Insert an element in the heap.
 * The containing type of 'node' must be comparable against
 * other elements, i.e. calling 'pheap_cmp' on it should be possible. */
static inline void pheap_insert(struct pheap *h, struct pheap_node *node);

/* Merge two pairing heaps into 'dst'. The second one 'src' becomes
 * empty. The comparison functions of both heaps must not only be
 * compatible, but also be **the same function**, otherwise the merging
 * is not done. */
static inline void pheap_merge(struct pheap *dst, struct pheap *src);

/* After modifying the value of a containing element, it must
 * be re-ordered in the heap. Use this function to re-insert it
 * at its proper place.
 * The key update can be done in either direction (increase or decrease).
 *
 * Some optimizations could be done depending on the comparison function
 * and the change type, but this function is generic and will work
 * for both, albeit slightly slower in case of:
 *
 *  - increasing low priorities if this is a min-heap
 *  - decreasing high priorities if this is a max-heap
 *
 */
static inline void pheap_reinsert(struct pheap *h, struct pheap_node *n);

/* Pairing-heap node utility functions. */

#define PHEAP_NODE_FOREACH_CHILD(i, n) \
    for (i = (n)->child; i != NULL; i = (i)->next)

static inline void
pheap_node_init(struct pheap_node *n)
{
    *n = (struct pheap_node) PHEAP_NODE_INITIALIZER;
}

static inline void
pheap_node_add_child(struct pheap_node *head, struct pheap_node *n)
{
    struct pheap_node *child = head->child;

    n->parent = head;
    n->next = child;
    if (child != NULL) {
        child->parent = NULL;
        child->prev = n;
    }
    head->child = n;
}

static inline struct pheap_node *
pheap_node_merge(struct pheap_node *a, struct pheap_node *b, pheap_cmp cmp)
{
    if (a == NULL) {
        return b;
    } else if (b == NULL) {
        return a;
    }

    if (cmp(a, b) < 0) {
        pheap_node_add_child(a, b);
        return a;
    }

    pheap_node_add_child(b, a);
    return b;
}

/* Remove n from its list of siblings.
 * Must not be called on the root node. */
static inline void
pheap_node_unlink(struct pheap_node *n)
{
    if (n == n->parent->child) {
        struct pheap_node *parent = n->parent;

        parent->child = n->next;
        if (n->next != NULL) {
            n->next->prev = NULL;
            n->next->parent = parent;
        }
    } else {
        if (n->prev != NULL) {
            n->prev->next = n->next;
        }
        if (n->next != NULL) {
            n->next->prev = n->prev;
        }
    }
    n->next = n->prev = NULL;
}

static inline struct pheap_node *
pheap_node_pairwise_merge(struct pheap_node *n, pheap_cmp cmp)
{
    struct pheap_node *next, *root;
    struct pheap_node *a, *b;

    if (n == NULL) {
        return NULL;
    }

    if (n->next == NULL) {
        return n;
    }

    root = NULL;

    a = n;
    while (a != NULL) {
        b = a->next;
        pheap_node_unlink(a);
        if (b != NULL) {
            next = b->next;
            pheap_node_unlink(b);
        } else {
            next = NULL;
        }
        root = pheap_node_merge(root,
                                pheap_node_merge(a, b, cmp),
                                cmp);
        a = next;
    }

    return root;
}

/* Pairing-heap implementation. */

static inline void
pheap_init(struct pheap *h, pheap_cmp cmp)
{
    *h = (struct pheap) PHEAP_INITIALIZER(cmp);
}

static inline bool
pheap_is_empty(struct pheap *h)
{
    return h->root == NULL;
}

static inline struct pheap_node *
pheap_peek(struct pheap *h)
{
    return pheap_is_empty(h) ? NULL : h->root;
}

static inline struct pheap_node *
pheap_pop(struct pheap *h)
{
    struct pheap_node *top = pheap_peek(h);

    if (top != NULL) {
        h->root = pheap_node_pairwise_merge(top->child, h->cmp);
        pheap_node_init(top);
    }
    return top;
}

static inline void
pheap_insert(struct pheap *h, struct pheap_node *node)
{
    /* Assume the node was never user before. */
    pheap_node_init(node);
    h->root = pheap_node_merge(h->root, node, h->cmp);
}

static inline void
pheap_merge(struct pheap *dst, struct pheap *src)
{
    if (dst->cmp == src->cmp) {
        dst->root = pheap_node_merge(dst->root, src->root, dst->cmp);
        src->root = NULL;
    }
}

/* After increase/decrease-key, the node of the modified
 * element must be 'reinserted' into the heap to repair
 * a potential invariant violation. */
static inline void
pheap_reinsert(struct pheap *h, struct pheap_node *n)
{
    struct pheap new_heap = PHEAP_INITIALIZER(h->cmp);

    /* Remove 'n' from 'h'. */
    if (n == pheap_peek(h)) {
        new_heap.root = pheap_pop(h);
    } else {
        struct pheap_node *child;

        pheap_node_unlink(n);
        child = pheap_node_pairwise_merge(n->child, h->cmp);
        n->child = NULL;
        new_heap.root = pheap_node_merge(n, child, h->cmp);
    }

    /* Insert 'n' back into 'h'. */
    pheap_merge(h, &new_heap);
}

#endif /* _PAIRING_HEAP_H_ */
