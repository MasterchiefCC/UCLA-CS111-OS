// NAME: CHEN CHEN
// EMAIL: chenchenstudent@gmail.com
// ID: 004710308
#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef void (*add_handler_ptr)(long long *prt, long long value);
typedef enum { none, mutex, spin, atomic } lock_type;
typedef intptr_t flag_t;

flag_t YIELD_FLAG = 0;
long long iterations;
pthread_mutex_t mutex_lock = PTHREAD_MUTEX_INITIALIZER;
int spin_lock = 0;
add_handler_ptr add_handler;

#define set_strcmp(s1, s2) strcmp((const char *)s1, (const char *)s2)

#define err_exit(msg)                                                          \
  fprintf(stderr, "%s error, errno = %d, message = %s\n", msg, errno,          \
          strerror(errno));                                                    \
  exit(EXIT_FAILURE)

#define malloc_check(prt, len)                                                 \
  prt = malloc(len);                                                           \
  if (prt == NULL) {                                                           \
    err_exit("malloc");                                                        \
  }

#define get_time_check(time)                                                   \
  if (clock_gettime(CLOCK_MONOTONIC, &time) < 0) {                             \
    err_exit("clock_gettime");                                                 \
  }

#define thread_create_check(thread, fun, cnt)                                  \
  if (pthread_create(thread, NULL, fun, &cnt) != 0) {                          \
    err_exit("thread create");                                                 \
  }

#define thread_join_check(thread)                                              \
  if (pthread_join(thread, NULL) != 0) {                                       \
    err_exit("thread join");                                                   \
  }

#define signal_check(type, handler)                                            \
  if (signal(type, handler) == SIG_ERR) {                                      \
    err_exit("signal");                                                        \
  }

#define SYNC_OPT 's'
#define YIELD_OPT 'y'
#define DEBUG_OPT 'd'
#define THREAD_OPT 't'
#define ITERATION_OPT 'i'
#define NANOSECONDS_ELAPSED 1000000000L

static void add(long long *prt, long long value);
static void mutex_lock_add(long long *prt, long long value);
static void spin_lock_add(long long *prt, long long value);
static void atomic_add(long long *prt, long long value);
void *add_interface(void *cnt);
void sign_handler(int sig);

int main(int argc, char **argv) {
  int ret, cnt_thrds, i;
  char *add_type;
  long long cnt, used_time, total_ops;
  lock_type cur_lock = none;

  struct timespec start, end;
  pthread_t *threads;

  struct option opt[] = {
      {"yield", no_argument, 0, YIELD_OPT},
      {"debug", no_argument, 0, DEBUG_OPT},
      {"sync", required_argument, 0, SYNC_OPT},
      {"threads", required_argument, 0, THREAD_OPT},
      {"iterations", required_argument, 0, ITERATION_OPT},
  };

  signal_check(SIGSEGV, sign_handler);

  while ((ret = getopt_long(argc, argv, "t:i:", opt, NULL)) != -1) {
    switch (ret) {
    case YIELD_OPT:
      YIELD_FLAG = 1;
      break;

    case ITERATION_OPT:
      iterations = strtoll(optarg, NULL, 10);
      break;

    case THREAD_OPT:
      cnt_thrds = atoi(optarg);
      break;

    case SYNC_OPT:
      if (strlen(optarg) == 1) {
        if (set_strcmp("m", optarg) == 0) {
          cur_lock = mutex;
        } else if (set_strcmp("s", optarg) == 0) {
          cur_lock = spin;
        } else if (set_strcmp("c", optarg) == 0) {
          cur_lock = atomic;
        } else {
          goto error_args;
        }
      } else {
        goto error_args;
      }
      break;

    default:
      goto error_args;
      break;

    error_args:
      err_exit("Usage: ./lab2_add [--threads=num_threads] "
               "[--iterations=num_iterations] [--yield] [--sync={m,s,c}]\n");
    }
  }

  switch (cur_lock) {
  case none:
    add_handler = add;
    add_type = "none";
    break;

  case mutex:
    add_handler = mutex_lock_add;
    add_type = "m";
    break;

  case spin:
    add_handler = spin_lock_add;
    add_type = "s";
    break;

  case atomic:
    add_handler = atomic_add;
    add_type = "c";
    break;

  default:
    goto error_args;
  }

  get_time_check(start);

  malloc_check(threads, cnt_thrds * (sizeof(pthread_t)));

  for (i = 0; i < cnt_thrds; ++i) {
    thread_create_check(threads + i, add_interface, cnt);
  }

  for (i = 0; i < cnt_thrds; ++i) {
    thread_join_check(threads[i]);
  }

  get_time_check(end);

  used_time = (end.tv_sec - start.tv_sec) * NANOSECONDS_ELAPSED +
              (end.tv_nsec - start.tv_nsec);

  total_ops = cnt_thrds * iterations * 2;

  free(threads);

  printf("add%s-%s,%d,%lld,%lld,%lld,%lld,%lld\n",
         (YIELD_FLAG == 1 ? "-yield" : ""), add_type, cnt_thrds, iterations,
         total_ops, used_time, used_time / total_ops, cnt);

  exit(EXIT_SUCCESS);
  // TODO:
  // error:
  //   free(threads);
  //   err_exit("threads create, or threads join");
}

static void add(long long *prt, long long value) {
  long long tmp = *prt + value;
  if (YIELD_FLAG == 1) {
    // TODO:
    // system_call #define
    sched_yield();
  }

  *prt = tmp;
}

static void mutex_lock_add(long long *prt, long long value) {
  // TODO:
  //?
  pthread_mutex_lock(&mutex_lock);
  add(prt, value);
  pthread_mutex_unlock(&mutex_lock);
}

static void spin_lock_add(long long *prt, long long value) {
  while (__sync_lock_test_and_set(&spin_lock, 1) == 1)
    ;
  add(prt, value);
  __sync_lock_release(&spin_lock);
}

static void atomic_add(long long *prt, long long value) {
  long long temp, ret;

  do {
    temp = *prt;
    ret = *prt + value;

    if (YIELD_FLAG == 1) {
      // TODO:
      sched_yield();
    }

  } while (temp != __sync_val_compare_and_swap(prt, temp, ret));
}

void *add_interface(void *cnt) {
  int i;
  for (i = 0; i < iterations; ++i) {
    add_handler((long long *)cnt, 1);
  }

  for (i = 0; i < iterations; ++i) {
    add_handler((long long *)cnt, -1);
  }

  return NULL;
}

void sign_handler(int sig) {
  if (sig == SIGSEGV) {
    err_exit("Seg fault caught");
  }
}