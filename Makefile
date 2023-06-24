CC = gcc
CFLAGS = -Wall -Wextra -Werror -Wpedantic
PATH_SRC = src/
PATH_HEADERS = include/

testlogger: $(PATH_SRC)testlogger.c
	$(CC) $(CFLAGS) $(PATH_SRC)testlogger.c -o testlogger