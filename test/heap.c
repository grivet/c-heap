/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#include "heap.h"

unsigned int n_cmp;
static bool cmp_count_enabled;

void
n_cmp_inc(void)
{
    if (cmp_count_enabled) {
        n_cmp++;
    }
}

void
n_cmp_reset(void)
{
    n_cmp = 0;
}

void
n_cmp_enable(bool enabled)
{
    cmp_count_enabled = enabled;
}

void
heap_init(struct heap *h)
{
    h->init(h->heap, h->cmp);
}

bool
heap_is_empty(struct heap *h)
{
    return h->is_empty(h->heap);
}

void
heap_insert(struct heap *h, struct element *e)
{
    h->insert(h->heap, e);
    e->inserted = true;
}

struct element *
heap_peek(struct heap *h)
{
    return h->peek(h->heap);
}

struct element *
heap_pop(struct heap *h)
{
    struct element *e = h->pop(h->heap);

    if (e != NULL) {
        e->inserted = false;
    }
    return e;
}

void
heap_update_key(struct heap *h, struct element *e, long long int v)
{
    e->priority = v;
    h->update(h->heap, e, v);
}

void
heap_validate(struct heap *h)
{
    n_cmp_enable(false);
    h->validate(h->heap);
    n_cmp_enable(true);
}
