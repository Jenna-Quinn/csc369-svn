# You are welcome to modiy the Makefile as long as it still produces the
# executables test_malloc and test_malloc_opt when make is run with no
# arguments

all : test_malloc test_malloc_opt

test_malloc: test_malloc.c mymemory.c
	gcc -Wall -Wno-deprecated-declarations -g -o test_malloc test_malloc.c mymemory.c -lpthread

test_malloc_opt: test_malloc.c mymemory_opt.c
	gcc -Wall -Wno-deprecated-declarations -g -o test_malloc_opt test_malloc.c mymemory_opt.c -lpthread

clean:
	rm test_malloc test_malloc_opt *.o
