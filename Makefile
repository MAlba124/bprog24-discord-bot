CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99 -DCCORD_SIGINTCATCH
CFLAGS_DEBUG = $(CFLAGS) -g
CFLAGS_RELEASE = $(CFLAGS) -O2
CLINKFLAGS = -ldiscord -lcurl -lpthread
EXE_NAME = bprogbot

OBJS = objs/parser.o     \
	objs/mem.o       \
	objs/evaluator.o \
	objs/vector.o

# TODO: change cflags to release when needed
objs/%.o: src/%.c
	$(CC) $(CFLAGS_DEBUG) -c -o $@ $< $(CLINKFLAGS)

$(EXE_NAME): $(OBJS) src/main.c
	$(CC) $(CFLAGS) -o $(EXE_NAME) src/main.c $(OBJS) $(CLINKFLAGS)

.PHONY: build
build: $(OBJS) src/main.c
	$(CC) $(CFLAGS_DEBUG) -o $(EXE_NAME) src/main.c $(OBJS) $(CLINKFLAGS)

.PHONY: build_release
build_release: $(OBJS) src/main.c
	$(CC) $(CFLAGS_RELEASE) -o $(EXE_NAME) src/main.c $(OBJS) $(CLINKFLAGS)

.PHONY: run
run: build
	./$(EXE_NAME)

.PHONY: clean
clean:
	@rm $(EXE_NAME)

.PHONY: check
check:
	$(CC) $(CFLAGS) -fsyntax-only src/main.c

.PHONY: test
test: objs/mem.o src/test.c
	$(CC) $(CFLAGS) -o test objs/mem.o src/test.c $(CLINKFLAGS)
	./test
