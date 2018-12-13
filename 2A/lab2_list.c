// NAME: CHEN CHEN
// EMAIL: chenchenstudent@gmail.com
// ID: 004710308
#include "SortedList.h"
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum { none, mutex, spin, atomic } lock_type;
typedef void *(*list_handler_ptr)(void *arg);

long long iterations;
list_handler_ptr list_handler;
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
int spin_lock = 0, opt_yield = 0;
SortedList_t *head;

#define set_strcmp(s1, s2) strcmp((const char *)s1, (const char *)s2)

#define err_exit(msg)                                                          \
  fprintf(stderr, "%s error, errno = %d, message = %s\n", msg, errno,          \
          strerror(errno));                                                    \
  exit(EXIT_FAILURE)

#define err_exit_other(msg)                                                    \
  fprintf(stderr, "%s error, errno = %d, message = %s\n", msg, errno,          \
          strerror(errno));                                                    \
  exit(2)

#define malloc_check(prt, len)                                                 \
  prt = malloc(len);                                                           \
  if (prt == NULL) {                                                           \
    err_exit("malloc");                                                        \
  }

#define signal_check(type, handler)                                            \
  if (signal(type, handler) == SIG_ERR) {                                      \
    err_exit("signal");                                                        \
  }

#define get_time_check(time)                                                   \
  if (clock_gettime(CLOCK_MONOTONIC, &time) < 0) {                             \
    err_exit("clock_gettime");                                                 \
  }

#define thread_create_check(thread, fun, cnt)                                  \
  if (pthread_create(thread, NULL, fun, cnt) != 0) {                           \
    err_exit("thread create");                                                 \
  }

#define thread_join_check(thread)                                              \
  if (pthread_join(thread, NULL) != 0) {                                       \
    err_exit("thread join");                                                   \
  }

#define SYNC_OPT 's'
#define YIELD_OPT 'y'
#define THREAD_OPT 't'
#define ITERATION_OPT 'i'
#define NANOSECONDS_ELAPSED 1000000000L

static void *list(void *arg);
static void *mutex_lock_list(void *arg);
static void *spin_lock_list(void *arg);
void sign_handler(int sig);

int main(int argc, char **argv) {
  int ret;
  unsigned i, j, thrds_cnt, len;
  char c;
  long long element_cnt, used_time, avg_time, total_ops;

  lock_type cur_lock = none;
  struct timespec start, end;

  SortedListElement_t *element;
  pthread_t *threads;
  char *temp, *lock_type;

  struct option opt[] = {
      {"yield", required_argument, 0, YIELD_OPT},
      {"sync", required_argument, 0, SYNC_OPT},
      {"threads", required_argument, 0, THREAD_OPT},
      {"iterations", required_argument, 0, ITERATION_OPT},
  };

  signal_check(SIGSEGV, sign_handler);

  while ((ret = getopt_long(argc, argv, "t:s:", opt, NULL)) != -1) {
    switch (ret) {
    case YIELD_OPT:
      for (i = 0; i < strlen(optarg); ++i) {
        c = optarg[i];

        switch (c) {
        case 'i':
          opt_yield |= INSERT_YIELD;
          break;

        case 'd':
          opt_yield |= DELETE_YIELD;
          break;

        case 'l':
          opt_yield |= LOOKUP_YIELD;
          break;

        default:
          goto error_args;
        }
      }
      break;

    case THREAD_OPT:
      thrds_cnt = atoi(optarg);
      break;

    case ITERATION_OPT:
      iterations = atoi(optarg);
      break;

    case SYNC_OPT:
      if (set_strcmp("m", optarg) == 0) {
        cur_lock = mutex;
      } else if (set_strcmp("s", optarg) == 0) {
        cur_lock = spin;
      } else {
        goto error_args;
      }
      break;

    default:
      goto error_args;
      break;

    error_args:
      err_exit("Usage: ./lab2_list [--threads=t] [--iterations=i] "
               "[--yield={dli}] [--sync={m,s}]");
    }
  }

  switch (cur_lock) {
  case none:
    list_handler = list;
    lock_type = "none";
    break;

  case mutex:
    list_handler = mutex_lock_list;
    lock_type = "m";
    break;

  case spin:
    list_handler = spin_lock_list;
    lock_type = "s";
    break;

  default:
    goto error_args;
  }

  len = sizeof(SortedList_t);
  malloc_check(head, len);
  head->key = NULL;
  head->next = head->prev = head;

  element_cnt = thrds_cnt * iterations;
  len = element_cnt * sizeof(SortedListElement_t);
  malloc_check(element, len);

  srand(time(NULL));

  for (i = 0; i < element_cnt; ++i) {
    len = rand() % 15 + 1;
    malloc_check(temp, (len + 1) * sizeof(char));

    for (j = 0; j < len; ++j) {
      temp[j] = 'a' + rand() % 26;
    }
    temp[j] = '\0';
    element[i].key = (const char *)temp;
  }

  malloc_check(threads, thrds_cnt * sizeof(pthread_t));

  get_time_check(start);

  for (i = 0; i < thrds_cnt; ++i) {
    thread_create_check(threads + i, list_handler, element + i * iterations);
  }

  for (i = 0; i < thrds_cnt; ++i) {
    thread_join_check(threads[i]);
  }

  get_time_check(end);

  if (SortedList_length(head) != 0) {
    err_exit_other("list length end with not 0");
  }

  used_time = (end.tv_sec - start.tv_sec) * NANOSECONDS_ELAPSED +
              (end.tv_nsec - start.tv_nsec);

  total_ops = thrds_cnt * iterations * 3;

  avg_time = used_time / total_ops;

  if (opt_yield == 0) {
    temp = "none";
  } else if ((opt_yield & INSERT_YIELD) && (opt_yield & DELETE_YIELD) &&
             (opt_yield & LOOKUP_YIELD)) {
    temp = "idl";
  } else if ((opt_yield & INSERT_YIELD) && (opt_yield & DELETE_YIELD)) {
    temp = "id";
  } else if ((opt_yield & INSERT_YIELD) && (opt_yield & LOOKUP_YIELD)) {
    temp = "il";
  } else if (opt_yield & INSERT_YIELD) {
    temp = "i";
  } else if ((opt_yield & DELETE_YIELD) && (opt_yield & LOOKUP_YIELD)) {
    temp = "dl";
  } else if (opt_yield & DELETE_YIELD) {
    temp = "d";
  } else {
    temp = "l";
  }

  free(threads);
  free(element);
  // TODO:
  // free key
  free(head);

  printf("list-%s-%s,%d,%lld,%d,%lld,%lld,%lld\n", temp, lock_type, thrds_cnt,
         iterations, 1, total_ops, used_time, avg_time);

  exit(EXIT_SUCCESS);
}

