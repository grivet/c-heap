/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#ifndef _FIBONACCI_HEAP_H_
#define _FIBONACCI_HEAP_H_

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Fibonacci heap.
 *
 * This heap has the best possible complexity bounds,
 * but is not practical. It has the exact same requirements /
 * usage as the pairing heap, but its node are larger and
 * its operations slightly slower in absolute terms.
 *
 * This implementation is provided mostly for comparison
 * and as a study.
 *
 * No allocation is made during any heap operations.
 */

struct fheap_node {
    /* Navigate the current heap level. */
    struct fheap_node *prev;
    struct fheap_node *next;
    /* Change the heap level. */
    struct fheap_node *parent;
    struct fheap_node *child;
    /* Metadata. */
    int rank;
    bool mark;
};

#define FHEAP_NODE_INITIALIZER(N) { \
    .prev = NULL, .next = NULL, \
    .parent = NULL, .child = NULL, \
    .rank = 0, .mark = false, \
}

typedef int (*fheap_cmp)(struct fheap_node *a, struct fheap_node *b);

struct fheap {
    struct fheap_node *root;
    fheap_cmp cmp;
};

#define FHEAP_INITIALIZER(CMP) { \
    .root = NULL, .cmp = CMP, \
}

/* Fibonacci heap API. */

static inline void fheap_init(struct fheap *h, fheap_cmp cmp);
static inline bool fheap_is_empty(struct fheap *h);
static inline struct fheap_node *fheap_peek(struct fheap *h);
static inline struct fheap_node *fheap_pop(struct fheap *h);
static inline void fheap_insert(struct fheap *h, struct fheap_node *node);
static inline void fheap_merge(struct fheap *dst, struct fheap *src);
static inline void fheap_update_key(struct fheap *h, struct fheap_node *n);

/* Fibonacci-heap node utility functions. */

#define FHEAP_NODE_FOREACH_PEER(node, start) \
    for (struct fheap_node *__it = start, \
                           *__next = __it ? __it->next : NULL; \
         node = __it, __it != NULL; \
         __it = __next, __next = __it ? __it->next : NULL)

#define FHEAP_NODE_FOREACH_CHILD(node, parent) \
    FHEAP_NODE_FOREACH_PEER (node, parent->child)

static inline bool
fheap_prop(struct fheap *h, struct fheap_node *a, struct fheap_node *b)
{
    /* Return true if the heap property is respected considering 'a' and 'b',
     * meaning that 'a' could be parent of 'b'. */
    return (h->cmp(a, b) <= 0);
}

static inline void
fheap_node_init(struct fheap_node *n)
{
    *n = (struct fheap_node) FHEAP_NODE_INITIALIZER(n);
}

static inline struct fheap_node *
fheap_node_link(struct fheap_node *a, struct fheap_node *b)
{
    if (a->next) {
        a->next->prev = b;
        b->next = a->next;
    }

    a->next = b;
    b->prev = a;

    return a;
}

static inline struct fheap_node *
fheap_node_add_peer(struct fheap *h, struct fheap_node *a, struct fheap_node *b)
{
    if (a == NULL) {
        return b;
    }
    if (b == NULL) {
        return a;
    }

    if (fheap_prop(h, a, b)) {
        return fheap_node_link(a, b);
    } else {
        return fheap_node_link(b, a);
    }
}

static inline struct fheap_node *
fheap_node_add_child(struct fheap *h, struct fheap_node *p, struct fheap_node *n)
{
    p->child = fheap_node_add_peer(h, p->child, n);
    n->parent = p;
    p->rank++;
    return p;
}

/* Remove node 'n' from its list of siblings. */
static inline void
fheap_node_level_cut(struct fheap_node *n)
{
    if (!n) {
        return;
    }

    if (n->parent) {
        n->parent->rank--;
        if (n->parent->child == n) {
            n->parent->child = n->next;
        }
        n->parent = NULL;
    }
    if (n->next) {
        n->next->prev = n->prev;
    }
    if (n->prev) {
        n->prev->next = n->next;
    }

    n->next = n->prev = NULL;
}

/* Remove all references to the parent node of all siblings. of 'n'. */
static inline void
fheap_node_level_orphan(struct fheap_node *n)
{
    if (!n) {
        return;
    }

    if (n->parent) {
        n->parent->rank = 0;
        n->parent->child = NULL;
    }

    do {
        n->parent = NULL;
        n = n->next;
    } while (n);
}

static inline struct fheap_node *
fheap_node_level_merge(struct fheap *h,
                       struct fheap_node *l1, struct fheap_node *l2)
{
    struct fheap_node *end;

