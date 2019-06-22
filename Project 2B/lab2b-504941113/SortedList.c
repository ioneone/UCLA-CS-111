// Name  : Junhong Wang
// ID    : 504941113
// Email : oneone@g.ucla.edu

#include "SortedList.h"

#include <stdlib.h>
#include <pthread.h>
#include <string.h>

void insert_before(SortedListElement_t *current, SortedListElement_t *element) {
	current->prev->next = element;
	element->prev       = current->prev;
	element->next       = current;
	current->prev       = element;
}

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
	if (list == NULL || element == NULL) return;

	SortedListElement_t *current = list->next;

	while (current != list) {
		if (strcmp(current->key, element->key) <= 0) break;
		current = current->next;
	}

	if (opt_yield & LOOKUP_YIELD) sched_yield();

	insert_before(current, element);
	
}

void remove(SortedListElement_t *element) {
	element->prev->next = element->next;
	element->next->prev = element->prev;
}

int SortedList_delete(SortedListElement_t *element) {
	if (element == NULL) return 1;

	if (element->next->prev != element || element->prev->next != element) return 1;

	if (opt_yield & DELETE_YIELD) sched_yield();

	remove(element);

	return 0;

}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
	if (list == NULL) return NULL;

	SortedListElement_t *current = list->next;

	while (current != list) {
		if (strcmp(current->key, key) == 0) return current;
		if (opt_yield & LOOKUP_YIELD) sched_yield();
		current = current->next;
	}

	return NULL;
}

int SortedList_length(SortedList_t *list) {
	if (list == NULL) return -1;

	int length = 0;
	SortedListElement_t *current = list->next;

	while (current != list) {
		if (current->prev->next != current || current->next->prev != current) return -1;
		if (opt_yield & LOOKUP_YIELD) sched_yield();
		current = current->next;
		length++;
	}

	return length;

}

