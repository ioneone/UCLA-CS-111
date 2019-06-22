// Name  : Junhong Wang
// ID    : 504941113
// Email : oneone@g.ucla.edu

#include <pthread.h>
#include <time.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BILLION 1000000000L

pthread_t *tids;

int opt_yield = 0;
int opt_sync  = 0;
int s_lock    = 0;

pthread_mutex_t m_lock;

char sync_method;
char tag[100] = "";

struct ThreadInfo {
	long long *counter;
	int iterations;
};

void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if (opt_yield) sched_yield();
	*pointer = sum;
}

void mutex_add(long long *pointer, long long value) {
	pthread_mutex_lock(&m_lock);
	add(pointer, value);
	pthread_mutex_unlock(&m_lock);
}

void spin_add(long long *pointer, long long value) {
	while (__sync_lock_test_and_set(&s_lock, 1));
	add(pointer, value);
	__sync_lock_release(&s_lock);
}

void atomic_add(long long *pointer, long long value) {
	// no longer using origianl add function

	long long expectedCounter;
	long long newCounter;

	do {
		expectedCounter = *pointer;
		newCounter      = expectedCounter + value;
		if (opt_yield) sched_yield();
	} while (__sync_val_compare_and_swap(pointer, expectedCounter, newCounter) != expectedCounter);

}


char* get_usage() {
	return 
		"\
Usage: ./lab2a_add [OPTION]...\n\
Starts the specified number of threads,\n\
each of which will use the above add function to\n\
1. add 1 to the counter the specified number of times\n\
2. add -1 to the counter the specified number of times\n\
3. exit to re-join the parent thread\n\
Then prints to stdout a comma-separated-value (CSV) record.\n\
\n\
  --threads=NUMBER     takes a parameter for the number of\n\
                       parallel threads (default 1)\n\
  --iterations=NUMBER  takes a parameter for the number of\n\
                       iterations (default 1)\n\
  --yield              cause a thread to immediately yield\n\
  --sync=TYPE          execute with synchronization\n\
                       use mutex if TYPE=m\n\
                       use spin-lock if TYPE=s\n\
                       use compare-and-swap if TYPE=c\n\
		";
}

void thread_handle_loop(long long *counter, int iterations, int value) {
	int i;

	for (i = 0; i < iterations; i++) {

		if (opt_sync) {

			if (sync_method == 'm')
				mutex_add(counter, value);
			else if (sync_method == 's')
				spin_add(counter, value);
			else if (sync_method == 'c')
				atomic_add(counter, value);

		} 
		else {
			add(counter, value);
		}

	}
}

void* thread_function(void *threadInfo) {
	struct ThreadInfo *info = (struct ThreadInfo*) threadInfo;

	thread_handle_loop(info->counter, info->iterations,  1);
	thread_handle_loop(info->counter, info->iterations, -1);
	
	return NULL;
}

void handle_yield_tag() {
	if (opt_yield) strcat(tag, "-yield");
}

void handle_sync_tag() {
	if (opt_sync) {
		if (sync_method == 'm')
			strcat(tag, "-m");
		else if (sync_method == 's')
			strcat(tag, "-s");
		else if (sync_method == 'c')
			strcat(tag, "-c");
	} 
	else {
		strcat(tag, "-none");
	}
}

char* get_tag() {
	
	strcat(tag, "add");
	
	handle_yield_tag();

	handle_sync_tag();
	
	return tag;
}

int is_sync_method_valid(char sync_method) {
	return sync_method == 'm' || sync_method == 's' || sync_method == 'c';
}

void handle_unrecognized_argument() {
	fprintf(stderr, "Unrecognized argument detected\n%s\n", get_usage());
	exit(1);
}

void initialize_mutex() {
	if (pthread_mutex_init(&m_lock, NULL)) {
		fprintf(stderr, "Error initializing mutex\n");
		exit(1);
	}
}

void handle_sync_option(char *optarg) {
	opt_sync = 1;
	sync_method = optarg[0];
	if (!is_sync_method_valid(sync_method)) handle_unrecognized_argument();
	if (sync_method == 'm') initialize_mutex();
}