    if (l1 == NULL) {
        return l2;
    }

    if (l2 == NULL) {
        return l1;
    }

    if (fheap_prop(h, l2, l1)) {
        struct fheap_node *tmp = l1;

        l1 = l2;
        l2 = tmp;
    }

    end = l1;
    for (end = l1; end && end->next; end = end->next);
    if (end) {
        end->next = l2;
        l2->prev = end;
    }

    return l1;
}

static inline void
fheap_node_cut(struct fheap *h, struct fheap_node *n)
{
    fheap_node_level_cut(n);
    n->mark = false;
    h->root = fheap_node_add_peer(h, h->root, n);
}

static inline void
fheap_node_cascade(struct fheap *h, struct fheap_node *n)
{
    while (n && n->parent) {
        if (n->mark == false) {
            n->mark = true;
            break;
        }
        fheap_node_cut(h, n);
        n = n->parent;
    }
}

/* Fibonacci-heap user-interface: */

static inline void
fheap_init(struct fheap *h, fheap_cmp cmp)
{
    *h = (struct fheap) FHEAP_INITIALIZER(cmp);
}

static inline bool
fheap_is_empty(struct fheap *h)
{
    return h->root == NULL;
}

static inline struct fheap_node *
fheap_peek(struct fheap *h)
{
    return fheap_is_empty(h) ? NULL : h->root;
}

static inline void
fheap_consolidate(struct fheap *h)
{
    const int n_ranks = 64;
    struct fheap_node *ranks[n_ranks];
    struct fheap_node *n;
    size_t max_rank = 0;

    if (h->root == NULL) {
        return;
    }

    memset(ranks, 0, sizeof ranks);

    FHEAP_NODE_FOREACH_PEER (n, h->root) {
        size_t r = n->rank;

        fheap_node_level_cut(n);
        while (ranks[r] != NULL) {
            if (fheap_prop(h, n, ranks[r])) {
                fheap_node_add_child(h, n, ranks[r]);
            } else {
                fheap_node_add_child(h, ranks[r], n);
                n = ranks[r];
            }
            ranks[r++] = NULL;
        }
        ranks[r] = n;
        if (max_rank <= r) {
            max_rank = r + 1;
        }
    }
    h->root = NULL;

    for (size_t i = 0; i < max_rank; i++) {
        h->root = fheap_node_add_peer(h, h->root, ranks[i]);
    }
}

static inline struct fheap_node *
fheap_pop(struct fheap *h)
{
    struct fheap_node *next, *child;
    struct fheap_node *root;

    root = fheap_peek(h);
    if (root == NULL) {
        return NULL;
    }

    /* Take references before cutting the root. */
    child = root->child;
    next = root->next;

    /* Isolate the root from its peers. */
    fheap_node_level_cut(root);
    fheap_node_level_orphan(child);

    /* Put the children of root in the first level. */
    h->root = fheap_node_level_merge(h, next, child);

    fheap_consolidate(h);

    return root;
}

static inline void
fheap_insert(struct fheap *h, struct fheap_node *node)
{
    fheap_node_init(node);
    h->root = fheap_node_add_peer(h, h->root, node);
}

static inline void
fheap_merge(struct fheap *dst, struct fheap *src)
{
    if (dst->cmp == src->cmp) {
        dst->root = fheap_node_level_merge(dst, dst->root, src->root);
    }
}

static inline void
fheap_reinsert(struct fheap *h, struct fheap_node *n)
{
    struct fheap new_heap = FHEAP_INITIALIZER(h->cmp);

    if (n == fheap_peek(h)) {
        new_heap.root = fheap_pop(h);
    } else {
        struct fheap_node *child = n->child;

        fheap_node_level_cut(n);
        fheap_node_level_orphan(child);
        new_heap.root = fheap_node_level_merge(h, n, child);
    }

    fheap_merge(h, &new_heap);
}

static inline void
fheap_update_key(struct fheap *h, struct fheap_node *n)
{
    struct fheap_node *c;

    if (n->parent) {
        if (!fheap_prop(h, n->parent, n)) {
            struct fheap_node *p = n->parent;

            fheap_node_cut(h, n);
            fheap_node_cascade(h, p);
            return;
        }
    } else if (h->root == n ||
               !fheap_prop(h, h->root, n)) {
        fheap_reinsert(h, n);
        return;
    }

    FHEAP_NODE_FOREACH_CHILD (c, n) {
        if (!fheap_prop(h, n, c)) {
            fheap_reinsert(h, n);
            return;
        }
    }
}

#endif /* _FIBONACCI_HEAP_H_ */
