all: ganache.o hashtable.o util.o ganache.h hashtable.h util.h
	gcc ganache.o hashtable.o util.o -o ganache -g

ganache.o: ganache.c 
	gcc ganache.c -c -o ganache.o -Wall -g

hashtable.o: hashtable.c 
	gcc hashtable.c -c -o hashtable.o -Wall -g

util.o: util.c 
	gcc util.c -c -o util.o -Wall -g

clean:
	rm *.o