static void *list(void *arg) {

  int i, ret;
  SortedListElement_t *temp = arg, *target;

  for (i = 0; i < iterations; ++i) {
    SortedList_insert(head, temp + i);
  }

  ret = SortedList_length(head);
  if (ret == -1) {
    err_exit_other("insert list");
  }

  for (i = 0; i < iterations; ++i) {
    target = SortedList_lookup(head, temp[i].key);
    if (target == NULL) {
      err_exit_other("SortedList_lookup cann't find target");
    }

    ret = SortedList_delete(target);
    if (ret != 0) {
      err_exit_other("SortedList_delete cann't delete target");
    }
  }

  return NULL;
}

static void *mutex_lock_list(void *arg) {

  int i, ret;
  SortedListElement_t *temp = arg, *target;

  for (i = 0; i < iterations; ++i) {
    pthread_mutex_lock(&mutex_lock); // lock
    // critial area
    SortedList_insert(head, temp + i);

    pthread_mutex_unlock(&mutex_lock); // unlock
  }

  pthread_mutex_lock(&mutex_lock); // lock
  // critial area
  ret = SortedList_length(head);

  pthread_mutex_unlock(&mutex_lock); // unlock

  if (ret == -1) {
    err_exit_other("insert list");
  }

  for (i = 0; i < iterations; ++i) {
    pthread_mutex_lock(&mutex_lock); // lock
    // critial area

    target = SortedList_lookup(head, temp[i].key);
    if (target == NULL) {
      err_exit_other("SortedList_lookup cann't find target");
    }

    ret = SortedList_delete(target);
    if (ret != 0) {
      err_exit_other("SortedList_delete cann't delete target");
    }

    pthread_mutex_unlock(&mutex_lock); // unlock
  }

  return NULL;
}

static void *spin_lock_list(void *arg) {

  int i, ret;
  SortedListElement_t *temp = arg, *target;

  for (i = 0; i < iterations; ++i) {
    while (__sync_lock_test_and_set(&spin_lock, 1) == 1) // lock
      ;
    // critial area
    SortedList_insert(head, temp + i);

    __sync_lock_release(&spin_lock); // unlock
  }

  while (__sync_lock_test_and_set(&spin_lock, 1) == 1) // lock
    ;
  // critial area

  ret = SortedList_length(head);

  __sync_lock_release(&spin_lock); // unlock

  if (ret == -1) {
    err_exit_other("insert list");
  }

  for (i = 0; i < iterations; ++i) {
    while (__sync_lock_test_and_set(&spin_lock, 1) == 1) // lock
      ;
    // critial area

    target = SortedList_lookup(head, temp[i].key);
    if (target == NULL) {
      err_exit_other("SortedList_lookup cann't find target");
    }

    ret = SortedList_delete(target);
    if (ret != 0) {
      err_exit_other("SortedList_delete cann't delete target");
    }

    __sync_lock_release(&spin_lock); // unlock
  }

  return NULL;
}

void sign_handler(int sig) {
  if (sig == SIGSEGV) {
    err_exit_other("Seg fault caught");
  }
}