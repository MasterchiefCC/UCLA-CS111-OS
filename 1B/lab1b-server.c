#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <mcrypt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

typedef int fd_t;
typedef intptr_t flag_t;
typedef uint16_t in_port_t;
typedef __socklen_t socklen_t;

#define KEY_LEN 16
#define TO_SEHLL 1551
#define TO_CLIENT 6655
#define LISTEN_BACKLOG 50
#define PORT_OPT 'p'
#define ENCRYPT_OPT 'e'
#define DEBUG_OPT 'd'
#define POLL_MODE (POLLIN | POLLHUP | POLLERR)

#define set_memzero(buf, n) (void)memset((void *)buf, 0, (size_t)n)

#define set_memcpy(dst, src, n)                                                \
  (void)memcpy((void *)dst, (void *)src, (size_t)n)

#define set_signal(sig_type)                                                   \
  if (signal(sig_type, &signal_handler) == SIG_ERR) {                          \
    err_exit("set signal");                                                    \
  }

#define err_exit(msg)                                                          \
  fprintf(stderr, "%s error, errno = %d, message = %s\n", msg, errno,          \
          strerror(errno));                                                    \
  exit(EXIT_FAILURE)

//////////////check
#define read_check(fd, buf, n)                                                 \
  nbytes = read(fd, buf, n);                                                   \
  if (nbytes < 0) {                                                            \
    err_exit("read");                                                          \
  }

#define write_check(fd, buf, n)                                                \
  nbytes = write(fd, buf, n);                                                  \
  if (nbytes < 0) {                                                            \
    err_exit("write");                                                         \
  }

#define close_check(fd)                                                        \
  if (close(fd) < 0) {                                                         \
    err_exit("close");                                                         \
  }

#define dup2_check(oldfd, newfd)                                               \
  if (dup2(oldfd, newfd) < 0) {                                                \
    err_exit("dup2");                                                          \
  }

#define pipe_check(fd)                                                         \
  if (pipe(fd) < 0) {                                                          \
    err_exit("pipe");                                                          \
  }

#define listen_check(fd)                                                       \
  if (listen(fd, LISTEN_BACKLOG) < 0) {                                        \
    err_exit("listen");                                                        \
  }

#define encryption_check(td, buffer, size)                                     \
  if (mcrypt_generic(td, &buffer, size) != 0) {                                \
    err_exit("encryption");                                                    \
  }

#define decryption_check(td, buffer, size)                                     \
  if (mdecrypt_generic(td, &buffer, size) != 0) {                              \
    err_exit("decryption");                                                    \
  }

#define kill_check(pid, sig)                                                   \
  if (kill(pid, sig) < 0) {                                                    \
    err_exit("kill");                                                          \
  }

#define execvp_check(path)                                                     \
  {                                                                            \
    char *args[2] = {path, NULL};                                              \
    ret = execvp(path, args);                                                  \
  }                                                                            \
  if (ret < 0) {                                                               \
    err_exit("execvp");                                                        \
  }

#define malloc_check(prt, len)                                                 \
  prt = malloc(len);                                                           \
  if (prt == NULL) {                                                           \
    err_exit("malloc");                                                        \
  }

////////////////fd
#define file_create_check(fd, path)                                            \
  fd = creat(path, S_IRWXU);                                                   \
  if (fd < 0) {                                                                \
    err_exit("create file");                                                   \
  }

#define open_RDONLY_file_check(fd, path)                                       \
  fd = open(path, O_RDONLY);                                                   \
  if (fd < 0) {                                                                \
    err_exit("open read only file");                                           \
  }

#define socket_check(sock_fd)                                                  \
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);                                   \
  if (sock_fd < 0) {                                                           \
    err_exit("open socket");                                                   \
  }

#define waitpid_check(pid, stat)                                               \
  if (waitpid(pid, &stat, 0) < 0) {                                            \
    err_exit("waitpid");                                                       \
  }

