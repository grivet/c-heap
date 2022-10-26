/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#include <assert.h>
#include <stdio.h>

#include "heap.h"

#include "fibonacci-heap.h"
#include "util.h"

ALLOW_UNDEFINED_BEHAVIOR
static int
min_node_cmp(struct fheap_node *a, struct fheap_node *b)
{
    struct element *e[2] = {
        container_of(a, struct element, fnode),
        container_of(b, struct element, fnode),
    };

    return min_element_cmp(e[0], e[1]);
}

static int
max_node_cmp(struct fheap_node *a, struct fheap_node *b)
{
    return -min_node_cmp(a, b);
}

static void
fibonacci_heap_init(void *heap, void *cmp)
{
    return fheap_init(heap, cmp);
}

static bool
fibonacci_heap_is_empty(void *heap)
{
    return fheap_is_empty(heap);
}

static void
fibonacci_heap_insert(void *heap, struct element *e)
{
    fheap_insert(heap, &e->fnode);
}

ALLOW_UNDEFINED_BEHAVIOR
static struct element *
fibonacci_heap_peek(void *heap)
{
    struct fheap_node *n;

    n = fheap_peek(heap);
    if (n != NULL) {
        return container_of(n, struct element, fnode);
    } else {
        return NULL;
    }
}

ALLOW_UNDEFINED_BEHAVIOR
static struct element *
fibonacci_heap_pop(void *heap)
{
    struct fheap_node *n;

    n = fheap_pop(heap);
    if (n != NULL) {
        return container_of(n, struct element, fnode);
    } else {
        return NULL;
    }
}

static void
fibonacci_heap_update(void *heap, struct element *e, long long int v)
{
    (void) v;
    fheap_update_key(heap, &e->fnode);
}

static void
fibonacci_heap_validate(void *_h)
{
    struct fheap_node *stack[64];
    struct fheap *h = _h;
    int c = -1;

    if (h->root == NULL) {
        return;
    }

    stack[++c] = h->root;
    while (c >= 0) {
        struct fheap_node *n, *s;

//#define PRINT_FIB

        s = stack[c--];
#ifdef PRINT_FIB
        printf("[0x%03lx:%d] ",
                (intptr_t) s->parent & 0xfff, s->parent ? s->parent->rank : 0);
#endif
        FHEAP_NODE_FOREACH_PEER (n, s) {
            struct fheap_node *child;

#ifdef PRINT_FIB
            printf("0x%03lx:%d ", (intptr_t) n & 0xfff, n->rank);
#endif

            assert("Heap invariant not respected between parent / child." &&
                   fheap_prop(h, s, n));

            if (n->child) {
                assert("Heap invariant not respected between parent / child." &&
                       fheap_prop(h, n, n->child));
                stack[++c] = n->child;
            }

            FHEAP_NODE_FOREACH_CHILD (child, n) {
                assert(child->parent == n);
            }

            assert("Fibonacci structure not respected." && c <= 64);
        }
#ifdef PRINT_FIB
        printf("\n");
#endif
    }
}

static struct fheap heap;

struct heap min_fibonacci_heap = {
    .heap = &heap,
    .cmp = min_node_cmp,
    .init = fibonacci_heap_init,
    .is_empty = fibonacci_heap_is_empty,
    .insert = fibonacci_heap_insert,
    .peek = fibonacci_heap_peek,
    .pop = fibonacci_heap_pop,
    .update = fibonacci_heap_update,
    .validate = fibonacci_heap_validate,
    .desc = "min-fibonacci-heap",
};

struct heap max_fibonacci_heap = {
    .heap = &heap,
    .cmp = max_node_cmp,
    .init = fibonacci_heap_init,
    .is_empty = fibonacci_heap_is_empty,
    .insert = fibonacci_heap_insert,
    .peek = fibonacci_heap_peek,
    .pop = fibonacci_heap_pop,
    .update = fibonacci_heap_update,
    .validate = fibonacci_heap_validate,
    .desc = "min-fibonacci-heap",
};
