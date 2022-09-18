/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>

/*
 * This file describes a generic heap interface,
 * to be instanciated using any of the available implementations.
 */

/* Necessary for an intrusive type. */
#include "pairing-heap.h"

struct element {
    union {
        struct {
            struct pheap_node hnode;
            union {
                long long int expiration;
                long long int priority;
            };
        };
        /* Take a whole cacheline per elements. */
        uint8_t pad[64];
    };
};

static inline int
min_element_cmp(const void *_a, const void *_b)
{
    const struct element *e[2] = { _a, _b };
    uint32_t a = e[0]->priority, b = e[1]->priority;

    /* Compare without risk of underflow. */
    return (a < b) ? -1 :
           (a > b) ? 1 :
           0;
}

static inline int
max_element_cmp(const void *a, const void *b)
{
    return -min_element_cmp(a, b);
}

typedef void (*heap_init_fn)(void *heap, void *cmp);
typedef bool (*heap_is_empty_fn)(void *heap);
typedef void (*heap_insert_fn)(void *heap, struct element *e);
typedef struct element * (*heap_peek_fn)(void *heap);
typedef struct element * (*heap_pop_fn)(void *heap);
typedef void (*heap_update_fn)(void *heap, struct element *e, long long int v);

struct heap_interface {
    void *heap;
    void *heap_cmp_fn;
    const char *desc;
    heap_init_fn init;
    heap_is_empty_fn is_empty;
    heap_insert_fn insert;
    heap_peek_fn peek;
    heap_pop_fn pop;
    heap_update_fn update;
};

extern struct heap_interface min_pairing_heap;
extern struct heap_interface min_binary_heap;

extern struct heap_interface max_pairing_heap;
extern struct heap_interface max_binary_heap;

#endif /* HEAP_H */
