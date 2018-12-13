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

//////global/////
struct termios canonical_input_mode;

flag_t DEBUG_FLAG = 0;

#define KEY_LEN 16
#define TO_SERVER 1551
#define FROM_SERVER 6655
#define PORT_OPT 'p'
#define LOG_OPT 'l'
#define ENCRYPT_OPT 'e'
#define DEBUG_OPT 'd'
#define POLL_MODE (POLLIN | POLLHUP | POLLERR)

#define set_memzero(buf, n) (void)memset((void *)buf, 0, (size_t)n)

#define set_memcpy(dst, src, n)                                                \
  (void)memcpy((void *)dst, (void *)src, (size_t)n)

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

#define encryption_check(td, buffer, size)                                     \
  if (mcrypt_generic(td, &buffer, size) != 0) {                                \
    err_exit("encryption");                                                    \
  }

#define decryption_check(td, buffer, size)                                     \
  if (mdecrypt_generic(td, &buffer, size) != 0) {                              \
    err_exit("decryption");                                                    \
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

static void send_check(fd_t fd, char *buffer, int cnt);
static void encryption_to_server(MCRYPT encryp_desc, char *buffer,
                                 int bytes_read);
static MCRYPT set_encryption(char *key_path);
static void log_debug(fd_t log_file_fd, char *buffer, int nbytes, int direct);
void restore_terminal_settings();

int main(int argc, char **argv) {
  int ret = 0, n = 0, nbytes = 0;
  char *log_path = NULL, *key_path = NULL, buffer[256];

  in_port_t port = 0;
  fd_t log_file_fd, sock_fd;
  flag_t LOG_FLAG = 0, ENCRYPT_FLAG = 0, PORT_FLAG = 0;

  MCRYPT encryp_desc;

  struct termios non_canonical_input_mode;

  struct sockaddr_in sin;
  struct hostent *h;
  struct pollfd pfd[2];

  static char name[] = "localhost";

  struct option opts[] = {{"debug", no_argument, 0, DEBUG_OPT},
                          {"log", optional_argument, 0, LOG_OPT},
                          {"port", required_argument, 0, PORT_OPT},
                          {"encrypt", required_argument, 0, ENCRYPT_OPT},
                          {0, 0, 0, 0}};

  while ((ret = getopt_long(argc, argv, "dl:p:e:", opts, NULL)) != -1) {
    switch (ret) {
    case DEBUG_OPT:
      DEBUG_FLAG = 1;
      break;

    case LOG_OPT:
      LOG_FLAG = 1;
      log_path = optarg;

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
      err_exit("Usage: lab1b-client --port=1551 [--log=logfile] "
               "[--encrypt=keyfile]");

      break;
    }
  }

  if (PORT_FLAG == 0) {
    err_exit("Usage: lab1b-client --port=1551 [--log=logfile] "
             "[--encrypt=keyfile]");
  }

  if (LOG_FLAG == 1) {
    file_create_check(log_file_fd, log_path);
  }

  if (ENCRYPT_FLAG == 1) {
    encryp_desc = set_encryption(key_path);
  }

  get_stdin_termial_mode_paras(canonical_input_mode);

  atexit(restore_terminal_settings);

  get_stdin_termial_mode_paras(non_canonical_input_mode);

  non_canonical_input_mode.c_iflag = ISTRIP; // only lower 7 bits
  non_canonical_input_mode.c_oflag = 0;      // no processing
  non_canonical_input_mode.c_lflag = 0;      // no processing

  change_stdin_termial_mode_paras(non_canonical_input_mode);

  h = gethostbyname(name);

  socket_check(sock_fd);

  set_memzero(&sin, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);

  set_memcpy(&sin.sin_addr.s_addr, h->h_addr, h->h_length);

  if (connect(sock_fd, (const struct sockaddr *)&sin, sizeof(sin)) < 0) {
    err_exit("connect");
  }

  pfd[0].fd = STDIN_FILENO;
  pfd[0].events = POLL_MODE;

  pfd[1].fd = sock_fd;
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

      send_check(STDOUT_FILENO, buffer, nbytes);

      if (ENCRYPT_FLAG == 1) {
        encryption_to_server(encryp_desc, buffer, nbytes);
      }

      if (LOG_FLAG == 1) {
        log_debug(log_file_fd, buffer, nbytes, TO_SERVER);
      }

      send_check(pfd[1].fd, buffer, nbytes);
    }

    if (pfd[1].revents & POLLIN) {
      set_memzero(buffer, 256);

      read_check(pfd[1].fd, buffer, 256);

      if (nbytes == 0) {
        //^C || ^D
        close_check(pfd[1].fd);
        exit(EXIT_SUCCESS);
      }

      if (LOG_FLAG == 1) {
        log_debug(log_file_fd, buffer, nbytes, FROM_SERVER);
      }

      if (ENCRYPT_FLAG == 1) {
        decryption_check(encryp_desc, buffer, nbytes);
      }

      send_check(STDOUT_FILENO, buffer, nbytes);
    }

    if (pfd[1].revents & (POLLHUP | POLLERR)) {

      close_check(pfd[1].fd);

      exit(EXIT_SUCCESS);
    }
  }
}

//---------------------------function implement-----------------------//
static void send_check(fd_t fd, char *buffer, int cnt) {
  int i = 0, nbytes;
  char c;
  for (; i < cnt; ++i) {
    c = *(buffer + i);
    switch (c) {
    case '\r':
    case '\n':

      if (fd == STDOUT_FILENO) {
        write_check(fd, "\r\n", 2);
      } else {
        write_check(fd, "\n", 1);
      }

      break;

    default:
      write_check(fd, &c, 1);

      break;
    }
  }
}

static void log_debug(fd_t log_file_fd, char *buffer, int nbytes, int direct) {

  switch (direct) {
  case TO_SERVER:
    if (dprintf(log_file_fd, "SENT %d bytes: ", nbytes) < 0) {
      err_exit("write log");
    }

    break;

  case FROM_SERVER:
    if (dprintf(log_file_fd, "RECEIVED %d bytes: ", nbytes) < 0) {
      err_exit("write log");
    }

    break;

  default:
    err_exit("unknown direct");
  }

  send_check(log_file_fd, buffer, nbytes);
  write_check(log_file_fd, "\n", 1);
}

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

void restore_terminal_settings() {

  change_stdin_termial_mode_paras(canonical_input_mode);
}

static void encryption_to_server(MCRYPT encryp_desc, char *buffer,
                                 int bytes_read) {
  int i;

  for (i = 0; i < bytes_read; ++i) {

    if (buffer[i] == '\r' || buffer[i] == '\n') {
      continue;
    }

    encryption_check(encryp_desc, buffer[i], 1);
  }
}
