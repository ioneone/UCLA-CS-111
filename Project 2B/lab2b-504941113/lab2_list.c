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

int num_threads    = 1;
int num_iterations = 1;
int num_lists      = 1;

int opt_yield  = 0;
int opt_sync   = 0;
int key_length = 3;

int *s_locks;

pthread_mutex_t *m_locks;

SortedList_t *lists;
SortedListElement_t *elements;
pthread_t *tids;
struct ThreadInfo *info;

char tag[100] = "";
char sync_method;

long long total_lock_acquisition_time = 0;

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
  --lists=NUMBER       break the single (huge) sorted\n\
                       list into the specified number of sub-lists\n\
                       (each with its own list header and synchronization object)\n\
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

long long get_run_time(struct timespec *start, struct timespec *end) {
	return BILLION * (end->tv_sec - start->tv_sec) + end->tv_nsec - start->tv_nsec;
}

void pthread_mutex_lock_with_timer(pthread_mutex_t *m_lock) {
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	pthread_mutex_lock(m_lock);
	clock_gettime(CLOCK_MONOTONIC, &end);
	total_lock_acquisition_time += get_run_time(&start, &end);
}

void __sync_lock_spin_with_timer(int *s_lock, int value) {
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);
	while (__sync_lock_test_and_set(s_lock, value));
	clock_gettime(CLOCK_MONOTONIC, &end);
	total_lock_acquisition_time += get_run_time(&start, &end);
}

void mutex_list_insert(int index, SortedListElement_t *element) {
	pthread_mutex_lock_with_timer(&m_locks[index]);
	SortedList_insert(&lists[index], element);
	pthread_mutex_unlock(&m_locks[index]);
}

void spin_list_insert(int index, SortedListElement_t *element) {
	__sync_lock_spin_with_timer(&s_locks[index], 1);
	SortedList_insert(&lists[index], element);
	__sync_lock_release(&s_locks[index]);
}

int mutex_list_length(int index) {
	int length;
	pthread_mutex_lock_with_timer(&m_locks[index]);
	length = SortedList_length(&lists[index]);
	pthread_mutex_unlock(&m_locks[index]);
	return length;
}

