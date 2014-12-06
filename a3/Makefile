all: ext2_cp ext2_mkdir ext2_ln ext2_rm

ext2.o: ext2.c ext2.h
	gcc -Wall -g -c $<

ext2_cp: ext2_cp.c ext2.o
	gcc -Wall -g -o ext2_cp $^

ext2_mkdir: ext2_mkdir.c ext2.o
	gcc -Wall -g -o ext2_mkdir $^

ext2_ln: ext2_ln.c ext2.o
	gcc -Wall -g -o ext2_ln $^

ext2_rm: ext2_rm.c ext2.o
	gcc -Wall -g -o ext2_rm $^

clean:
	rm *.o
