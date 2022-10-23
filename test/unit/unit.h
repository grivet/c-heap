/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#ifndef UNIT_H
#define UNIT_H

#include <stdbool.h>
#include <stdint.h>

#include "mov-avg.h"
#include "util.h"

enum init_mode {
    INCREASING,
    DECREASING,
    RANDOM,
    STAGGERED, /* Use several 'plateaus' of repeating values. */
    N_MODES,
};

struct unit_params {
    unsigned int n_elems;
    uint32_t seed;
    enum init_mode mode;
};

#define UNIT_PARAMS_INITIALIZER { \
    .n_elems = 0, .seed = 0, .mode = INCREASING, \
}

struct unit_results {
    struct mov_avg_cma n_cmp_cma;
};

#define UNIT_RESULTS_INITIALIZER { \
    .n_cmp_cma = MOV_AVG_CMA_INITIALIZER, \
}

struct unit_test {
    struct unit_params params;
    struct unit_results results;
    struct heap *h;
};

#define UNIT_INITIALIZER { \
    .params = UNIT_PARAMS_INITIALIZER, \
    .results = UNIT_RESULTS_INITIALIZER, \
    .h = NULL, \
}

#endif /* UNIT_H */
