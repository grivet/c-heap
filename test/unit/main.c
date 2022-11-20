/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#define _GNU_SOURCE

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "mov-avg.h"
#include "util.h"
#include "heap.h"

#include "unit.h"

static bool verbose;

static struct unit_params params = {
    .n_elems = 1000,
    .seed = 0,
};

static void
usage(const char *program_name, int error)
{
    FILE *s = error ? stderr : stdout;

    fprintf(s, "Usage: %s [-hvs]\n", program_name);
    fprintf(s, "\n");
    fprintf(s, "-v:        Verbose [%s].\n", verbose ? "y" : "n");
    fprintf(s, "-n <uint>: Use n elements [%u].\n", params.n_elems);
    fprintf(s, "-s <uint>: Use given seed [%u].\n", params.seed);
    fprintf(s, "-h:        Show this help.\n");

    exit(error);
}

static int
parse_params(int argc, char * const argv[], struct unit_params *params)
{
    bool do_usage = false;
    int opt;

    while ((opt = getopt(argc, argv, "hn:s:v")) != -1) {
        switch (opt) {
        case 'h':
            do_usage = true;
            break;
        case 'n':
            if (!str_to_uint(optarg, 10, &params->n_elems)) {
               fprintf(stderr, "Failed to parse uint: '%s'\n", optarg);
               return -1;
            }
            break;
        case 's':
            if (!str_to_uint(optarg, 10, &params->seed)) {
               fprintf(stderr, "Failed to parse uint: '%s'\n", optarg);
               return -1;
            }
            break;
        case 'v':
            verbose = true;
            break;
        default:
            usage(argv[0], -1);
            break;
        }
    }

    if (do_usage) {
        usage(argv[0], 0);
    }

    return 0;
}

static void
elements_init_staggered(struct element e[], size_t n)
{
    long long int *values = xmalloc(n * sizeof(values[0]));
    long long int v;
    size_t i;

    for (i = 0; i < n; i++) {
        if (i % 7 == 0) {
            v = random_u32();
        }
        values[i] = v;
    }
    shuffle_lli(values, n);
    for (i = 0; i < n; i++) {
        e[i].priority = values[i];
    }
    free(values);
}

static void
elements_init(struct element e[], size_t n, enum init_mode mode)
{
    size_t i;

    if (mode == STAGGERED) {
        elements_init_staggered(e, n);
        return;
    }

    for (i = 0; i < n; i++) {
        e[i].priority = random_u32();
    }
    n_cmp_enable(false);
    if (mode == INCREASING) {
        qsort(e, n, sizeof e[0], min_element_cmp);
    } else if (mode == DECREASING) {
        qsort(e, n, sizeof e[0], max_element_cmp);
    }
    n_cmp_enable(true);
}

static char *mode2txt[] = {
    [INCREASING] = "increasing",
    [DECREASING] = "decreasing",
    [RANDOM] = "random",
    [STAGGERED] = "staggered",
    [N_MODES] = "<invalid>",
};

static inline int
none_priority_cmp(long long int a, long long int b)
{
    (void) a, b;
    return 0;
}

enum sort_type { EQ, LT, GT };
int (*cmps[])(long long int, long long int) = {
    [EQ] = none_priority_cmp,
    [LT] = min_priority_cmp,
    [GT] = max_priority_cmp,
};

static void
test_basic_insertion(struct unit_test *u)
{
    struct unit_params *p = &u->params;
    enum init_mode mode = p->mode;
    unsigned int n = p->n_elems;
    struct element *elements;
    enum sort_type st = EQ;
    struct heap *h = u->h;
    struct element *top;
    long long int *prios;
    size_t i;

    heap_init(h);

    elements = xcalloc(n, sizeof elements[0]);
    prios = xcalloc(n, sizeof prios[0]);

    n_cmp_reset();
    elements_init(elements, n, mode);
    for (i = 0; i < n; i++) {
        heap_insert(h, &elements[i]);
        heap_validate(h);
    }
    if (verbose) {
        printf("%s insertions[%s]: n-cmp: %u\n",
               h->desc, mode2txt[mode], n_cmp);
    }
    n_cmp_reset();

    i = 0;
    while ((top = heap_pop(h)) != NULL) {
        heap_validate(h);
        prios[i++] = top->priority;
    }

    assert("Unexpected number of removal." && i == n);
    assert("Heap unexpectedly non-empty." && heap_is_empty(h));

    if (verbose) {
        printf("%s removals[%s]: n-cmp: %u\n",
               h->desc, mode2txt[mode], n_cmp);
    }

    for (i = 0; i < n - 1; i++) {
        long long int a = prios[i], b = prios[i + 1];
        /* Learn the expected sorting from the first deletion(s).
         * Enforce that it is always the same once 'learned'. */
        if (st == EQ) {
            switch (cmps[LT](a, b)) {
            case -1: st = LT; break;
            case 1: st = GT; break;
            }
        } else {
            assert("Inconsistent sorting of keys." && cmps[st](a, b) <= 0);
        }
    }

    free(elements);
    free(prios);
}

