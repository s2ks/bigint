CC=gcc

SRC 	:= 			\
	   src/bigint.c 	\
	   src/bigint_util.c

OBJS 	:= $(SRC:.c=.o)

CFLAGS += -Wall -Wextra -Wpedantic
CFLAGS += -I include

# optimize for debugging
CFLAGS += -ggdb -Og
#CFLAGS += -fsanitize=address
#CFLAGS += -fsanitize=undefined
#LDFLAGS += -fsanitize=address
#LDFLAGS += -fsanitize=undefined
# or disable debugging (disables codegen for assert)
#CFLAGS += -D NDEBUG

#LDFLAGS += -L ~/.local/lib/
#LDFLAGS += -larg

# optimise for profiling
#LDFLAGS += -lprofiler
LDFLAGS += -pg
CFLAGS += -pg

HEADERS := include/bigint.h


all: bigint_test

BIGINT_TEST 	:= tests/bigint_test.c

BIGINT_TEST_OBJS := $(BIGINT_TEST:.c=.o)

# linking
bigint_test: $(OBJS) $(BIGINT_TEST_OBJS)
	$(CC) -o $@ $(LDFLAGS) $(OBJS) $(BIGINT_TEST_OBJS)

# compiling/assembling
%.o: %.c $(HEADERS)
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: clean

clean:
	-rm $(OBJS)
	-rm $(BIGINT_TEST_OBJS)
	-rm bigint_test
