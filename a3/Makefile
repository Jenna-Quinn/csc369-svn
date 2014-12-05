all: ext2_cp ext2_mkdir ext2_ln ext2_rm

ext2.o: ext2.c ext2.h
	gcc -Wall -g -c $<

%: %.c ext2.o
	gcc -Wall -g -o % $^

clean:
	rm *.o
