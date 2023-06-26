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
	ENV_FLAG = -pthread 
endif

debugversion: linked_list.o thpool.o logger.o userfile.o socket.o setting.o trie.o taskworker.o parsedata.o main.o 
	$(CC) $(CFLAGS) -D DISABLE_MUTI_THREAD -I$(PATH_HEADERS) linked_list.o thpool.o logger.o userfile.o socket.o setting.o trie.o taskworker.o parsedata.o main.o -lm -g -o debugversion

testlogger: $(PATH_SRC)testlogger.c $(PATH_COMMON)logger.c $(PATH_HEADERS)logger.h
	$(CC) $(CFLAGS) -D DISABLE_MUTI_THREAD -I$(PATH_HEADERS) $(PATH_COMMON)logger.c $(PATH_SRC)testlogger.c -o testlogger

thpool_debug:$(PATH_COMMON)linked_list.c $(PATH_HEADERS)linked_list.h  $(PATH_COMMON)thpool.c $(PATH_HEADERS)thpool.h $(PATH_COMMON)logger.c $(PATH_HEADERS)logger.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) $(PATH_COMMON)linked_list.c $(ENV_FLAG) $(PATH_COMMON)thpool.c $(PATH_COMMON)logger.c $(PATH_SRC)thpool_test.c -o thpool_debug -g

userfile_debug:$(PATH_COMMON)linked_list.c $(PATH_HEADERS)linked_list.h  $(PATH_COMMON)thpool.c $(PATH_HEADERS)thpool.h $(PATH_COMMON)logger.c $(PATH_HEADERS)logger.h $(PATH_COMMON)userfile.c $(PATH_HEADERS)userfile.h
	$(CC) $(CFLAGS)  -D DISABLE_MUTI_THREAD -I$(PATH_HEADERS) $(PATH_COMMON)linked_list.c $(PATH_COMMON)thpool.c $(PATH_COMMON)logger.c $(PATH_COMMON)userfile.c $(PATH_SRC)userfile_debug.c -o userfile_debug $(ENV_FLAG)

setting_test:$(PATH_COMMON)setting.c $(PATH_HEADERS)setting.h
	$(CC) $(CFLAGS) -D DISABLE_MUTI_THREAD -I$(PATH_HEADERS) $(PATH_COMMON)setting.c $(PATH_SRC)setting_test.c -o setting_test -lws2_32

taskworker.o: $(PATH_COMMON)taskworker.c $(PATH_HEADERS)taskworker.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) -c $(PATH_COMMON)taskworker.c -o taskworker.o

dao_test: sqlite3.o linked_list.o thpool.o logger.o userfile.o dao.o trie.o db.o dao_test.o
	$(CC) $(CFLAGS) -D DISABLE_MUTI_THREAD -I$(PATH_HEADERS) sqlite3.o linked_list.o thpool.o logger.o userfile.o dao.o trie.o db.o dao_test.o $(ENV_FLAG) -o dao_test -lm

main.o : $(PATH_SRC)main.c $(PATH_HEADERS)main.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) -c $(PATH_SRC)main.c -o main.o

dao_test.o: $(PATH_SRC)dao_test.c $(PATH_HEADERS)dao.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) -c $(PATH_SRC)dao_test.c -o dao_test.o

db.o: $(PATH_COMMON)db.c $(PATH_HEADERS)db.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) -c $(PATH_COMMON)db.c -o db.o

trie.o: $(PATH_COMMON)trie.c $(PATH_HEADERS)trie.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) -c $(PATH_COMMON)trie.c -o trie.o

dao.o: $(PATH_COMMON)dao.c $(PATH_HEADERS)dao.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) -c $(PATH_COMMON)dao.c -o dao.o

userfile.o: $(PATH_COMMON)userfile.c $(PATH_HEADERS)userfile.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) -c $(PATH_COMMON)userfile.c -o userfile.o

socket.o: $(PATH_SPEC)socket.c $(PATH_HEADERS)socket.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) -c $(PATH_SPEC)socket.c -o socket.o

setting.o: $(PATH_COMMON)setting.c $(PATH_HEADERS)setting.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) -c $(PATH_COMMON)setting.c -o setting.o

parsedata.o: $(PATH_COMMON)parsedata.c $(PATH_HEADERS)parsedata.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) -c $(PATH_COMMON)parsedata.c -o parsedata.o

logger.o: $(PATH_COMMON)logger.c $(PATH_HEADERS)logger.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) -c $(PATH_COMMON)logger.c -o logger.o

linked_list.o: $(PATH_COMMON)linked_list.c $(PATH_HEADERS)linked_list.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) -c $(PATH_COMMON)linked_list.c -o linked_list.o

thpool.o: $(PATH_COMMON)thpool.c $(PATH_HEADERS)thpool.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) -c $(PATH_COMMON)thpool.c -o thpool.o

sqlite3.o: $(PATH_COMMON)sqlite3.c $(PATH_HEADERS)sqlite3.h
	$(CC) $(CFLAGS) -I$(PATH_HEADERS) -c $(PATH_COMMON)sqlite3.c -o sqlite3.o

clean:
	rm -f testlogger thpool_debug userfile_debug setting_test dao_test debugversion *.o
