/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#ifndef BENCH_H
#define BENCH_H

#include "mov-avg.h"
#include "util.h"

struct test_params {
    const char *name;
    unsigned int n_elems;
    unsigned int range;
    unsigned int seed;
    /* Probability within [0, 100) that an element has
     * its priority updated and being re-inserted in the queue. */
    uint32_t p_update;
};

#define TEST_PARAMS_INITIALIZER { \
    .name = NULL, \
    .n_elems = 0, .range = 0, .seed = 0, \
    .p_update = 0, \
}

struct test_results {
    union {
        struct {
            long long int start;
            long long int insertion;
            long long int end;
        } times;
        long long int t[3];
    };
    long long int delta;
    unsigned int sweep_limit;
    struct mov_avg_cma cma;
};

#define TEST_RESULTS_INITIALIZER { \
    .t = { 0, 0, 0, }, \
    .delta = 0, .sweep_limit = 0, \
    .cma = MOV_AVG_CMA_INITIALIZER, \
}

struct test {
    struct test_params params;
    struct test_results results;
    struct heap *h;
};

#define TEST_INITIALIZER { \
    .params = TEST_PARAMS_INITIALIZER, \
    .results = TEST_RESULTS_INITIALIZER, \
    .h = NULL, \
}

#endif /* BENCH_H */
