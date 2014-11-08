
sim :  sim.o pagetable.o avl.o rand.o clock.o lru.o fifo.o opt.o
	gcc -Wall -g -o sim $^

%.o : %.c avl.h pagetable.h
	gcc -Wall -g -c $<

clean : 
	rm *.o sim