static void
test_insertion(struct heap *h)
{
    struct unit_test u = UNIT_INITIALIZER;
    enum init_mode mode;

    u.params = (struct unit_params) params;
    u.h = h;

    if (verbose) {
        printf("Running insertion tests on %s with %u elements:\n",
               h->desc, u.params.n_elems);
    }

    for (mode = INCREASING; mode < N_MODES; mode++) {
        u.params.mode = mode;
        test_basic_insertion(&u);
    }
}

static void
test_modify_key_(struct unit_test *u)
{
    struct unit_params *p = &u->params;
    enum init_mode mode = p->mode;
    unsigned int n = p->n_elems;
    struct element *elements;
    enum sort_type st = EQ;
    struct heap *h = u->h;
    struct element *top;
    long long int *prios;
    size_t i;

    heap_init(h);

    elements = xcalloc(n, sizeof elements[0]);
    prios = xcalloc(n, sizeof prios[0]);

    n_cmp_reset();
    elements_init(elements, n, mode);
    for (i = 0; i < n; i++) {
        heap_insert(h, &elements[i]);
        heap_validate(h);
    }
    if (verbose) {
        printf("Modify-key[%s]/insertions: n-cmp: %u\n",
               mode2txt[mode], n_cmp);
    }

    /* Increase one element to max possible value. */
    n_cmp_reset();
    heap_update_key(h, &elements[0], LLONG_MAX);
    heap_validate(h);
    if (verbose) {
        printf("Modify-key[%s]/increase-0: n-cmp: %u\n",
               mode2txt[mode], n_cmp);
    }

    /* Decrease one element to min possible value. */
    n_cmp_reset();
    heap_update_key(h, &elements[n - 1], 0);
    heap_validate(h);
    if (verbose) {
        printf("Modify-key[%s]/decrease-N: n-cmp: %u\n",
               mode2txt[mode], n_cmp);
    }

    /* Decrease one element to min possible value. */
    n_cmp_reset();
    heap_update_key(h, &elements[n / 2], 0);
    heap_validate(h);
    if (verbose) {
        printf("Modify-key[%s]/decrease-half: n-cmp: %u\n",
               mode2txt[mode], n_cmp);
    }

    /* Increase one element to max possible value. */
    n_cmp_reset();
    heap_update_key(h, &elements[n / 2], LLONG_MAX);
    heap_validate(h);
    if (verbose) {
        printf("Modify-key[%s]/increase-half: n-cmp: %u\n",
               mode2txt[mode], n_cmp);
    }

    n_cmp_reset();

    i = 0;
    while ((top = heap_pop(h)) != NULL) {
        heap_validate(h);
        prios[i++] = top->priority;
    }

    assert("Unexpected number of removal." && i == n);
    assert("Heap unexpectedly non-empty." && heap_is_empty(h));

    if (verbose) {
        printf("Modify-key[%s]/removals: n-cmp: %u\n",
               mode2txt[mode], n_cmp);
    }

    for (i = 0; i < n - 1; i++) {
        long long int a = prios[i], b = prios[i + 1];
        /* Learn the expected sorting from the first deletion(s).
         * Enforce that it is always the same once 'learned'. */
        if (st == EQ) {
            switch (cmps[LT](a, b)) {
            case -1: st = LT; break;
            case 1: st = GT; break;
            }
        } else {
            assert("Inconsistent sorting of keys." && cmps[st](a, b) <= 0);
        }
    }

    free(elements);
    free(prios);
}

static void
test_modify_key(struct heap *h)
{
    struct unit_test u = UNIT_INITIALIZER;
    enum init_mode mode;

    u.params = (struct unit_params) params;
    u.h = h;

    if (verbose) {
        printf("Running key update tests on %s with %u elements:\n",
               h->desc, u.params.n_elems);
    }

    for (mode = INCREASING; mode < N_MODES; mode++) {
        u.params.mode = mode;
        test_modify_key_(&u);
    }
}

int main(int argc, char *argv[])
{
    if (parse_params(argc, argv, &params) < 0) {
        usage(argv[0], -1);
    }

    if (params.seed == 0) {
        params.seed = time_usec();
    }
    random_init(params.seed);

    if (verbose) {
        printf("Using seed: %u\n", params.seed);
    }

    test_insertion(&min_pairing_heap);
    test_modify_key(&min_pairing_heap);

    test_insertion(&max_pairing_heap);
    test_modify_key(&max_pairing_heap);

    test_insertion(&min_binary_heap);
    test_modify_key(&min_binary_heap);

    test_insertion(&max_binary_heap);
    test_modify_key(&max_binary_heap);

    test_insertion(&min_fibonacci_heap);
    test_modify_key(&min_fibonacci_heap);

    test_insertion(&max_fibonacci_heap);
    test_modify_key(&max_fibonacci_heap);

    if (verbose) {
        printf("Test succeeded.\n");
    }

    return 0;
}
