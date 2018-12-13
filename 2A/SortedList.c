// NAME: CHEN CHEN
// EMAIL: chenchenstudent@gmail.com
// ID: 004710308
#include "SortedList.h"
#include <sched.h>
#include <stdio.h>
#include <string.h>

#define set_strcmp(s1, s2) strcmp((const char *)s1, (const char *)s2)

void SortedList_insert(SortedList_t *list, SortedListElement_t *element) {
  SortedListElement_t *cur;

  if (list == NULL || element == NULL) {
    return;
  }

  cur = list->next;

  while (cur != list) {

    if (strcmp(element->key, cur->key) <= 0) {
      break;
    }

    cur = cur->next;
  }

  if (opt_yield & INSERT_YIELD)
    sched_yield();

  element->next = cur;
  element->prev = cur->prev;
  cur->prev->next = element;
  cur->prev = element;
}

int SortedList_delete(SortedListElement_t *element) {
  if (element == NULL || element->next->prev != element->prev->next)
    return 1;

  if (opt_yield & DELETE_YIELD) {
    sched_yield();
  }

  element->prev->next = element->next;
  element->next->prev = element->prev;
  return 0;
}

SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key) {
  SortedListElement_t *temp, *head;

  if (list == NULL || key == NULL) {
    return NULL;
  }

  temp = list->next, head = list;

  while (temp != head) {

    if (set_strcmp(temp->key, key) == 0) {
      return temp;
    }

    if (opt_yield & LOOKUP_YIELD) {
      sched_yield();
    }

    temp = temp->next;
  }

  return NULL;
}

int SortedList_length(SortedList_t *list) {
  SortedListElement_t *temp, *head;
  if (list == NULL) {
    return -1;
  }

  int cnt = 0;

   temp= list->next, head = list;

  while (temp != head) {
    if (temp->prev->next != temp || temp->next->prev != temp) {
      return -1;
    }

    ++cnt;

    if (opt_yield & LOOKUP_YIELD) {
      sched_yield();
    }

    temp = temp->next;
  }

  return cnt;
}
