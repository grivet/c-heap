Heaps in C
==========

This repository provides three generic heap implementations in C.

I had issues with the traditional binary heap in some workloads and needed
a fair comparison with the pairing heap. I also wanted to explore a bit the
similarities between pairing and fibonacci heaps, so I added it.

The result of my tests was that the pairing heap is better suited for
workloads where an element will regularly change priority.

## Usage

```
./configure
make run
```

A code sample showing the use of the pairing heap can be found at the end
of this document.

## Implementations

There are three header-only implementations that provide the building
blocks to implement each heap type.

Genericity is achieved by using intrusive types for pairing and fibonacci
heaps. The binary heap is implemented as a container to benefit from
its spatial properties.

Implementations are then wrapped within an abstract heap interface
within the `test` directory, to be used by unit and performance tests.
This layer is very thin, although some additions were written to enforce
heap invariants during tests.

### Pairing heap

The `pheap` and `pheap_node` types are defined, along with pairing heap operators:

  * `pheap_init`
  * `pheap_is_empty`
  * `pheap_peek`
  * `pheap_pop`
  * `pheap_insert`
  * `pheap_merge`
  * `pheap_reinsert`

To update an element key, update its priority within its container, then re-insert
its `pheap_node` field. Only a node that was already within the heap should be
reinserted.

Ancillary functions such as `pheap_node_merge` and `pheap_node_pairwise_merge` are
used to implement the above operators and should be avoided. Unlike many
other public implementation, they are written in iterative form.

### Binary heap

A `bheap` and `bheap_node` types is defined, along with binary heap operators:

  * `bheap_init`
  * `bheap_is_empty`
  * `bheap_peek`
  * `bheap_pop`
  * `bheap_insert`
  * `bheap_update_key`

Ancillary functions such as `bheap_down` and `bheap_up` used to 'bubble'
elements down and up are exposed but should only be used by the above operators.

This is a 'containing' structure, each elements holding a generic pointer to
data as well as a priority value.

### Fibonacci heap

The `fheap` and `fheap_node` types are defined, along with fibonacci heap operators:

  * `fheap_init`
  * `fheap_is_empty`
  * `fheap_peek`
  * `fheap_pop`
  * `fheap_insert`
  * `fheap_merge`
  * `fheap_update_key`

Ancillary functions `fheap_reinsert`, `fheap_consolidate` and other node functions
are exposed and should be avoided.

Similarly to the pairing heap, a key update is done by reinserting a node. However,
because the process was more complex than with the pairing heap, the actual `update_key`
function is fully implemented and should be used directly.

## Tests

Unit tests are done for insertion and key modification, but they involve all operators.
Unit tests are compiled with address, leak and undefined sanitizers by default, depending
on compiler availability.

A performance test was written to simulate the priority queue context I wanted to
use one for, which was a connection tracker. Priorities are connection timeout
values and the priority queue allowing immediate access to the next connection
to remove. Timeouts are regularly update for all connections as traffic is seen.

The benchmark thus has 3 cases per heap, with 0, 10 and 30% of key updates per sweep.

The fibonacci heap is rather slow, so the number of elements was slightly reduced in
the benchmark.


## Example

```c
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "pairing-heap.h"

struct entry {
	struct pheap_node node;
	int data;
};

#define container_of(addr, type, field) \
    ((type *) (void *) ((char *) (addr) - offsetof (type, field)))
static int
entry_cmp(struct pheap_node *a, struct pheap_node *b)
{
	struct entry *entries[2] = {
		container_of(a, struct entry, node),
		container_of(b, struct entry, node),
	};

	return (entries[0]->data - entries[1]->data);
}

int main(void)
{
#define N 10
	struct entry entries[N];
	struct entry *sorted[N];
	struct pheap_node *node;
	struct pheap heap;
	unsigned int seed;
	int i;

	seed = (unsigned) time(NULL);

	pheap_init(&heap, entry_cmp);
	for (i = 0; i < N; i++) {
		entries[i].data = rand_r(&seed);
		pheap_insert(&heap, &entries[i].node);
	}

	i = 0;
	while ((node = pheap_pop(&heap))) {
		sorted[i++] = container_of(node, struct entry, node);
	}

	for (i = 0; i < N; i++) {
		printf("%d\n", sorted[i]->data);
	}

	return 0;
}
```
