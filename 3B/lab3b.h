#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <limits.h>
#include <map>
#include <set>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <time.h>
#include <unistd.h>
#include <vector>

using namespace std;

typedef int fd_t;

#define err_exit(msg)                                                          \
  fprintf(stderr, "%s error, errno = %d, message = %s\n", msg, errno,          \
          strerror(errno));                                                    \
  exit(EXIT_FAILURE)

#define open_RDONLY_file_check(fd, path)                                       \
  fd = open(path, O_RDONLY);                                                   \
  if (fd < 0) {                                                                \
    err_exit("open read only file");                                           \
  }

class CSV_file {
public:
  CSV_file(string file_name);

  bool inline is_damged();

private:
  CSV_file();

  ///////////////////////////////////////////////////////////

  unsigned int inline getlines();

  void inline read_ext2_summ();

  ///////////////////////////////////////////////////////////
  vector<vector<string>> csv_file;

  unsigned int rows;
  unsigned int block_cnt;
  unsigned int inodes_cnt;

  bool damged;
};