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
static unsigned int n_cmp;
static bool cmp_count_enabled;

static struct unit_params params = {
    .n_elems = 1000,
    .seed = 0,
};

void
n_cmp_inc()
{
    if (cmp_count_enabled) {
        n_cmp++;
    }
}

static void
n_cmp_reset()
{
    n_cmp = 0;
}

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
    uint32_t *values = xmalloc(n * sizeof(values[0]));
    uint32_t v;
    size_t i;

    for (i = 0; i < n; i++) {
        if (i % 7 == 0) {
            v = random_u32();
        }
        values[i] = v;
    }
    shuffle_u32(values, n);
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
    if (mode == INCREASING) {
        qsort(e, n, sizeof e[0], min_element_cmp);
    } else if (mode == DECREASING) {
        qsort(e, n, sizeof e[0], max_element_cmp);
    }
}

static char *mode2txt[] = {
    [INCREASING] = "increasing",
    [DECREASING] = "decreasing",
    [RANDOM] = "random",
    [STAGGERED] = "staggered",
    [N_MODES] = "<invalid>",
};

#if 0
static void
print_heap_size(struct pheap *h, struct mov_avg_cma *cma)
{
    printf("heap(%u): [d:%u, w:%u] avg-n-cmp:%.2lf(%.2lf)\n",
            pheap_length(h), pheap_depth(h), pheap_width(h),
            mov_avg_cma(cma), mov_avg_cma_std_dev(cma));
}

#define N_ELEMS 100

static void
test_pheap_modify_key(enum init_mode mode)
{
    struct pheap heap = PHEAP_INITIALIZER(element_node_cmp_inc);
    struct element elements[N_ELEMS];
    struct mov_avg_cma cma;
    uint32_t prios[N_ELEMS];
    struct pheap_node *top;
    size_t i;

    mov_avg_cma_init(&cma);
    elements_init(elements, N_ELEMS, mode);
    for (i = 0; i < N_ELEMS; i++) {
        n_cmp = 0;
        pheap_insert(&heap, &elements[i].hnode);
        mov_avg_cma_update(&cma, n_cmp);
        pheap_validate(&heap);
    }

    printf("Modify-key[%s]/init-state: ", mode2txt[mode]);
    print_heap_size(&heap, &cma);
    mov_avg_cma_init(&cma);
    n_cmp = 0;

    /* Increase one element to max possible value. */
    elements[0].priority = UINT32_MAX;
    pheap_reinsert(&heap, &elements[0].hnode);
    mov_avg_cma_update(&cma, n_cmp);
    pheap_validate(&heap);

    printf("Modify-key[%s]/increase-0: ", mode2txt[mode]);
    print_heap_size(&heap, &cma);
    mov_avg_cma_init(&cma);
    n_cmp = 0;

    /* Decrease one element to min possible value. */
    elements[N_ELEMS - 1].priority = 0;
    pheap_reinsert(&heap, &elements[N_ELEMS - 1].hnode);
    mov_avg_cma_update(&cma, n_cmp);
    pheap_validate(&heap);

    printf("Modify-key[%s]/decrease-N: ", mode2txt[mode]);
    print_heap_size(&heap, &cma);
    mov_avg_cma_init(&cma);
    n_cmp = 0;

    /* Decrease one element to min possible value. */
    elements[N_ELEMS / 2].priority = 0;
    pheap_reinsert(&heap, &elements[N_ELEMS / 2].hnode);
    mov_avg_cma_update(&cma, n_cmp);
    pheap_validate(&heap);

    printf("Modify-key[%s]/decrease-Half: ", mode2txt[mode]);
    print_heap_size(&heap, &cma);
    mov_avg_cma_init(&cma);
    n_cmp = 0;

    /* Increase one element to min possible value. */
    elements[N_ELEMS / 2].priority = UINT32_MAX;
    pheap_reinsert(&heap, &elements[N_ELEMS / 2].hnode);
    mov_avg_cma_update(&cma, n_cmp);
    pheap_validate(&heap);

    printf("Modify-key[%s]/increase-Half: ", mode2txt[mode]);
    print_heap_size(&heap, &cma);
    mov_avg_cma_init(&cma);
    n_cmp = 0;

    i = 0;
    while ((top = pheap_pop(&heap)) != NULL) {
        struct element *e = container_of(top, struct element, node);
        pheap_validate(&heap);
        prios[i++] = e->priority;
    }
    assert("Unexpected number of removal." && i == N_ELEMS);
    assert("Heap unexpectedly non-empty." && pheap_is_empty(&heap));

    for (i = 0; i < N_ELEMS - 1; i++) {
        assert("Incorrect sorting of keys." && prios[i] <= prios[i + 1]);
    }
}

#endif

static void
unit_heap_init(struct unit_heap_interface *uh)
{
    uh->h->init(uh->h->heap, uh->heap_counting_cmp);
}

static void
unit_heap_validate(struct unit_heap_interface *uh)
{
    cmp_count_enabled = false;
    uh->validate(uh->h->heap);
    cmp_count_enabled = true;
}

static void
test_basic_insertion(struct unit_test *u)
{
    struct heap_interface *h = u->uh->h;
    struct unit_params *p = &u->params;
    enum init_mode mode = p->mode;
    unsigned int n = p->n_elems;
    struct element *elements;
    struct element *top;
    uint32_t *prios;
    size_t i;

    unit_heap_init(u->uh);

    elements = xcalloc(n, sizeof elements[0]);
    prios = xcalloc(n, sizeof prios[0]);

    n_cmp_reset();
    elements_init(elements, n, mode);
    for (i = 0; i < n; i++) {
        h->insert(h->heap, &elements[i]);
        unit_heap_validate(u->uh);
    }
    if (verbose) {
        printf("%u insertions[%s]: avg-n-cmp: %.2lf\n",
               n, mode2txt[mode],
               n_cmp ? (double) n / (double) n_cmp : 0);
    }
    n_cmp_reset();

    i = 0;
    while ((top = h->pop(h->heap)) != NULL) {
        unit_heap_validate(u->uh);
        prios[i++] = top->priority;
    }

    assert("Unexpected number of removal." && i == n);
    assert("Heap unexpectedly non-empty." && h->is_empty(h->heap));
    for (i = 0; i < n - 1; i++) {
        assert("Incorrect sorting of keys." && prios[i] <= prios[i + 1]);
    }

    if (verbose) {
        printf("%u removals[%s]: avg-n-cmp: %.2lf\n",
               n, mode2txt[mode],
               n_cmp ? (double) n / (double) n_cmp : 0);
    }

    free(elements);
    free(prios);
}

static void
test_insertion(struct unit_heap_interface *uh)
{
    struct unit_test u = UNIT_INITIALIZER;
    enum init_mode mode;

    u.params = (struct unit_params) params;
    u.uh = uh;

    for (mode = INCREASING; mode < N_MODES; mode++) {
        u.params.mode = mode;
        test_basic_insertion(&u);
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

    test_insertion(&unit_min_pairing_heap);
    test_insertion(&unit_min_binary_heap);

    if (verbose) {
        printf("Test succeeded.\n");
    }

    return 0;
}
