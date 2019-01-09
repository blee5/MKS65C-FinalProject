all: ganache.o hashtable.o ganache.h hashtable.h
	gcc ganache.o hashtable.o -o ganache

ganache.o: ganache.c 
	gcc ganache.c -c -o ganache.o -Wall

hashtable.o: hashtable.c 
	gcc hashtable.c -c -o hashtable.o -Wall

clean:
	rm *.o
