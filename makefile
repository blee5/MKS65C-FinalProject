all: ganache.o hashtable.o files.o util.o ganache.h hashtable.h files.h util.h
	gcc ganache.o hashtable.o files.o util.o -o ganache -g

ganache.o: ganache.c 
	gcc ganache.c -c -o ganache.o -Wall -g

hashtable.o: hashtable.c 
	gcc hashtable.c -c -o hashtable.o -Wall -g

files.o: files.c
	gcc files.c -c -o files.o -Wall -g

util.o: util.c 
	gcc util.c -c -o util.o -Wall -g

clean:
	rm *.o
