all: ganache.c ganache.h
	gcc ganache.c ganache.h -o ganache -Wall

debug: ganache.c ganache.h
	gcc ganache.c ganache.h -o ganache -Wall -g
