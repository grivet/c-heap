/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#define _GNU_SOURCE

#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#include "mov-avg.h"
#include "util.h"
#include "heap.h"

#include "bench.h"

static struct test_params params = {
    .name = NULL,
    .n_elems = 1000000,
    .range = 24 * 60 * 60 * 1000,
    .seed = 0,
    .p_update = 0,
};

static void
usage(const char *program_name, int error)
{
    FILE *s = error ? stderr : stdout;

    fprintf(s, "Usage: %s [-hnrs]\n", program_name);
    fprintf(s, "\n");
    fprintf(s, "Run performance tests on heap operations using different implementations,\n");
    fprintf(s, "agains the pairing heap reference.\n");
    fprintf(s, "\n");
    fprintf(s, "-n <uint>:   Number of elements to sift through [n=%u].\n", params.n_elems);
    fprintf(s, "-r <uint>:   Range of elements priorities [r=%u].\n", params.range);
    fprintf(s, "-s <uint>:   Use given seed [s=%u].\n", params.seed);
    fprintf(s, "-h           Show this help.\n");

    exit(error);
}

static int
parse_params(int argc, char * const argv[], struct test_params *params)
{
    int opt;

    while ((opt = getopt(argc, argv, "hn:r:s:")) != -1) {
        switch (opt) {
        case 'n':
            if (!str_to_uint(optarg, 10, &params->n_elems)) {
               fprintf(stderr, "Failed to parse uint: '%s'\n", optarg);
               return -1;
            }
            break;
        case 'r':
            if (!str_to_uint(optarg, 10, &params->range)) {
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
        case 'h':
            usage(argv[0], 0);
            break;
        default:
            usage(argv[0], -1);
            break;
        }
    }

   return 0;
}

static long long int time_delta;
static long long int
clock_read(void)
{
    long long int now = time_msec();

    if (LLONG_MAX - now > time_delta) {
        return now + time_delta;
    }
    return LLONG_MAX;
}

static void
clock_drift(long long int delta)
{
    if (LLONG_MAX - time_delta > delta) {
        time_delta += delta;
    } else {
        time_delta = LLONG_MAX;
    }
}

#define NAME_LEN 15

static void
test_column_print(void)
{
    printf("%*s (P-u%%): %*s %*s %*s%*s (ms)\n",
            NAME_LEN, "Queue type",
            10, "insert",
            10, "delete",
            5, " ",
            10, "sweep-avg(stdev)");
}

static void
test_print(struct test *t)
{
    struct test_results *r = &t->results;
    struct test_params *p = &t->params;

    printf("%*s (%03u%%):", NAME_LEN, t->h->desc, p->p_update);
    printf(" %10lld", r->times.insertion - r->times.start);
    printf(" %10lld", r->times.end - r->times.insertion);
    printf("%*s", 5, " ");
    printf(" %9.1lf(%.1lf)", mov_avg_cma(&r->cma),
                             mov_avg_cma_std_dev(&r->cma));
    printf("\n");
}

static void
test_execute(struct test *t)
{
    struct test_results *r = &t->results;
    struct test_params *p = &t->params;
    long long int now = clock_read();
    struct heap_interface *h = t->h;
    struct element *elems;
    void *heap = h->heap;
    long long int delta;
    unsigned int limit;
    unsigned int i;

    /* Sweep through 'limit' elements at once, each
     * 'delta' time increments. */
    limit = MAX(100, p->n_elems / 10);
    delta = MAX(1, p->range / 10);

    elems = xcalloc(p->n_elems, sizeof elems[0]);

    r->delta = delta;
    r->sweep_limit = limit;
    for (i = 0; i < p->n_elems; i++) {
        elems[i].expiration = now + random_u32_range(p->range);
    }

    h->init(heap, h->heap_cmp_fn);

    /* Test results are not using the fake internal time, but actual
     * monotonic clock. */
    r->times.start = time_msec();
    for (i = 0; i < p->n_elems; i++) {
        h->insert(heap, &elems[i]);
        elems[i].inserted = true;
    }

    r->times.insertion = time_msec();
    while (!h->is_empty(heap)) {
        long long int sweep_start_ms;
        unsigned int count = 0;
        struct element *e;

        sweep_start_ms = time_msec();
        while (count < limit) {
            e = h->pop(heap);
            if (e == NULL) {
                break;
            }
            /* Half of random updates happening on oldest element,
             * other half within the heap at any point. */
            if (random_u32_range(100) < (p->p_update / 2)) {
                e->expiration += p->range;
                h->insert(heap, e);
            } else {
                while (e->expiration > clock_read()) {
                    clock_drift(delta);
                }
                e->inserted = false;
                count++;
            }
        }
        /* Re-assign the other half of the random update. */
        for (i = 0; i < p->n_elems; i++) {
            if (elems[i].inserted &&
                random_u32_range(100) < (p->p_update / 2)) {
                h->update(heap, &elems[i],
                          elems[i].expiration + p->range);
            }
        }
        mov_avg_cma_update(&r->cma, time_msec() - sweep_start_ms);
    }
    r->times.end = time_msec();

    free(elems);
    return;
}

void
test_run(struct heap_interface *h)
{
    struct test t = TEST_INITIALIZER;

    t.params = (struct test_params) params;
    t.h = h;

    t.params.p_update = 0;
    test_execute(&t);
    test_print(&t);

    t.params.p_update = 10;
    test_execute(&t);
    test_print(&t);

    t.params.p_update = 30;
    test_execute(&t);
    test_print(&t);
}

int main(int argc, char * const argv[])
{
    if (parse_params(argc, argv, &params) < 0) {
        usage(argv[0], -1);
    }

    if (params.seed == 0) {
        params.seed = time_usec();
    }
    random_init(params.seed);

    test_column_print();
    test_run(&min_pairing_heap);
    test_run(&min_binary_heap);

    return 0;
}