int spin_list_length(int index) {
	int length;
	__sync_lock_spin_with_timer(&s_locks[index], 1);
	length = SortedList_length(&lists[index]);
	__sync_lock_release(&s_locks[index]);
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

void mutex_list_delete(int index, const char *key) {
	pthread_mutex_lock_with_timer(&m_locks[index]);
	handle_list_delete(&lists[index], key);
	pthread_mutex_unlock(&m_locks[index]);
}

void spin_list_delete(int index, const char *key) {
	__sync_lock_spin_with_timer(&s_locks[index], 1);
	handle_list_delete(&lists[index], key);
	__sync_lock_release(&s_locks[index]);
}

int get_num_elements() {
	return num_threads * num_iterations;
}

unsigned long hash(const char *str) {
	unsigned long sum = 0;
	int i;

	for (i = 0; i < key_length; i++) 
		sum += (i + 1) * str[i];

	return sum;
}

void thread_list_insert(int thread_index) {
	int i, list_index;

	for (i = thread_index; i < get_num_elements(); i += num_threads) {

		list_index = hash(elements[i].key) % num_lists;

		if (opt_sync) {

			if (sync_method == 'm') 
				mutex_list_insert(list_index, &elements[i]);
			else if (sync_method == 's')
				spin_list_insert(list_index, &elements[i]);

		} else {
			SortedList_insert(&lists[list_index], &elements[i]);
		}
		
	}
}

void thread_list_length() {
	int i, list_length;

	for (i = 0; i < num_lists; i++) {

		if (opt_sync) {

			if (sync_method == 'm') 
				list_length = mutex_list_length(i);
			else if (sync_method == 's')
				list_length = spin_list_length(i);

		}
		else {
			list_length = SortedList_length(&lists[i]);
		}

		if (list_length == -1) {
			fprintf(stderr, "Error getting the length of the list\n");
			exit(2);
		}

	}

}

void thread_list_delete(int thread_index) {
	int i, list_index;

	for (i = thread_index; i < get_num_elements(); i += num_threads) {

		list_index = hash(elements[i].key) % num_lists;

		if (opt_sync) {

			if (sync_method == 'm')
				mutex_list_delete(list_index, elements[i].key);
			else if (sync_method == 's')
				spin_list_delete(list_index, elements[i].key);

		}
		else {
			handle_list_delete(&lists[list_index], elements[i].key);
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

void free_lists() {
	free(lists);
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

void free_m_locks() {
	free(m_locks);
}

void free_s_locks() {
	free(s_locks);
}

int is_sync_method_valid(char sync_method) {
	return sync_method == 'm' || sync_method == 's';
}

void check_malloc_error(void *ptr) {
	if (ptr == NULL) {
		fprintf(stderr, "Error allocating memory\n");
		exit(1);
	}
}

void initialize_mutex_locks() {
	int i;

	m_locks = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t) * num_lists);
	check_malloc_error(m_locks);

	atexit(free_m_locks);

	for (i = 0; i < num_lists; i++) {
		if (pthread_mutex_init(&m_locks[i], NULL)) {
			fprintf(stderr, "Error initializing mutex\n");
			exit(1);
		}
	}
}

void initialize_spin_locks() {
	int i;

	s_locks = (int*) malloc(sizeof(int) * num_lists);\
	check_malloc_error(s_locks);

	atexit(free_s_locks);

	for (i = 0; i < num_lists; i++)
		s_locks[i] = 0;
}

void initialize_locks() {
	if (!opt_sync) return;

	if (sync_method == 'm') {
		initialize_mutex_locks();
	}
	else if (sync_method == 's') {
		initialize_spin_locks();
	}
}

void handle_sync_option(char *optarg) {
	opt_sync = 1;
	sync_method = optarg[0];
	if (!is_sync_method_valid(sync_method)) handle_unrecognized_argument();
}

void handle_options(int argc, char **argv) {

	int option;

	struct option options[] = {
		{"threads",    required_argument, NULL, 't'}, // takes a parameter for the number of parallel threads (--threads=#, default 1).
		{"iterations", required_argument, NULL, 'i'}, // takes a parameter for the number of iterations (--iterations=#, default 1).
		{"yield",      required_argument, NULL, 'y'}, // takes a parameter to enable (any combination of) optional critical section yields
		{"sync",       required_argument, NULL, 's'}, // synchronization option
		{"lists",      required_argument, NULL, 'l'}, // number of lists
		{0, 0, 0, 0}
	};

	while (1) {
		option = getopt_long(argc, argv, "", options, NULL);

		if (option == -1) break;

		switch (option) {
			case 't':
				num_threads = atoi(optarg);
				break;
			case 'i':
				num_iterations = atoi(optarg);
				break;
			case 'y':
				handle_yield_option(optarg);
				break;
			case 's':
				handle_sync_option(optarg);
				break;
			case 'l':
				num_lists = atoi(optarg);
				break;
			default:
				handle_unrecognized_argument();
		}


	}
}

long long get_average_time_per_operation(long long run_time, long long num_operations) {
	return run_time / num_operations;
}

long long get_num_operations(int num_threads, int num_iterations) {
	return num_threads * num_iterations * 3;
}

long long get_num_lock_operations(int num_threads, int num_iterations) {
	if (!opt_sync) return 0;

	// for each thread
	// insertion: num_iterations
	// length   : 1
	// deletion : num_iterations
	return num_threads * (num_iterations * 2 + 1);
}

long long get_average_wait_for_lock_time(long long num_lock_operations) {
	if (num_lock_operations == 0) return 0;
	return total_lock_acquisition_time / num_lock_operations;
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
	long long num_operations      = get_num_operations(num_threads, num_iterations);
	long long num_lock_operations = get_num_lock_operations(num_threads, num_iterations);
	long long run_time            = get_run_time(start, end);
	printf(
		"%s,%d,%d,%d,%lld,%lld,%lld,%lld\n", 
		get_tag(), 
		num_threads,
		num_iterations,
		num_lists,
		num_operations,
		run_time,
		get_average_time_per_operation(run_time, num_operations),
		get_average_wait_for_lock_time(num_lock_operations)
	);
}

void handle_segmentation_fault() {
	// do not use fprintf in signal handling
	write(2, "Segmentation Fault Detected\n", 28);
	exit(2);
}

void initialize_lists() {
	int i;

	lists = (SortedList_t*) malloc(sizeof(SortedList_t) * num_lists);
	check_malloc_error(lists);

	atexit(free_lists);

	for (i = 0; i < num_lists; i++) {
		lists[i].key  = NULL;
		lists[i].prev = &lists[i];
		lists[i].next = &lists[i];
	}
	
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
	tids = (pthread_t*) malloc(sizeof(pthread_t) * num_threads);
	check_malloc_error(tids);

	atexit(free_tids);
}

void initialize_thread_info() {
	int i;

	info = (struct ThreadInfo*) malloc(sizeof(struct ThreadInfo) * num_threads);
	check_malloc_error(info);

	atexit(free_info);

	for (i = 0; i < num_threads; i++)
		info[i].index = i;
}

void handle_threads() {
	int i;

	initialize_tids();
	
	initialize_thread_info();

	for (i = 0; i < num_threads; i++)
		safe_pthread_create(&tids[i], NULL, thread_function, (void*) &info[i]);

	for (i = 0; i < num_threads; i++)
		safe_pthread_join(tids[i], NULL);
}

int get_list_length() {
	int i, length;
	int total_length = 0;
	for (i = 0; i < num_lists; i++) {
		length = SortedList_length(&lists[i]);
		if (length == -1) {
			fprintf(stderr, "Error getting the length of the list\n");
			exit(2);
		}
		total_length += length;
	}
	return total_length;
}

void check_list_length() {
	if (get_list_length() != 0) {
		fprintf(stderr, "List length is not zero\n");
		exit(2);
	}
}

int main(int argc, char **argv) {

	struct timespec start, end;

	// set flags
	handle_options(argc, argv);

	// initializes locks if necessary
	initialize_locks();

	// initializes an empty list.
	initialize_lists();

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
	check_list_length();

	// prints to stdout a comma-separated-value (CSV) record
	handle_report(&start, &end);

	exit(0);

}


