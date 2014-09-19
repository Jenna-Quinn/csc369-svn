#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {

	struct timeval start, end;
	double diff;
	int num_tries = 100;

	if (argc == 2) {
		num_tries = atoi(argv[1]);
	}

	/* MOST OF YOUR CODE WILL GO HERE */
    
    // Record start time of system call
    gettimeofday(&start, NULL);

    // Read 0 bytes from an empty file
    FILE *fp = fopen("/dev/null","r");
    fread(NULL, 0, 0, fp);
    
    // Record end of system call
    gettimeofday(&end, NULL);

	/* TO HERE */

	diff = 1000000 * (end.tv_sec - start.tv_sec) +
			(end.tv_usec - start.tv_usec);
	diff = diff / num_tries;

    //EDIT: I changed the print statement because you probably
	// won't be measuring getpid()
	printf("Time is %f usec (num tries = %d)\n", diff, num_tries);
	// The number of times we need to execute the system call to get a
	// measurement that seems stable is:
	// NUM_TRIES: 9500
    
    /*
     
     The times were sporadic and then declined slowly:
     
     # run.sh
     for i in 100 200 300 500 1000 1500 2000 2500 3000 3500 4000 4500 5000 5500 6000 6500 7000 7500 8000 8500 9000 9500 10000; do
        ./syscall_cost $i
     done

     # output
     wolf:~/csc369$ ./run.sh
     Time is 0.780000 usec (num tries = 100)
     Time is 0.375000 usec (num tries = 200)
     Time is 0.250000 usec (num tries = 300)
     Time is 0.150000 usec (num tries = 500)
     Time is 0.074000 usec (num tries = 1000)
     Time is 0.092667 usec (num tries = 1500)
     Time is 0.038000 usec (num tries = 2000)
     Time is 0.053600 usec (num tries = 2500)
     Time is 0.025333 usec (num tries = 3000)
     Time is 0.021143 usec (num tries = 3500)
     Time is 0.018750 usec (num tries = 4000)
     Time is 0.029556 usec (num tries = 4500)
     Time is 0.026400 usec (num tries = 5000)
     Time is 0.024182 usec (num tries = 5500)
     Time is 0.021667 usec (num tries = 6000)
     Time is 0.019231 usec (num tries = 6500)
     Time is 0.018857 usec (num tries = 7000)
     Time is 0.017600 usec (num tries = 7500)
     Time is 0.016625 usec (num tries = 8000)
     Time is 0.012471 usec (num tries = 8500)
     Time is 0.011444 usec (num tries = 9000)
     Time is 0.010842 usec (num tries = 9500)
     Time is 0.010300 usec (num tries = 10000)
     
     */
    
	return 0;
}
