/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(c) 2022 Gaëtan Rivet
 */

#ifndef UTIL_H
#define UTIL_H

#define _XOPEN_SOURCE 700

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#ifndef offsetof
#define offsetof(type, field) \
    ((size_t)((char *)&(((type *)0)->field) - (char *)0))
#endif

/* Annotate any function using 'container_of' with this macro */
#define ALLOW_UNDEFINED_BEHAVIOR \
    __attribute__((no_sanitize("undefined")))
#define container_of(addr, type, field) \
    ((type *) (void *) ((char *) (addr) - offsetof (type, field)))

#define ARRAY_SIZE(ar) (sizeof(ar) / sizeof(ar[0]))

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a < b ? a : b)

extern uint32_t rand_seed;

/* The state word must be initialized to non-zero */
static inline uint32_t
xorshift32(uint32_t x[1])
{
    /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
    x[0] ^= x[0] << 13;
    x[0] ^= x[0] >> 17;
    x[0] ^= x[0] << 5;
    return x[0];
}

void random_init(uint32_t seed);

static inline uint32_t
random_u32(void)
{
    return xorshift32(&rand_seed);
}

static inline uint32_t
random_u32_range(uint32_t max)
{
    return random_u32() % max;
}

static inline void
swap_u32(uint32_t *a, uint32_t *b)
{
    uint32_t tmp = *a;
    *a = *b;
    *b = tmp;
}

static inline void
shuffle_u32(uint32_t *p, size_t n)
{
    for (; n > 1; n--, p++) {
        uint32_t *q = &p[random_u32_range(n)];
        swap_u32(p, q);
    }
}

void out_of_memory(void);
void *xcalloc(size_t count, size_t size);
void *xzalloc(size_t size);
void *xmalloc(size_t size);

long long int time_msec(void);
long long int time_usec(void);

bool str_to_uint(const char *s, int base, unsigned int *result);

#endif /* UTIL_H */
