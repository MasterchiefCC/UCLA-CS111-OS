// NAME: CHEN CHEN
// EMAIL: chenchenstudent@gmail.com
// ID: 004710308
#include <ctype.h>
#include <fcntl.h>
#include <getopt.h>
#include <math.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <netdb.h>
#include <netinet/in.h>

#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

typedef int fd_t;

#define PERIOD_OPT 'p'
#define SCALE_OPT 's'
#define LOG_OPT 'l'
#define ID_OPT 'i'
#define HOST_OPT 'h'

#define Fahrenheit_scale 'F'
#define Centigrade_scale 'C'

#define TO_SERVER 6655
#define NO_SERVER 1551

#define Thermal 4275

#define A0 1

#define set_memzero(buf, n) (void)memset(buf, 0, n)
#define set_strlen(s) strlen((const char *)s)
#define set_strcmp(s1, s2) strcmp((const char *)s1, (const char *)s2)
#define set_strncmp(s1, s2, n) strncmp((const char *)s1, (const char *)s2, n)
#define print_stdout(str) fprintf(stdout, "%s\n", str)

#define set_memcpy(dst, src, n)                                                \
  (void)memcpy((void *)dst, (void *)src, (size_t)n)

#define err_exit(msg)                                                          \
  fprintf(stderr, "%s error, errno = %d, message = %s\n", msg, errno,          \
          strerror(errno));                                                    \
  exit(EXIT_FAILURE)

#define socket_check(sock_fd)                                                  \
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);                                   \
  if (sock_fd < 0) {                                                           \
    err_exit("open socket");                                                   \
  }

#define marr_init_aio_check(context)                                           \
  context = mraa_aio_init(A0);                                                 \
  if (context == NULL) {                                                       \
    mraa_deinit();                                                             \
    err_exit("marr aio init");                                                 \
  }

#define fopen_check(file, dir)                                                 \
  file = fopen(dir, "w+");                                                     \
  if (file == NULL) {                                                          \
    err_exit("file_open");                                                     \
  }


void read_command_from_server(fd_t sock_fd);
void process_command_from_server(char *prt);
void send_msg(char *str, int target);
void shutdown_cmd();
double read_temperature();
void update_time();
