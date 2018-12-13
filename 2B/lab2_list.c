// NAME: CHEN CHEN
// EMAIL: chenchenstudent@gmail.com
// ID: 004710308
#include "SortedList.h"
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum { none, mutex, spin } lock_type;

typedef struct {
  SortedList_t *list;
  int spin_lock;
  pthread_mutex_t mutex_lock;
} list_node;

typedef struct {
  list_node *list;
  int cnt;
} list_vector;

list_vector list_buffer;

long long iterations = 1;
lock_type cur_lock = none;
int opt_yield = 0;

#define set_memzero(buf, n) (void)memset((void *)buf, 0, (size_t)n)

#define set_hash(key, c) ((unsigned)key * 31 + c)

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

#define calloc_check(prt, num, size)                                           \
  prt = calloc(num, size);                                                     \
  if (prt == NULL) {                                                           \
    err_exit("calloc");                                                        \
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

#define thread_join_check(thread, retval)                                      \
  if (pthread_join(thread, retval) != 0) {                                     \
    err_exit("thread join");                                                   \
  }

#define LIST_OPT 'l'
#define SYNC_OPT 's'
#define YIELD_OPT 'y'
#define THREAD_OPT 't'
#define ITERATION_OPT 'i'

void sign_handler(int sig);
void *list_interface(void *arg);
unsigned long hash_key(const char *data, size_t len);
void *list_fun(void *arg, struct timespec *lock_time);

unsigned long hash(const char *key) {
  // Jenkins One At A Time Hash
  // https://en.wikipedia.org/wiki/Jenkins_hash_function
  uint32_t hash, i;
  unsigned int len = strlen(key);
  for (hash = i = 0; i < len; ++i) {
    hash += key[i];
    hash += (hash << 10);
    hash ^= (hash >> 6);
  }
  hash += (hash << 3);
  hash ^= (hash >> 11);
  hash += (hash << 15);
  return hash;
}

int main(int argc, char **argv) {
  int ret = 0, i, j, thrds_cnt = 1, list_cnt = 1, len, element_cnt;
  long long used_time, ops_cnt, avg_for_ops, lock_time, lock_ops_cnt,
      avg_for_lock;
  char c, *temp, *lock_str;
  void *temp_time = NULL;
  list_node *node_temp;
  SortedListElement_t *element;
  pthread_t *threads;

  struct timespec start, end, lock_wait;

  struct option long_options[] = {
      {"sync", required_argument, 0, SYNC_OPT},
      {"lists", required_argument, 0, LIST_OPT},
      {"yield", required_argument, 0, YIELD_OPT},
      {"threads", required_argument, 0, THREAD_OPT},
      {"iterations", required_argument, 0, ITERATION_OPT}};

  signal_check(SIGSEGV, sign_handler);

  while ((ret = getopt_long(argc, argv, "t:s:", long_options, NULL)) != -1) {

    switch (ret) {
    case 't':
      thrds_cnt = atoi(optarg);
      break;

    case 'i':
      iterations = atoi(optarg);
      break;

    case 'l':
      list_cnt = atoi(optarg);
      break;

    case 'y':
      for (i = 0; i < (int)strlen(optarg); ++i) {
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

    case 's':
      if (strcmp("m", optarg) == 0) {
        cur_lock = mutex;
      } else if (strcmp("s", optarg) == 0) {
        cur_lock = spin;
      } else {
        goto error_args;
      }
      break;

    default:
      goto error_args;
      break;

    error_args:
      err_exit("Usage: ./lab2_list [--threads=num] [--iterations=num] "
               "[--yield={dli}] [--sync={m,s}]");
    }
  }

  list_buffer.cnt = list_cnt;
  calloc_check(list_buffer.list, list_cnt, sizeof(*list_buffer.list));

  for (i = 0; i < list_cnt; ++i) {
    node_temp = (list_buffer.list + i);

    len = sizeof(SortedList_t);
    malloc_check(node_temp->list, len);

    node_temp->list->key = NULL;
    node_temp->list->next = node_temp->list->prev = node_temp->list;

    ret = pthread_mutex_init(&node_temp->mutex_lock, NULL);
    if (ret != 0) {
      err_exit("pthread_mutex_init");
    }
    node_temp->spin_lock = 0;
  }

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

  len = thrds_cnt * sizeof(pthread_t);
  malloc_check(threads, len);

  set_memzero(&lock_wait, sizeof(lock_wait));

  get_time_check(start);

  for (i = 0; i < thrds_cnt; ++i) {
    thread_create_check(threads + i, list_interface, element + i * iterations);
  }

  for (i = 0; i < thrds_cnt; ++i) {
    thread_join_check(threads[i], &temp_time);
    if (temp_time) {
      lock_wait.tv_sec += ((struct timespec *)temp_time)->tv_sec;
      lock_wait.tv_nsec += ((struct timespec *)temp_time)->tv_nsec;
    }
  }

  get_time_check(end);

  // TODO:
  // free
  free(threads);
  free(element);

  used_time =
      (end.tv_sec - start.tv_sec) * 1000000000L + (end.tv_nsec - start.tv_nsec);

  ops_cnt = thrds_cnt * iterations * 3;

  avg_for_ops = used_time / ops_cnt;

  lock_time = lock_wait.tv_nsec + 1000000000L * (long long)lock_wait.tv_sec;

  lock_ops_cnt = thrds_cnt * (3 * iterations + 1);

  avg_for_lock = lock_time / lock_ops_cnt;

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

  lock_str = cur_lock == none ? "none" : cur_lock == mutex ? "m" : "s";
  printf("list-%s-%s,%d,%lld,%d,%lld,%lld,%lld,%lld\n", temp, lock_str,
         thrds_cnt, iterations, list_cnt, ops_cnt, used_time, avg_for_ops,
         avg_for_lock);
  exit(EXIT_SUCCESS);
}

void sign_handler(int sig) {
  if (sig == SIGSEGV) {
    err_exit_other("Seg fault caught");
  }
}

void *list_interface(void *arg) {
  struct timespec *lock_time = NULL;
  calloc_check(lock_time, 1, sizeof(*lock_time));

  return list_fun(arg, lock_time);
}

void spin_lock(unsigned long idx, struct timespec *lock_time) {
  struct timespec start, end;
  get_time_check(start);
  while (__sync_lock_test_and_set(&list_buffer.list[idx].spin_lock, 1) == 1)
    ;
  get_time_check(end);
  lock_time->tv_sec += end.tv_sec - start.tv_sec;
  lock_time->tv_nsec += end.tv_nsec - start.tv_nsec;
}

void mutex_lock(unsigned long idx, struct timespec *lock_time) {
  struct timespec start, end;
  get_time_check(start);
  pthread_mutex_lock(&list_buffer.list[idx].mutex_lock);
  get_time_check(end);
  lock_time->tv_sec += end.tv_sec - start.tv_sec;
  lock_time->tv_nsec += end.tv_nsec - start.tv_nsec;
}

#define start_lock(idx, lock_time)                                             \
  switch (cur_lock) {                                                          \
  case spin:                                                                   \
    spin_lock(idx, lock_time);                                                 \
    break;                                                                     \
                                                                               \
  case mutex:                                                                  \
    mutex_lock(idx, lock_time);                                                \
    break;                                                                     \
                                                                               \
  default:                                                                     \
    break;                                                                     \
  }

#define start_unlock(idx)                                                      \
  switch (cur_lock) {                                                          \
  case spin:                                                                   \
    __sync_lock_release((&list_buffer.list[idx].spin_lock));                   \
    break;                                                                     \
                                                                               \
  case mutex:                                                                  \
    pthread_mutex_unlock(&list_buffer.list[idx].mutex_lock);                   \
    break;                                                                     \
                                                                               \
  default:                                                                     \
    break;                                                                     \
  }

void *list_fun(void *arg, struct timespec *lock_time) {
  SortedListElement_t *cur_list = arg, *target;
  int i, total = list_buffer.cnt, temp_len, len, ret;
  unsigned long idx;
  //struct timespec start, end;

  // insert
  for (i = 0; i < iterations; ++i) {
    len = strlen((cur_list + i)->key);
    idx = hash_key((cur_list + i)->key, len) % total;

    start_lock(idx, lock_time);

    SortedList_insert(list_buffer.list[idx].list, cur_list + i);

    start_unlock(idx);
  }

  // len
  len = 0;
  for (i = 0; i < list_buffer.cnt; ++i) {

    start_lock(i, lock_time);

    temp_len = SortedList_length(list_buffer.list[i].list);
    if (temp_len == -1) {
      err_exit_other("SortedList_length");
    }

    len += temp_len;

    start_unlock(i);
  }

  // lookup
  // delete
  for (i = 0; i < iterations; ++i) {
    len = strlen((cur_list + i)->key);
    idx = hash_key((cur_list + i)->key, len) % total;

    start_lock(idx, lock_time);

    target = SortedList_lookup(list_buffer.list[idx].list, (cur_list + i)->key);
    if (target == NULL) {
      err_exit_other("SortedList_lookup");
    }

    ret = SortedList_delete(target);
    if (ret != 0) {
      err_exit_other("SortedList_delete");
    }

    start_unlock(idx);
  }

  return lock_time;
}

unsigned long hash_key(const char *data, size_t len) {

  unsigned i, key;

  key = 0;

  for (i = 0; i < len; ++i) {
    key = set_hash(key, data[i]);
  }

  return key;
}