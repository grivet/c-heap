/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#include <stddef.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#include "heap.h"
#include "binary-heap.h"
#include "unit.h"

static inline int
min_element_count_cmp(const void *a, const void *b)
{
    n_cmp_inc();
    return min_element_cmp(a, b);
}

#define LEFT(i) (2 * i + 1)
#define RIGHT(i) (2 * i + 2)
#define PARENT(i) ((i - 1) / 2)

static bool
is_heap(void **h, size_t n, bheap_cmp cmp)
{
    size_t i;

    for (i = 0; i < n; i++) {
        if (LEFT(i) >= n)
            break;
        if (cmp(h[i], h[LEFT(i)]) > 0) {
            struct element *e[2] = { h[i], h[LEFT(i)] };

            printf("size=%zu, failing on "
                   "node %zu (%lld) > (L) %zu (%lld)\n",
                   n, i, e[0]->priority, LEFT(i), e[1]->priority);
            abort();
        }
        if (RIGHT(i) >= n)
            break;
        if (cmp(h[i], h[RIGHT(i)]) > 0) {
            struct element *e[2] = { h[i], h[RIGHT(i)] };
            printf("size=%zu, failing on "
                   "node %zu (%lld) > (R) %zu (%lld)\n",
                   n, i, e[0]->priority, LEFT(i), e[1]->priority);
            abort();
        }
    }
    return true;
}

void
bheap_validate(void *_h)
{
    struct bheap *h = _h;

    is_heap(h->entries, h->n, h->cmp);
}

struct unit_heap_interface unit_min_binary_heap = {
    .h = &min_binary_heap,
    .heap_counting_cmp = min_element_count_cmp,
    .validate = bheap_validate,
};