void handle_options(int argc, char **argv, int *numThreads, int *numIterations) {

	int option;

	struct option options[] = {
		{"threads",    required_argument, NULL, 't'}, // takes a parameter for the number of parallel threads (--threads=#, default 1).
		{"iterations", required_argument, NULL, 'i'}, // takes a parameter for the number of iterations (--iterations=#, default 1).
		{"yield",      no_argument,       NULL, 'y'}, // Add a new --yield to your driver program that sets opt_yield to 1
		{"sync",       required_argument, NULL, 's'}, // synchronization option
		{0, 0, 0, 0}
	};

	while (1) {
		option = getopt_long(argc, argv, "", options, NULL);

		if (option == -1) break;

		switch (option) {
			case 't':
				*numThreads = atoi(optarg);
				break;
			case 'i':
				*numIterations = atoi(optarg);
				break;
			case 'y':
				opt_yield = 1;
				break;
			case 's':
				handle_sync_option(optarg);
				break;
			default:
				handle_unrecognized_argument();
		}
	}
}

long long get_num_operations(int numThreads, int numIterations) {
	return numThreads * numIterations * 2;
}

long long get_run_time(struct timespec *start, struct timespec *end) {
	return BILLION * (end->tv_sec - start->tv_sec) + end->tv_nsec - start->tv_nsec;
}

long long get_average_time_per_operation(long long runTime, long long numOperations) {
	return runTime / numOperations;
}

void free_tids() {
	free(tids);
}

void safe_pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*start_routine)(void*), void *arg) {
	if (pthread_create(thread, attr, start_routine, arg) != 0) {
		fprintf(stderr, "Error creating a thread\n");
		exit(1);
	}
}

void safe_pthread_join(pthread_t thread, void **retval) {
	if (pthread_join(thread, retval) != 0) {
		fprintf(stderr, "Error joining a thread\n");
		exit(1);
	}
}

void check_malloc_error(void *ptr) {
	if (ptr == NULL) {
		fprintf(stderr, "Error allocating memory\n");
		exit(1);
	}
}

void initialize_tids(int numThreads) {
	tids = (pthread_t*) malloc(sizeof(pthread_t) * numThreads);
	check_malloc_error(tids);

	atexit(free_tids);
}

void handle_threads(long long *counter, int numThreads, int numIterations) {
	int i;

	initialize_tids(numThreads);

	struct ThreadInfo info;
	info.counter    = counter;
	info.iterations = numIterations;

	for (i = 0; i < numThreads; i++)
		safe_pthread_create(&tids[i], NULL, thread_function, (void*) &info);

	for (i = 0; i < numThreads; i++)
		safe_pthread_join(tids[i], NULL);
}

void handle_report(long long counter, int numThreads, int numIterations, struct timespec *start, struct timespec *end) {
	long long numOperations = get_num_operations(numThreads, numIterations);
	long long runTime       = get_run_time(start, end);
	printf(
		"%s,%d,%d,%lld,%lld,%lld,%lld\n", 
		get_tag(), 
		numThreads,
		numIterations,
		numOperations,
		runTime,
		get_average_time_per_operation(runTime, numOperations),
		counter
	);
} 

int main(int argc, char **argv) {

	long long counter = 0;
	int numThreads = 1;
	int numIterations = 1;
	struct timespec start, end;
	
	// set flags
	handle_options(argc, argv, &numThreads, &numIterations);

	// notes the (high resolution) starting time for the run (using clock_gettime(3))
	clock_gettime(CLOCK_MONOTONIC, &start);

	// starts the specified number of threads, each of which will use the above add function
	handle_threads(&counter, numThreads, numIterations);

	// wait for all threads to complete, and notes the (high resolution) ending time for the run
	clock_gettime(CLOCK_MONOTONIC, &end);

	// prints to stdout a comma-separated-value (CSV) record
	handle_report(counter, numThreads, numIterations, &start, &end);
	
	exit(0);
}
