CC = gcc
CFLAGS = -Wall -Wextra -g -I..

PROGS = echoserveri echoclient
COMMON_OBJS = ../csapp.o

all: $(PROGS)

echoserveri: echoserveri.o echo.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ echoserveri.o echo.o $(COMMON_OBJS) -lpthread

echoclient: echoclient.o $(COMMON_OBJS)
	$(CC) $(CFLAGS) -o $@ echoclient.o $(COMMON_OBJS) -lpthread

../csapp.o: ../csapp.c ../csapp.h
	$(CC) $(CFLAGS) -c -o $@ ../csapp.c

echoserveri.o: echoserveri.c ../csapp.h
	$(CC) $(CFLAGS) -c echoserveri.c

echoclient.o: echoclient.c ../csapp.h
	$(CC) $(CFLAGS) -c echoclient.c

echo.o: echo.c ../csapp.h
	$(CC) $(CFLAGS) -c echo.c

clean:
	rm -f $(PROGS) *.o ../csapp.o

.PHONY: all clean
