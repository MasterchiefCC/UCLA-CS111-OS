// NAME: CHEN CHEN
// EMAIL: chenchenstudent@gmail.com
// ID: 004710308
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <mraa.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define A0 1
#define GPIO_50 60

#define PERIOD_OPT 'p'
#define SCALE_OPT 's'
#define LOG_OPT 'l'
#define Fahrenheit_scale 'F'
#define Centigrade_scale 'C'
#define Thermal 4275

#define set_memzero(buf, n) (void)memset(buf, 0, n)
#define set_strlen(s) strlen((const char *)s)
#define set_strcmp(s1, s2) strcmp((const char *)s1, (const char *)s2)
#define set_strncmp(s1, s2, n) strncmp((const char *)s1, (const char *)s2, n)
#define print_stdout(str) fprintf(stdout, "%s\n", str)

#define err_exit(msg)                                                          \
  fprintf(stderr, "%s error, errno = %d, message = %s\n", msg, errno,          \
          strerror(errno));                                                    \
  exit(EXIT_FAILURE)

#define fopen_check(file, dir)                                                 \
  file = fopen(dir, "w+");                                                     \
  if (file == NULL) {                                                          \
    err_exit("file_open");                                                     \
  }

#define malloc_check(prt, len)                                                 \
  prt = malloc(len);                                                           \
  if (prt == NULL) {                                                           \
    err_exit("malloc");                                                        \
  }

#define marr_init_aio_check(context)                                           \
  context = mraa_aio_init(A0);                                                 \
  if (context == NULL) {                                                       \
    mraa_deinit();                                                             \
    err_exit("marr aio init");                                                 \
  }

#define marr_gpio_init_check(context)                                          \
  context = mraa_gpio_init(GPIO_50);                                           \
  if (context == NULL) {                                                       \
    mraa_deinit();                                                             \
    err_exit("marr gpio init");                                                \
  }

#define print_log_file(str)                                                    \
  if (log_file != NULL) {                                                      \
    fprintf(log_file, "%s\n", str);                                            \
    fflush(log_file);                                                          \
  }

#define print_stdout_log(str)                                                  \
  print_stdout(str);                                                           \
  print_log_file(str);

#define read_temperature()                                                     \
  read_sensor = mraa_aio_read(sensor);                                         \
  R = 100000.0 * (1023.0 / ((float)read_sensor) - 1.0);                        \
  cur_temp = 1.0 / (log(R / 100000.0) / Thermal + 1 / 298.15) - 273.15;        \
  cur_temp = temp_scale == Fahrenheit_scale ? ret * 1.8 + 32.0 : ret;

#define update_time()                                                          \
  gettimeofday(&cur_clock, 0);                                                 \
                                                                               \
  if (print_IO && cur_clock.tv_sec >= next_time) {                             \
    read_temperature();                                                        \
    tmp = cur_temp * 10;                                                       \
                                                                               \
    cur_time = localtime(&cur_clock.tv_sec);                                   \
    set_memzero(buffer, sizeof(buffer));                                       \
                                                                               \
    sprintf(buffer, "%02d:%02d:%02d %d.%1d", cur_time->tm_hour,                \
            cur_time->tm_min, cur_time->tm_sec, tmp / 10, tmp % 10);           \
    print_stdout_log(buffer);                                                  \
                                                                               \
    next_time = cur_clock.tv_sec + time_slice;                                 \
  }

#define process_commands()                                                     \
  len = set_strlen(prt);                                                       \
  prt[len - 1] = '\0';                                                         \
  while (*prt == ' ' || *prt == '\t') {                                        \
    ++prt;                                                                     \
  }                                                                            \
                                                                               \
  if (set_strcmp(prt, "START") == 0) {                                         \
    print_log_file(prt);                                                       \
    print_IO = 1;                                                              \
  } else if (set_strcmp(prt, "STOP") == 0) {                                   \
    print_log_file(prt);                                                       \
    print_IO = 0;                                                              \
  } else if (set_strcmp(prt, "OFF") == 0) {                                    \
    print_log_file(prt);                                                       \
    shutdown_cmd();                                                            \
  } else if (set_strcmp(prt, "SCALE=F") == 0) {                                \
    print_log_file(prt);                                                       \
    temp_scale = 'F';                                                          \
  } else if (set_strcmp(prt, "SCALE=C") == 0) {                                \
    print_log_file(prt);                                                       \
    temp_scale = 'C';                                                          \
  } else if (set_strncmp(prt, "PERIOD=", set_strlen("PERIOD=")) == 0) {        \
    prt += 7;                                                                  \
    time_slice = atoi(prt);                                                    \
    print_log_file(buffer);                                                    \
  } else if (set_strncmp(prt, "LOG", set_strlen("LOG")) == 0) {                \
    print_log_file(prt);                                                       \
  } else {                                                                     \
    err_exit("Invalid command");                                               \
  }

extern void shutdown_cmd();