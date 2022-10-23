/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stdbool.h>

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
            bool inserted;
        };
        /* Take a whole cacheline per elements. */
        uint8_t pad[64];
    };
};

extern unsigned int n_cmp;

void n_cmp_inc(void);
void n_cmp_enable(bool enabled);
void n_cmp_reset(void);

static inline int
min_priority_cmp(long long int a, long long int b)
{
    n_cmp_inc();
    /* Compare without risk of underflow. */
    return (a < b) ? -1 :
           (a > b) ? 1 :
           0;
}

static inline int
max_priority_cmp(long long int a, long long int b)
{
    return -min_priority_cmp(a, b);
}

static inline int
min_element_cmp(const void *_a, const void *_b)
{
    const struct element *e[2] = { _a, _b };
    return min_priority_cmp(e[0]->priority, e[1]->priority);
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
typedef void (*heap_validate_fn)(void *heap);

struct heap {
    void *heap;
    void *cmp;
    heap_init_fn init;
    heap_is_empty_fn is_empty;
    heap_insert_fn insert;
    heap_peek_fn peek;
    heap_pop_fn pop;
    heap_update_fn update;
    heap_validate_fn validate;
    const char *desc;
};

void heap_init(struct heap *h);
bool heap_is_empty(struct heap *h);
void heap_insert(struct heap *h, struct element *e);
struct element *heap_peek(struct heap *h);
struct element *heap_pop(struct heap *h);
void heap_update_key(struct heap *h, struct element *e, long long int v);
void heap_validate(struct heap *h);

extern struct heap min_pairing_heap;
extern struct heap min_binary_heap;

extern struct heap max_pairing_heap;
extern struct heap max_binary_heap;

#endif /* HEAP_H */