//////////////other
#define open_encryp_module_check(encryp_desc, algorithm)                       \
  encryp_desc = mcrypt_module_open(algorithm, NULL, "cfb", NULL);              \
  if (encryp_desc == MCRYPT_FAILED) {                                          \
    err_exit("open encryp module");                                            \
  }

#define get_stdin_termial_mode_paras(termial_paras)                            \
  if (tcgetattr(STDIN_FILENO, &termial_paras) < 0) {                           \
    err_exit("get terminal paras");                                            \
  }

#define change_stdin_termial_mode_paras(termial_paras)                         \
  if (tcsetattr(STDIN_FILENO, TCSANOW, &termial_paras) < 0) {                  \
    err_exit("change terminal paras");                                         \
  }

static void send_check(fd_t fd, char *buffer, int cnt, int direct);
static void decryption_from_client(MCRYPT encryp_desc, char *buffer,
                                   int bytes_read);
static MCRYPT set_encryption(char *key_path);
void wait_child_process_end();
void signal_handler(int signal);

flag_t DEBUG_FLAG = 0;
pid_t cpid = -1;
int shell_read_parent_wrt[2];
int shell_wrt_parent_read[2];

int main(int argc, char **argv) {
  int n, ret = 0, nbytes = 0;
  char *key_path = NULL, buffer[256];

  socklen_t sock_len = 0;
  flag_t PORT_FLAG = 0, ENCRYPT_FLAG = 0;
  in_port_t port = 0;
  fd_t sock_fd, client_fd;

  MCRYPT encryp_desc;

  struct sockaddr_in server_sin, client_sin;
  struct pollfd pfd[2];

  static char path[] = "/bin/bash";

  struct option opts[] = {{"debug", no_argument, 0, DEBUG_OPT},
                          {"port", required_argument, 0, PORT_OPT},
                          {"encrypt", required_argument, 0, ENCRYPT_OPT},

                          {0, 0, 0, 0}};

  while ((ret = getopt_long(argc, argv, "dl:p:e:", opts, NULL)) != -1) {
    switch (ret) {
    case DEBUG_OPT:

      DEBUG_FLAG = 1;
      break;

    case PORT_OPT:

      PORT_FLAG = 1;
      port = atoi(optarg);

      break;

    case ENCRYPT_OPT:
      ENCRYPT_FLAG = 1;
      key_path = optarg;

      break;

    default:
      err_exit(
          "Usage: lab1b-client --port=1551 [--encrypt=path_to_key] [--debug]");

      break;
    }
  }

  if (PORT_FLAG == 0) {
    err_exit(
        "Usage: lab1b-client --port=1551 [--encrypt=path_to_key] [--debug]");
  }

  if (ENCRYPT_FLAG == 1) {
    encryp_desc = set_encryption(key_path);
  }

  set_signal(SIGINT);
  set_signal(SIGPIPE);
  set_signal(SIGTERM);

  socket_check(sock_fd);

  set_memzero(&server_sin, sizeof(server_sin));

  server_sin.sin_family = AF_INET;
  server_sin.sin_port = htons(port);
  server_sin.sin_addr.s_addr = INADDR_ANY;

  if (bind(sock_fd, (const struct sockaddr *)&server_sin, sizeof(server_sin)) <
      0) {
    err_exit("bind");
  }

  listen_check(sock_fd);

  sock_len = sizeof(client_sin);

  client_fd = accept(sock_fd, (struct sockaddr *)&client_sin, &sock_len);
  if (client_fd < 0) {
    err_exit("accept");
  }

  pipe_check(shell_read_parent_wrt);
  pipe_check(shell_wrt_parent_read);

  cpid = fork();
  if (cpid < 0) {
    err_exit("fork");
  }

  // child
  if (cpid == 0) {

    close_check(shell_read_parent_wrt[1]);
    close_check(shell_wrt_parent_read[0]);

    dup2_check(shell_read_parent_wrt[0], STDIN_FILENO);
    dup2_check(shell_wrt_parent_read[1], STDOUT_FILENO);
    dup2_check(shell_wrt_parent_read[1], STDERR_FILENO);

    close_check(shell_read_parent_wrt[0]);
    close_check(shell_wrt_parent_read[1]);

    execvp_check(path);

  }
  // parent
  else {

    close_check(shell_read_parent_wrt[0]);
    close_check(shell_wrt_parent_read[1]);

    pfd[0].fd = client_fd;
    pfd[0].events = POLL_MODE;

    pfd[1].fd = shell_wrt_parent_read[0];
    pfd[1].events = POLL_MODE;

    for (;;) {

      n = poll(pfd, 2, 0);

      if (n < 0) {
        err_exit("poll");
      }

      if (n == 0) {
        continue;
      }

      if (pfd[0].revents & POLLIN) {

        set_memzero(buffer, 256);

        read_check(pfd[0].fd, buffer, 256);

        if (nbytes <= 0) {
          kill_check(cpid, SIGTERM);
          wait_child_process_end(); // TODO
          break;
        }

        if (ENCRYPT_FLAG == 1) {
          decryption_from_client(encryp_desc, buffer, nbytes);
        }

        send_check(shell_read_parent_wrt[1], buffer, nbytes, TO_SEHLL);
      }

      if (pfd[1].revents & POLLIN) {

        set_memzero(buffer, 256);

        read_check(pfd[1].fd, buffer, 256);

        if (nbytes <= 0) {
          wait_child_process_end();
          break;
        }

        if (ENCRYPT_FLAG == 1) {
          encryption_check(encryp_desc, buffer, nbytes);
        }

        send_check(pfd[0].fd, buffer, nbytes, 0);
      }

      if (pfd[1].revents & (POLLHUP | POLLERR)) {
        close_check(pfd[1].fd);
        wait_child_process_end();
        break;
      }
    }
  }
  exit(0);
}

