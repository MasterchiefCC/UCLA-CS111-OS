// NAME: CHEN CHEN
// EMAIL: chenchenstudent@gmail.com
// ID: 004710308
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "lab4c.h"

FILE *log_file = NULL;

fd_t sock_fd;
int time_slice = 1, port, tmp, print_IO = 1, ret;
char c, *id, *name, buffer[512], temp_scale = Fahrenheit_scale;

struct hostent *h;
struct pollfd pfd;

struct tm *cur_time;
time_t next_time = 0;
struct timeval cur_clock;

mraa_aio_context sensor;

SSL_CTX *ssl_ctx = NULL;
SSL *ssl_fd = NULL;

void send_msg(char *str, int target) {
  if (target == TO_SERVER) {
    char out[200];
    sprintf(out, "%s\n", str);
    SSL_write(ssl_fd, out, strlen(out) + 1);
  }
  fprintf(stderr, "%s\n", str);
  fprintf(log_file, "%s\n", str);
  fflush(log_file);
}

int main(int argc, char **argv) {

  struct sockaddr_in sin;

  static struct option long_options[] = {
      {"id", required_argument, NULL, ID_OPT},
      {"log", required_argument, NULL, LOG_OPT},
      {"host", required_argument, NULL, HOST_OPT},
      {"scale", required_argument, NULL, SCALE_OPT},
      {"period", required_argument, NULL, PERIOD_OPT},
  };

  while ((ret = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
    switch (ret) {
    case ID_OPT:
      id = optarg;
      break;

    case HOST_OPT:
      name = optarg;
      break;

    case PERIOD_OPT:
      time_slice = atoi(optarg);
      break;

    case LOG_OPT:
      fopen_check(log_file, optarg);
      break;

    case SCALE_OPT:
      c = optarg[0];
      if (c == Fahrenheit_scale || c == Centigrade_scale) {
        temp_scale = c;
        break;
      }
      // fall through
    err_arg:
    default:
      err_exit("Usage: ./lab4b [--log=FILE] [--scale=F||C] [--period=time] "
               "[--id=xxxxxxxxx] [--host=xxx]");
    }
  }

  if (optind < argc) {
    port = atoi(argv[optind]);
    if (port <= 0) {
      goto err_arg;
    }
  }

  if (set_strlen(name) == 0 || log_file == NULL || set_strlen(id) != 9) {
    goto err_arg;
  }

  // socket setting start

  socket_check(sock_fd);

  h = gethostbyname(name);

  set_memzero(&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);

  set_memcpy(&sin.sin_addr.s_addr, h->h_addr, h->h_length);

  if (connect(sock_fd, (const struct sockaddr *)&sin, sizeof(sin)) < 0) {
    err_exit("connect");
  }

  // end to socket set

  // SSL
  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();

  ssl_ctx = SSL_CTX_new(TLSv1_client_method());
  if (ssl_ctx == NULL) {
    fprintf(stderr, "Unable to get SSL context\n");
    exit(2);
  }
  ssl_fd = SSL_new(ssl_ctx);
  if (ssl_fd == NULL) {
    fprintf(stderr, "Unable to complete SSL setup\n");
    exit(2);
  }
  if (!SSL_set_fd(ssl_fd, sock_fd)) {
    fprintf(stderr, "Unable to associate fd w/SSL\n");
    exit(2);
  }
  if (SSL_connect(ssl_fd) != 1) {
    fprintf(stderr, "SSL Connection rejected\n");
    exit(2);
  }

  set_memzero(buffer, 512);
  sprintf(buffer, "ID=%s", id);
  send_msg(buffer, TO_SERVER);

  marr_init_aio_check(sensor);

  pfd.fd = sock_fd;
  pfd.events = POLLIN;

  for (;;) {
    update_time();

    ret = poll(&pfd, 1, 0);

    if (ret) {
      read_command_from_server(sock_fd);
    }
  }

  mraa_aio_close(sensor);
}

void process_command_from_server(char *prt) {
  size_t len;

  len = set_strlen(prt);
  while (*prt == ' ' || *prt == '\t') {
    ++prt;
  }

  if (set_strcmp(prt, "START") == 0) {
    send_msg(prt, NO_SERVER);
    print_IO = 1;
  } else if (set_strcmp(prt, "STOP") == 0) {
    send_msg(prt, NO_SERVER);
    print_IO = 0;
  } else if (set_strcmp(prt, "OFF") == 0) {
    send_msg(prt, NO_SERVER);
    shutdown_cmd();
  } else if (set_strcmp(prt, "SCALE=F") == 0) {
    send_msg(prt, NO_SERVER);
    temp_scale = 'F';
  } else if (set_strcmp(prt, "SCALE=C") == 0) {
    send_msg(prt, NO_SERVER);
    temp_scale = 'C';
  } else if (set_strncmp(prt, "PERIOD=", set_strlen("PERIOD=")) == 0) {
    send_msg(prt, NO_SERVER);
    prt += 7;
    time_slice = atoi(prt);
  } else if (set_strncmp(prt, "LOG", set_strlen("LOG")) == 0) {
    send_msg(prt, NO_SERVER);
  } else {
    err_exit("Invalid command");
  }
}

void shutdown_cmd() {
  char buffer[128];
  struct tm *close_time;
  struct timeval close_clock;

  gettimeofday(&close_clock, 0);

  close_time = localtime(&close_clock.tv_sec);

  sprintf(buffer, "%02d:%02d:%02d SHUTDOWN", close_time->tm_hour,
          close_time->tm_min, close_time->tm_sec);

  send_msg(buffer, TO_SERVER);

  exit(EXIT_SUCCESS);
}

void read_command_from_server(fd_t sock_fd) {
  int ret = 0;
  char *start, *end;

  set_memzero(buffer, 512);

  ret = SSL_read(ssl_fd, buffer, 512);

  start = buffer;

  while (start < &buffer[ret]) {
    end = start;

    while (end < &buffer[ret] && *end != '\n') {
      ++end;
    }

    *end = 0;

    process_command_from_server(start);

    start = &end[1];
  }
}

double read_temperature() {
  int read_sensor = mraa_aio_read(sensor);
  float R = 100000.0 * (1023.0 / ((float)read_sensor) - 1.0);
  float cur_temp = 1.0 / (log(R / 100000.0) / Thermal + 1 / 298.15) - 273.15;
  return temp_scale == Fahrenheit_scale ? ret * 1.8 + 32.0 : ret;
}

void update_time() {

  gettimeofday(&cur_clock, 0);

  if (print_IO == 1 && cur_clock.tv_sec >= next_time) {
    double cur_temp = read_temperature();
    int tmp = cur_temp * 10;

    cur_time = localtime(&cur_clock.tv_sec);
    set_memzero(buffer, sizeof(buffer));

    sprintf(buffer, "%02d:%02d:%02d %d.%1d", cur_time->tm_hour,
            cur_time->tm_min, cur_time->tm_sec, tmp / 10, tmp % 10);

    send_msg(buffer, TO_SERVER);

    next_time = cur_clock.tv_sec + time_slice;
  }
}