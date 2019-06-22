// Name  : Junhong Wang
// ID    : 504941113
// Email : oneone@g.ucla.edu

#include "SortedList.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <signal.h>

#define BILLION 1000000000L

struct ThreadInfo {
	int index;
};

int numThreads = 1;
int numIterations = 1;
int opt_yield = 0;
int opt_sync = 0;
int key_length = 3;
int s_lock    = 0;

pthread_mutex_t m_lock;

SortedList_t *list;
SortedListElement_t *elements;
pthread_t *tids;
struct ThreadInfo *info;

char tag[100] = "";
char sync_method;

char* get_usage() {
	return 
		"\
Usage: ./lab2a_list [OPTION]...\n\
Creates and initializes (with random keys) the required number\n\
(threads x iterations) of list elements. Each thread inserts them\n\
all into a (single shared-by-all-threads) list. Then looks up and\n\
deletes each of the keys it had previously inserted. Then prints\n\
to stdout a comma-separated-value (CSV) record\n\
\n\
  --threads=NUMBER     takes a parameter for the number of\n\
                       parallel threads (default 1)\n\
  --iterations=NUMBER  takes a parameter for the number of\n\
                       iterations (default 1)\n\
  --yield=TYPE         takes a parameter to enable (any combination of)\n\
                       optional critical section yields\n\
                       --yield=[idl]\n\
                       i for insert\n\
                       d for delete\n\
                       l for lookups\n\
  --sync=TYPE          execute with synchronization\n\
                       use mutex if TYPE=m\n\
                       use spin-lock if TYPE=s\n\
		";
}

