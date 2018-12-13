// NAME: CHEN CHEN
// EMAIL: chenchenstudent@gmail.com
// ID: 004710308
#include "lab4b.h"

FILE *log_file = NULL;

int main(int argc, char **argv) {
  int ret = 0, tmp, len, read_sensor, print_IO = 1, time_slice = 1;
  char c, buffer[512], *prt, temp_scale = Fahrenheit_scale;
  float R, cur_temp;

  time_t next_time = 0;

  mraa_gpio_context bnt; // button
  mraa_aio_context sensor;

  struct pollfd pfd;
  struct tm *cur_time;
  struct timeval cur_clock;

  static struct option long_options[] = {
      {"log", required_argument, NULL, LOG_OPT},
      {"scale", required_argument, NULL, SCALE_OPT},
      {"period", required_argument, NULL, PERIOD_OPT},
  };

  while ((ret = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
    switch (ret) {
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

    default:
      err_exit("Usage: ./lab4b [--log=FILE] [--scale=F||C] [--period=time]");
    }
  }

  // init
  marr_init_aio_check(sensor);
  marr_gpio_init_check(bnt);

  mraa_gpio_dir(bnt, MRAA_GPIO_IN);
  mraa_gpio_isr(bnt, MRAA_GPIO_EDGE_RISING, &shutdown_cmd, NULL);

  pfd.fd = STDIN_FILENO;
  pfd.events = POLLIN;

  for (;;) {
    update_time();

    ret = poll(&pfd, 1, 0);
    if (ret) {
      set_memzero(buffer, sizeof(buffer));
      fgets(buffer, 512, stdin);

      prt = buffer;

      process_commands();
    }
  }
  mraa_aio_close(sensor);
  mraa_gpio_close(bnt);
}

void shutdown_cmd() {
  char buffer[128];
  struct tm *close_time;
  struct timeval close_clock;

  gettimeofday(&close_clock, 0);

  close_time = localtime(&close_clock.tv_sec);

  sprintf(buffer, "%02d:%02d:%02d SHUTDOWN", close_time->tm_hour,
          close_time->tm_min, close_time->tm_sec);

  print_stdout_log(buffer);

  exit(EXIT_SUCCESS);
}