#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

int shell_opt = 0, debug_opt = 0, ret = 0, to_child_pipe[2], from_child_pipe[2];
char *path = NULL;
pid_t cpid;
struct termios canonical_input_mode;

#define SHELL 's'
#define DEBUG 'd'

#define set_memzero(buf, n) (void)memset(buf, 0, n)

#define err_exit(msg)                                                          \
  fprintf(stderr, "%s error, errno = %d, message = %s\n", msg, errno,          \
          strerror(errno));                                                    \
  exit(EXIT_FAILURE)

#define read_check(fd, buf, cnt)                                               \
  ret = read(fd, buf, cnt);                                                    \
  if (ret < 0) {                                                               \
    err_exit("read");                                                          \
  }

#define write_check(fd, buf, cnt)                                              \
  ret = write(fd, buf, cnt);                                                   \
  if (ret < 0) {                                                               \
    err_exit("write");                                                         \
  }

#define close_check(fd)                                                        \
  ret = close(fd);                                                             \
  if (ret < 0) {                                                               \
    err_exit("close");                                                         \
  }

#define dup2_check(oldfd, newfd)                                               \
  ret = dup2(oldfd, newfd);                                                    \
  if (ret < 0) {                                                               \
    err_exit("dup2");                                                          \
  }

#define pipe_check(fd)                                                         \
  if (pipe(fd) < 0) {                                                          \
    err_exit("pipe");                                                          \
  }

#define execvp_check(path)                                                     \
  {                                                                            \
    char *args[2] = {path, NULL};                                              \
    ret = execvp(path, args);                                                  \
  }                                                                            \
  if (ret < 0) {                                                               \
    err_exit("execvp");                                                        \
  }

#define get_stdin_termial_mode_paras(termial_paras)                            \
  ret = tcgetattr(STDIN_FILENO, &termial_paras);                               \
  if (ret < 0) {                                                               \
    err_exit("get terminal paras");                                            \
  }

#define change_stdin_termial_mode_paras(termial_paras)                         \
  ret = tcsetattr(STDIN_FILENO, TCSANOW, &termial_paras);                      \
  if (ret < 0) {                                                               \
    err_exit("change terminal paras");                                         \
  }

void restore_terminal_settings();
void SIGPIPE_handler(int sig);
void do_write(char *buf, int ofd, int nbytes);
void with_shell();
void without_shell();

int main(int argc, char **argv) {
  struct termios non_canonical_input_mode;

  struct option long_options[] = {{"shell", required_argument, 0, SHELL},
                                  {"debug", no_argument, 0, DEBUG},
                                  {0, 0, 0, 0}};

  while ((ret = getopt_long(argc, argv, "s:d", long_options, NULL)) != -1) {
    switch (ret) {

    case SHELL:
      shell_opt = 1;
      path = optarg;
      break;

    case DEBUG:
      debug_opt = 1;
      break;

    default:
      fprintf(stderr, "Usage: lab1a [--shell=program --shell] \n");
      exit(EXIT_FAILURE);
    }
  }

  get_stdin_termial_mode_paras(canonical_input_mode);

  atexit(restore_terminal_settings);

  get_stdin_termial_mode_paras(non_canonical_input_mode);

  non_canonical_input_mode.c_iflag = ISTRIP; // only lower 7 bits
  non_canonical_input_mode.c_oflag = 0;      // no processing
  non_canonical_input_mode.c_lflag = 0;      // no processing

  change_stdin_termial_mode_paras(non_canonical_input_mode);

  if (shell_opt == 1) {
    with_shell();
  } else {
    without_shell();
  }
}

void restore_terminal_settings() {
  int stat;
  pid_t wait_ret;

  change_stdin_termial_mode_paras(canonical_input_mode);

  if (shell_opt == 1) {

    wait_ret = waitpid(cpid, &stat, 0);
    if (wait_ret < 0) {
      err_exit("waitpid");
    }

    fprintf(stderr, "shell exit signal=%d status=%d\n", WTERMSIG(stat),
            WEXITSTATUS(stat));
  }
}

void SIGPIPE_handler(int sig) {

  if (sig == SIGPIPE) {
    exit(EXIT_SUCCESS);
  }
}

void do_write(char *buf, int ofd, int nbytes) {
  int i;
  char c;
  for (i = 0; i < nbytes; i++) {
    c = *(buf + i);
    switch (c) {

    case 0x04:
      if (shell_opt == 1) {
        close_check(to_child_pipe[1]);
      } else
        exit(EXIT_SUCCESS);
      break;

    case 0x03:
      if (shell_opt == 1) {
        kill(cpid, SIGINT);
      }
      break;

    case '\r':
    case '\n':
      if (ofd == STDOUT_FILENO) {

        write_check(ofd, "\r\n", 2);

      } else {

        write_check(ofd, "\n", 1);
      }
      break;

    default:
      write_check(ofd, &c, 1);
    }
  }
}

void with_shell() {

  struct pollfd pfd[2];
  int cnt = 0, nbytes = 0;
  char buffer[256];

  signal(SIGPIPE, SIGPIPE_handler);

  pipe_check(to_child_pipe);
  pipe_check(from_child_pipe);

  cpid = fork();

  if (cpid < 0) {
    err_exit("fork");
  }

  // child
  if (cpid == 0) {

    close_check(to_child_pipe[1]);
    close_check(from_child_pipe[0]);

    dup2_check(to_child_pipe[0], STDIN_FILENO);
    dup2_check(from_child_pipe[1], STDERR_FILENO);
    dup2_check(from_child_pipe[1], STDERR_FILENO);

    close_check(to_child_pipe[0]);
    close_check(from_child_pipe[1]);

    execvp_check(path);
  }

  // parent
  else {

    close_check(to_child_pipe[0]);
    close_check(from_child_pipe[1]);

    pfd[0].fd = STDIN_FILENO;
    pfd[0].events = POLLIN;

    pfd[1].fd = from_child_pipe[0];
    pfd[1].events = (POLLIN | POLLHUP | POLLERR);

    for (;;) {

      cnt = poll(pfd, 2, 0);
      if (cnt < 0) {
        err_exit("poll");
      }

      if (pfd[0].revents & POLLIN) {

        set_memzero(buffer, 256);

        read_check(STDIN_FILENO, buffer, 256);
        nbytes = ret;

        do_write(buffer, STDOUT_FILENO, nbytes);
        do_write(buffer, to_child_pipe[1], nbytes);
      }

      if (pfd[1].revents & POLLIN) {

        set_memzero(buffer, 256);

        read_check(pfd[1].fd, buffer, 256);
        nbytes = ret;

        do_write(buffer, STDOUT_FILENO, nbytes);
      }

      if (pfd[1].revents & (POLLHUP | POLLERR)) {

        close_check(to_child_pipe[1]);

        break;
      }
    }
  }
}

void without_shell() {
  int nbytes;
  char buffer[256];

  read_check(STDIN_FILENO, buffer, 256);
  nbytes = ret;
  while (nbytes > 0) {
    do_write(buffer, STDOUT_FILENO, nbytes);
    read_check(STDIN_FILENO, buffer, 1);
    nbytes = ret;
  }
}