void handle_unrecognized_argument() {
	fprintf(stderr, "Unrecognized argument detected\n%s\n", get_usage());
	exit(1);
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

void mutex_list_insert(SortedList_t *list, SortedListElement_t *element) {
	pthread_mutex_lock(&m_lock);
	SortedList_insert(list, element);
	pthread_mutex_unlock(&m_lock);
}

void spin_list_insert(SortedList_t *list, SortedListElement_t *element) {
	while (__sync_lock_test_and_set(&s_lock, 1));
	SortedList_insert(list, element);
	__sync_lock_release(&s_lock);
}

int mutex_list_length(SortedList_t *list) {
	int length;
	pthread_mutex_lock(&m_lock);
	length = SortedList_length(list);
	pthread_mutex_unlock(&m_lock);
	return length;
}

int spin_list_length(SortedList_t *list) {
	int length;
	while (__sync_lock_test_and_set(&s_lock, 1));
	length = SortedList_length(list);
	__sync_lock_release(&s_lock);
	return length;
}

void handle_list_delete(SortedList_t *list, const char *key) {
	SortedListElement_t *element;
	element = SortedList_lookup(list, key);
	if (element == NULL) {
		fprintf(stderr, "Failed to find previously inserted element\n");
		exit(2);
	}
	if (SortedList_delete(element) == 1) {
		fprintf(stderr, "Failed to delete previously looked up element\n");
		exit(2);
	}
}

void mutex_list_delete(SortedList_t *list, const char *key) {
	pthread_mutex_lock(&m_lock);
	handle_list_delete(list, key);
	pthread_mutex_unlock(&m_lock);
}

void spin_list_delete(SortedList_t *list, const char *key) {
	while (__sync_lock_test_and_set(&s_lock, 1));
	handle_list_delete(list, key);
	__sync_lock_release(&s_lock);
}

int get_num_elements() {
	return numThreads * numIterations;
}

void thread_list_insert(int thread_index) {
	int i;

	for (i = thread_index; i < get_num_elements(); i += numThreads) {
		if (opt_sync) {

			if (sync_method == 'm') 
				mutex_list_insert(list, &elements[i]);
			else if (sync_method == 's')
				spin_list_insert(list, &elements[i]);

		} else {
			SortedList_insert(list, &elements[i]);
		}
		
	}
}

void thread_list_length() {
	int list_length;

	if (opt_sync) {

		if (sync_method == 'm')
			list_length = mutex_list_length(list);
		else if (sync_method == 's')
			list_length = spin_list_length(list);

	}
	else {
		list_length = SortedList_length(list);
	}

	if (list_length == -1) {
		fprintf(stderr, "Error getting the length of the list\n");
		exit(2);
	}
}

void thread_list_delete(int thread_index) {
	int i;

	for (i = thread_index; i < get_num_elements(); i += numThreads) {
		if (opt_sync) {

			if (sync_method == 'm')
				mutex_list_delete(list, elements[i].key);
			else if (sync_method == 's')
				spin_list_delete(list, elements[i].key);

		}
		else {
			handle_list_delete(list, elements[i].key);
		}
		
	}
}

void* thread_function(void *threadInfo) {

	struct ThreadInfo *info = (struct ThreadInfo*) threadInfo;

	// inserts them all into a (single shared-by-all-threads) list
	thread_list_insert(info->index);

	// gets the list length
	thread_list_length();

	// looks up and deletes each of the keys it had previously inserted
	thread_list_delete(info->index);

	// exits to re-join the parent thread
	return NULL;
	
}

void initialize_random_generator() {
	srand(time(NULL));
}

char generate_random_character() {
	return 'A' + (rand() % 26);
}

char* generate_random_key(int length) {
	int i;

	char *key = (char*) malloc(length + 1);

	for (i = 0; i < length; i++)
		key[i] = generate_random_character();

	key[key_length] = '\0'; // null terminated key

	return key;
}

void handle_yield_option(char *optarg) {
	int i;

	for (i = 0; i < (int) strlen(optarg); i++) {
		if (optarg[i] == 'i')
			opt_yield |= INSERT_YIELD;
		else if (optarg[i] == 'd')
			opt_yield |= DELETE_YIELD;
		else if (optarg[i] == 'l')
			opt_yield |= LOOKUP_YIELD;
		else
			handle_unrecognized_argument();
	}
}

void free_list() {
	free(list);
}

void free_elements() {
	int i;

	for (i = 0; i < get_num_elements(); i++)
		free((char*) elements[i].key);

	free(elements);
}

void free_tids() {
	free(tids);
}

void free_info() {
	free(info);
}

int is_sync_method_valid(char sync_method) {
	return sync_method == 'm' || sync_method == 's';
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

void handle_options(int argc, char **argv) {

	int option;

	struct option options[] = {
		{"threads",     required_argument, NULL, 't'}, // takes a parameter for the number of parallel threads (--threads=#, default 1).
		{"iterations", required_argument, NULL, 'i'}, // takes a parameter for the number of iterations (--iterations=#, default 1).
		{"yield",      required_argument, NULL, 'y'}, // takes a parameter to enable (any combination of) optional critical section yields
		{"sync",       required_argument, NULL, 's'}, // synchronization option
		{0, 0, 0, 0}
	};

	while (1) {
		option = getopt_long(argc, argv, "", options, NULL);

		if (option == -1) break;

		switch (option) {
			case 't':
				numThreads = atoi(optarg);
				break;
			case 'i':
				numIterations = atoi(optarg);
				break;
			case 'y':
				handle_yield_option(optarg);
				break;
			case 's':
				handle_sync_option(optarg);
				break;
			default:
				handle_unrecognized_argument();
		}


	}
}

long long get_run_time(struct timespec *start, struct timespec *end) {
	return BILLION * (end->tv_sec - start->tv_sec) + end->tv_nsec - start->tv_nsec;
}

long long get_average_time_per_operation(long long runTime, long long numOperations) {
	return runTime / numOperations;
}

long long get_num_operations(int numThreads, int numIterations) {
	return numThreads * numIterations * 3;
}

void handle_yield_tag() {
	if (opt_yield) {

		if (opt_yield & INSERT_YIELD)
			strcat(tag, "i");
		if (opt_yield & DELETE_YIELD)
			strcat(tag, "d");
		if (opt_yield & LOOKUP_YIELD)
			strcat(tag, "l");
		
	}
	else {
		strcat(tag, "none");
	}
	
}

void handle_sync_tag() {
	if (opt_sync) {
		if (sync_method == 'm')
			strcat(tag, "-m");
		else if (sync_method == 's')
			strcat(tag, "-s");
	} 
	else {
		strcat(tag, "-none");
	}
}

char* get_tag() {

	strcat(tag, "list-");
	
	handle_yield_tag();

	handle_sync_tag();
	
	return tag;
}

void handle_report(struct timespec *start, struct timespec *end) {
	int       numLists      = 1;
	long long numOperations = get_num_operations(numThreads, numIterations);
	long long runTime       = get_run_time(start, end);
	printf(
		"%s,%d,%d,%d,%lld,%lld,%lld\n", 
		get_tag(), 
		numThreads,
		numIterations,
		numLists,
		numOperations,
		runTime,
		get_average_time_per_operation(runTime, numOperations)
	);
}

void handle_segmentation_fault() {
	// do not use fprintf in signal handling
	write(2, "Segmentation Fault Detected\n", 28);
	exit(2);
}

void check_malloc_error(void *ptr) {
	if (ptr == NULL) {
		fprintf(stderr, "Error allocating memory\n");
		exit(1);
	}
}

void initialize_list() {
	list = (SortedList_t*) malloc(sizeof(SortedList_t));
	check_malloc_error(list);

	atexit(free_list);

	list->key  = NULL;
	list->prev = list;
	list->next = list;
}

void initialize_elements() {
	int i;

	elements = (SortedListElement_t*) malloc(sizeof(SortedListElement_t) * get_num_elements());
	check_malloc_error(elements);

	initialize_random_generator();

	for (i = 0; i < get_num_elements(); i++)
		elements[i].key = generate_random_key(key_length);

	atexit(free_elements);
}

void initialize_tids() {
	tids = (pthread_t*) malloc(sizeof(pthread_t) * numThreads);
	check_malloc_error(tids);

	atexit(free_tids);
}

void initialize_thread_info() {
	int i;

	info = (struct ThreadInfo*) malloc(sizeof(struct ThreadInfo) * numThreads);
	check_malloc_error(info);

	atexit(free_info);

	for (i = 0; i < numThreads; i++)
		info[i].index = i;
}

void handle_threads() {
	int i;

	initialize_tids();
	
	initialize_thread_info();

	for (i = 0; i < numThreads; i++)
		safe_pthread_create(&tids[i], NULL, thread_function, (void*) &info[i]);

	for (i = 0; i < numThreads; i++)
		safe_pthread_join(tids[i], NULL);
}

int main(int argc, char **argv) {

	struct timespec start, end;

	// set flags
	handle_options(argc, argv);

	// initializes an empty list.
	initialize_list();

	// creates and initializes (with random keys) the required number (threads x iterations) of list elements.
	initialize_elements();

	// prepare to catch segmentation fault
	signal(SIGSEGV, handle_segmentation_fault);

	// notes the (high resolution) starting time for the run (using clock_gettime(3)).
	clock_gettime(CLOCK_MONOTONIC, &start);

	// starts the specified number of threads.
	handle_threads();

	// waits for all threads to complete, and notes the (high resolution) ending time for the run.
	clock_gettime(CLOCK_MONOTONIC, &end);

	// checks the length of the list to confirm that it is zero.
	if (SortedList_length(list) != 0) {
		fprintf(stderr, "List length is not zero\n");
		exit(2);
	}

	// prints to stdout a comma-separated-value (CSV) record
	handle_report(&start, &end);

	exit(0);

}


