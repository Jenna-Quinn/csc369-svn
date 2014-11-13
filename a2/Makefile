
sim :  sim.o pagetable.o avl.o rand.o clock.o lru.o fifo.o opt.o
	gcc -Wall -g -o sim $^

%.o : %.c avl.h pagetable.h
	gcc -Wall -g -c $<

simpleloop : simpleloop.c
	gcc -Wall -g -o simpleloop $^

matmul : matmul.c timer.h
	gcc -Wall -g -o matmul $^

blocked : blocked.c timer.h
	gcc -Wall -g -o blocked $^

clean :
	rm *.o sim
