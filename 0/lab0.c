#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SET_SIGNAL 'S'
#define SET_SEGMENT 'F'
#define SET_INPUT 'I'
#define SET_OUTPUT 'O'
#define UNKNOWN '?'

// NAME: CHENCHEN
// EMAIL: chenchenstudent@gmail.com
// ID: 004710308

void dup_err_handler() {
  fprintf(stderr, "Error appears when dup the file descriptor.\n");
  exit(1);
}

void arg_err_handler(int argc, char** argv) {
  int i;
  fprintf(stderr, "Unrecognized Argument\n");
  for (i = 0; i < argc; ++i) {
    fprintf(stderr, " %s\n", argv[i]);
  }
  fprintf(stderr, "\n");
  exit(1);
}

void input_err_handler(const char* pathname) {
  fprintf(stderr, "unable to open input file %s\n", pathname);
  exit(2);
}

void output_err_handler(const char* pathname) {
  fprintf(stderr, "unable to open output file %s\n", pathname);
  exit(3);
}

void segment_err_handler() {
  fprintf(stderr, "caught and received SIGSEGV %s\n.", strerror(errno));
  exit(4);
}

int main(int argc, char** argv) {
  int tmp;
  char buffer;
  char *ifilepath = NULL, *ofilepath = NULL;
  struct option opts[] = {{"catch", no_argument, 0, SET_SIGNAL},
                          {"segfault", no_argument, 0, SET_SEGMENT},
                          {"input", required_argument, 0, SET_INPUT},
                          {"output", required_argument, 0, SET_OUTPUT},
                          {0, 0, 0, 0}};

  while ((tmp = getopt_long(argc, argv, "", opts, NULL)) != -1) {
    switch (tmp) {
      case SET_SIGNAL:
        signal(SIGSEGV, segment_err_handler);
        break;

      case SET_SEGMENT:
        raise(SIGSEGV);
        break;

      case SET_INPUT:

        ifilepath = optarg;
        break;

      case SET_OUTPUT:
        ofilepath = optarg;
        break;

      default:
        arg_err_handler(argc, argv);
    }
  }

  if (ifilepath != NULL) {
    tmp = open(ifilepath, O_RDONLY);

    if (tmp >= 0) {
      close(0);
      dup(tmp);
      close(tmp);
    } else {
      input_err_handler(ifilepath);
    }
  }

  if (ofilepath != NULL) {
    tmp = creat(ofilepath, 0666);

    if (tmp >= 0) {
      close(1);
      dup(tmp);
      close(tmp);
    } else {
      output_err_handler(ofilepath);
    }
  }

  while (1) {
    tmp = read(STDIN_FILENO, &buffer, sizeof(char));
    if (tmp <= 0) break;
    write(STDOUT_FILENO, &buffer, tmp);
  }

  exit(0);
}