CC = gcc
CFLAGS =  -Wpedantic
# -Wall -Wextra -Werror
PATH_SRC = src/
PATH_COMMON = $(PATH_SRC)common/
PATH_HEADERS = include/
ENV_FLAG = 

ifeq ($(OS),Windows_NT)
	PATH_SPEC = $(PATH_SRC)windows/
	ENV_FLAG = -lws2_32
else
	PATH_SPEC = $(PATH_SRC)linux/
	ENV_FLAG = -pthread -lws2_32
endif

debugversion: $(PATH_COMMON)linked_list.c $(PATH_HEADERS)linked_list.h  $(PATH_COMMON)thpool.c $(PATH_HEADERS)thpool.h $(PATH_COMMON)logger.c $(PATH_HEADERS)logger.h $(PATH_COMMON)userfile.c $(PATH_HEADERS)userfile.h $(PATH_SPEC)socket.c $(PATH_HEADERS)socket.h $(PATH_COMMON)taskworker.c $(PATH_HEADERS)taskworker.h $(PATH_COMMON)parsedata.c $(PATH_HEADERS)parsedata.h 
	$(CC) $(CFLAGS) -D DISABLE_MUTI_THREAD -I$(PATH_HEADERS) $(PATH_COMMON)linked_list.c $(ENV_FLAG) $(PATH_COMMON)thpool.c $(PATH_COMMON)logger.c $(PATH_COMMON)userfile.c $(PATH_SPEC)socket.c $(PATH_COMMON)taskworker.c $(PATH_COMMON)parsedata.c $(PATH_SRC)main.c -g -o debugversion

testlogger: $(PATH_SRC)testlogger.c $(PATH_COMMON)logger.c $(PATH_HEADERS)logger.h
	$(CC) $(CFLAGS) -D DISABLE_MUTI_THREAD -I$(PATH_HEADERS) $(PATH_COMMON)logger.c $(PATH_SRC)testlogger.c -o testlogger

thpool_debug:$(PATH_COMMON)linked_list.c $(PATH_HEADERS)linked_list.h  $(PATH_COMMON)thpool.c $(PATH_HEADERS)thpool.h $(PATH_COMMON)logger.c $(PATH_HEADERS)logger.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) $(PATH_COMMON)linked_list.c $(ENV_FLAG) $(PATH_COMMON)thpool.c $(PATH_COMMON)logger.c $(PATH_SRC)thpool_test.c -o thpool_debug -g

userfile_debug:$(PATH_COMMON)linked_list.c $(PATH_HEADERS)linked_list.h  $(PATH_COMMON)thpool.c $(PATH_HEADERS)thpool.h $(PATH_COMMON)logger.c $(PATH_HEADERS)logger.h $(PATH_COMMON)userfile.c $(PATH_HEADERS)userfile.h
	$(CC) $(CFLAGS)  -D DISABLE_MUTI_THREAD -I$(PATH_HEADERS) $(PATH_COMMON)linked_list.c $(PATH_COMMON)thpool.c $(PATH_COMMON)logger.c $(PATH_COMMON)userfile.c $(PATH_SRC)userfile_debug.c -o userfile_debug $(ENV_FLAG)

setting_test:$(PATH_COMMON)setting.c $(PATH_HEADERS)setting.h
	$(CC) $(CFLAGS) -D DISABLE_MUTI_THREAD -I$(PATH_HEADERS) $(PATH_COMMON)setting.c $(PATH_SRC)setting_test.c -o setting_test -lws2_32

dao_test: $(PATH_COMMON)sqlite3.c $(PATH_HEADERS)sqlite3.h $(PATH_COMMON)linked_list.c $(PATH_HEADERS)linked_list.h  $(PATH_COMMON)thpool.c $(PATH_HEADERS)thpool.h $(PATH_COMMON)logger.c $(PATH_HEADERS)logger.h $(PATH_COMMON)userfile.c $(PATH_HEADERS)userfile.h $(PATH_COMMON)db.c $(PATH_HEADERS)db.h $(PATH_COMMON)trie.c $(PATH_HEADERS)trie.h $(PATH_COMMON)dao.c $(PATH_HEADERS)dao.h
	$(CC) $(CFLAGS) -D DISABLE_MUTI_THREAD -I$(PATH_HEADERS) $(PATH_COMMON)sqlite3.c $(PATH_COMMON)linked_list.c  $(PATH_COMMON)thpool.c $(PATH_COMMON)logger.c $(PATH_COMMON)userfile.c $(PATH_COMMON)db.c $(PATH_COMMON)trie.c $(PATH_COMMON)dao.c $(PATH_SRC)dao_test.c $(ENV_FLAG) -o dao_test
clean:
	rm -f testlogger thpool_debug