////////////////////////

////////////////////////

void signal_handler(int signal) {
  if (signal == SIGPIPE && DEBUG_FLAG == 1) {
    wait_child_process_end();
  }
}

static void send_check(fd_t fd, char *buffer, int cnt, int direct) {
  int i = 0, nbytes;
  char c;
  for (; i < cnt; ++i) {
    c = *(buffer + i);

    switch (c) {

    case 0x03:
      if (direct == TO_SEHLL) {
        kill_check(cpid, SIGINT);
      }

      break;

    case 0x04:
      if (direct == TO_SEHLL) {
        close_check(shell_read_parent_wrt[1]);
      }

      break;

    default:
      write_check(fd, &c, 1);

      break;
    }
  }
}


//TODO possible mem_leak
static MCRYPT set_encryption(char *key_path) {
  MCRYPT encryp_desc;
  char *buffer = NULL, *iv_buffer = NULL;
  int iv_size, nbytes;
  fd_t key_fd;

  open_encryp_module_check(encryp_desc, "twofish");

  open_RDONLY_file_check(key_fd, key_path);

  malloc_check(buffer, KEY_LEN);

  iv_size = mcrypt_enc_get_iv_size(encryp_desc);

  malloc_check(iv_buffer, iv_size);

  set_memzero(buffer, KEY_LEN);

  read_check(key_fd, buffer, KEY_LEN);

  close_check(key_fd);

  set_memzero(iv_buffer, iv_size);

  if (mcrypt_generic_init(encryp_desc, buffer, KEY_LEN, iv_buffer) < 0) {
    goto error;
  }

  free(buffer);
  free(iv_buffer);

  return encryp_desc;

error:
  free(buffer);
  free(iv_buffer);
  err_exit("couldn't set up encryption");
}

void wait_child_process_end() {
  int stat;

  waitpid_check(cpid, stat);

  fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(stat),
          WEXITSTATUS(stat));
  exit(EXIT_SUCCESS);
}

static void decryption_from_client(MCRYPT encryp_desc, char *buffer,
                                   int bytes_read) {

  int i;
  for (i = 0; i < bytes_read; ++i) {

    if (buffer[i] == '\r' || buffer[i] == '\n') {
      continue;
    }

    decryption_check(encryp_desc, buffer[i], 1);
  }
}
