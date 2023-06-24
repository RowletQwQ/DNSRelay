CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wpedantic
PATH_SRC = src/
PATH_COMMON = $(PATH_SRC)common/
PATH_HEADERS = include/

ifeq ($(OS),Windows_NT)
	PATH_SPEC = $(PATH_SRC)windows/
else
	PATH_SPEC = $(PATH_SRC)linux/
endif

testlogger: $(PATH_SRC)testlogger.c $(PATH_COMMON)logger.c $(PATH_HEADERS)logger.h
	$(CC) $(CFLAGS) -D DISABLE_MUTI_THREAD -I$(PATH_HEADERS) $(PATH_COMMON)logger.c $(PATH_SRC)testlogger.c -o testlogger

thpool_debug: $(PATH_COMMON)linked_list.c $(PATH_HEADERS)linked_list.h $(PATH_COMMON)logger.c $(PATH_HEADERS)logger.h $(PATH_SPEC)thpool.c $(PATH_HEADERS)thpool.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) $(PATH_COMMON)linked_list.c -pthread $(PATH_SPEC)thpool.c $(PATH_COMMON)logger.c $(PATH_SPEC)thpool.c  $(PATH_SRC)thpool_test.c -o thpool_debug 