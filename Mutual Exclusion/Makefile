CC=gcc

commMake:  MutualExclusion.c  
	$(CC) MutualExclusion.c -lm -w -lpthread -o MutualExclusion 
	 
	 

all: run

run: cli
	./MutualExclusion -p 20000

.PHONY: all run

clean:
	rm -f *.o  

