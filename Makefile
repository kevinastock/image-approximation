# You'll probably have to tweak this for your platform.

CC = cc
CFLAGS = -c -O3 -Wall -I/usr/local/include -fopenmp
#CFLAGS = -c -g -O0 -Wall -I/usr/local/include
LIBS = -lglut -lGLU -fopenmp

default: main

main: main.o image.o renderer.o mh.o mt19937-64.o tr.o
	$(CC) main.o image.o renderer.o mh.o mt19937-64.o tr.o $(LIBS) -o $@

main.o: main.c wiproj.h 
	$(CC) $(CFLAGS) main.c

image.o: image.c wiproj.h 
	$(CC) $(CFLAGS) image.c

renderer.o: renderer.c wiproj.h 
	$(CC) $(CFLAGS) renderer.c

mh.o: mh.c wiproj.h mt64.h 
	$(CC) $(CFLAGS) mh.c

mt19937-64.o: mt19937-64.c mt64.h 
	$(CC) $(CFLAGS) mt19937-64.c

tr.o: tr.c tr.h
	$(CC) $(CFLAGS) tr.c

clean:
	-rm *.o
	-rm main

