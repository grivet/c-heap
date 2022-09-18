# SPDX-License-Identifier: BSD-3-Clause
# Copyright 2022 Gaëtan Rivet

include config.mk
ifeq ($(CONFIGURED),)
$(error Please run "./configure" before using make)
endif

all: unit bench

CFLAGS_ALL := -MD $(CFLAGS_AUTO) $(CFLAGS)
CFLAGS_ALL += -I$(CURDIR) -I$(CURDIR)/test

LDFLAGS_ALL = $(LDFLAGS_AUTO) $(LDFLAGS) -lm

test/%.o: test/%.c Makefile config.mk
	$(CC) $(CFLAGS_ALL) -c -o $@ $<

util_OBJS := test/util.o
util_OBJS += test/pairing-heap.o
util_OBJS += test/binary-heap.o

unit_OBJS := test/unit/main.o $(util_OBJS)
unit_OBJS += test/unit/pairing-heap.o
unit_OBJS += test/unit/binary-heap.o

unit: $(unit_OBJS)
	$(CC) $(CFLAGS_ALL) -o $@ $^ $(LDFLAGS_ALL)

bench_OBJS := test/bench/main.o $(util_OBJS)

bench: $(bench_OBJS)
	$(CC) $(CFLAGS_ALL) -o $@ $^ $(LDFLAGS_ALL)

.PHONY: check
check: unit
	ASAN_OPTIONS=abort_on_error=1 $(CURDIR)/unit -v

.PHONY: run
run: unit bench
	$(CURDIR)/unit && $(CURDIR)/bench

-include test/*.d
-include test/bench/*.d
-include test/unit/*.d

.PHONY: clean
clean:
	rm -f unit $(unit_OBJS) $(unit_OBJS:%.o=%.d)
	rm -f bench $(bench_OBJS) $(bench_OBJS:%.o=%.d)

.PHONY: distclean
distclean: clean
	rm config.mk
