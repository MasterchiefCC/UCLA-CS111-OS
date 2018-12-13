// NAME: CHEN CHEN
// EMAIL: chenchenstudent@gmail.com
// ID: 004710308
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "ext2_fs.h"

typedef int fd_t;

unsigned block_size;
struct ext2_super_block super_block_ctx;
fd_t fd;

#define SUPER_BLOCK_OFFSET 1024
#define BITMAP_BLOCK_BIT 8
#define DIR_VAL 0x4000
#define FILE_VAL 0x8000
#define SYMBOL_VAL 0xa000

#define EXT2_IND_BLOCK_LOG_OFFSET 12
#define EXT2_DIND_BLOCK_LOG_OFFSET (256 + EXT2_IND_BLOCK_LOG_OFFSET)
#define EXT2_TIND_BLOCK_LOG_OFFSET (65536 + EXT2_DIND_BLOCK_LOG_OFFSET)

#define EXT2_IND_BLOCK_LEVEL 1
#define EXT2_DIND_BLOCK_LEVEL (EXT2_IND_BLOCK_LEVEL + 1)
#define EXT2_TIND_BLOCK_LEVEL (EXT2_DIND_BLOCK_LEVEL + 1)

#define set_memzero(buf, n) (void)memset(buf, 0, n)

#define err_exit(msg)                                                          \
  fprintf(stderr, "%s error, errno = %d, message = %s\n", msg, errno,          \
          strerror(errno));                                                    \
  exit(EXIT_FAILURE)

#define BLOCK_CONF_OFFSET(block) ((block - 1) * block_size + SUPER_BLOCK_OFFSET)

#define open_check(str)                                                        \
  if ((fd = open(str, O_RDONLY)) == -1) {                                      \
    err_exit("open disk");                                                     \
  }

#define pread_check(struct_prt, struct_sz, offset)                             \
  if (pread(fd, struct_prt, struct_sz, offset) < 0) {                          \
    err_exit("pread");                                                         \
  }

#define malloc_check(prt, len)                                                 \
  prt = malloc(len);                                                           \
  if (prt == NULL) {                                                           \
    err_exit("malloc");                                                        \
  }

#define read_super_block()                                                     \
  pread_check(&super_block_ctx, sizeof(super_block_ctx), SUPER_BLOCK_OFFSET);  \
  fprintf(stdout, "SUPERBLOCK,%d,%d,%d,%d,%d,%d,%d\n",                         \
          super_block_ctx.s_blocks_count, super_block_ctx.s_inodes_count,      \
          block_size, super_block_ctx.s_inode_size,                            \
          super_block_ctx.s_blocks_per_group,                                  \
          super_block_ctx.s_inodes_per_group, super_block_ctx.s_first_ino);

#define read_time(raw_time, buffer)                                            \
  ptm = gmtime(raw_time);                                                      \
  strftime(buffer, 32, "%m/%d/%y %H:%M:%S", ptm)

static void read_group(unsigned cur_grp, unsigned tot_grp);

static void read_free_block(unsigned cur_grp, unsigned block_bitmap);

static void read_free_inode_bitmap(unsigned cur_grp, unsigned inode_bitmap,
                                   unsigned inode_table);

static void read_used_inode(unsigned inode_table, unsigned idx,
                            unsigned inode_idx);

static void read_indirect_entry(const uint32_t i_block_addr,
                                const unsigned inode_idx,
                                const unsigned logical_offset,
                                const char file_type, const int level);

static void read_directory_entry(unsigned inode_idx, unsigned block_idx);

int main(int argc, char **argv) {
  unsigned i, group_cnt;

  struct option long_opts[] = {{0, 0, 0, 0}};

  if (argc != 2) {
    err_exit("Usage: ./lab3a DISK");
  }

  if (getopt_long(argc, argv, "", long_opts, NULL) != -1) {
    err_exit("Usage: ./lab3a DISK");
  }

  open_check(argv[1]);

  block_size = EXT2_MIN_BLOCK_SIZE;

  read_super_block();

  group_cnt =
      super_block_ctx.s_blocks_count / super_block_ctx.s_blocks_per_group;
  if ((double)group_cnt < ((double)super_block_ctx.s_blocks_count /
                           super_block_ctx.s_blocks_per_group)) {
    ++group_cnt;
  }

  for (i = 0; i < group_cnt; ++i) {
    read_group(i, group_cnt);
  }

  exit(EXIT_SUCCESS);
}

static void read_group(unsigned cur_grp, unsigned tot_grp) {
  unsigned cur_grp_blocks, cur_grp_inodes, block_bitmap, inode_bitmap,
      inode_table;
  struct ext2_group_desc grp_ctx;

  unsigned long offset =
      block_size * (block_size == 1024 ? 2 : 1) + (32 * cur_grp);

  pread_check(&grp_ctx, sizeof(grp_ctx), offset);

  cur_grp_blocks = (cur_grp == tot_grp - 1)
                       ? (super_block_ctx.s_blocks_count -
                          (super_block_ctx.s_blocks_per_group * (tot_grp - 1)))
                       : (unsigned)super_block_ctx.s_blocks_per_group;

  cur_grp_inodes = (cur_grp == tot_grp - 1)
                       ? (super_block_ctx.s_inodes_count -
                          (super_block_ctx.s_inodes_per_group * (tot_grp - 1)))
                       : (unsigned)super_block_ctx.s_inodes_count;

  block_bitmap = grp_ctx.bg_block_bitmap;
  inode_bitmap = grp_ctx.bg_inode_bitmap;
  inode_table = grp_ctx.bg_inode_table;

  fprintf(stdout, "GROUP,%d,%d,%d,%d,%d,%d,%d,%d\n", cur_grp, cur_grp_blocks,
          cur_grp_inodes, grp_ctx.bg_free_blocks_count,
          grp_ctx.bg_free_inodes_count, block_bitmap, inode_bitmap,
          inode_table);

  read_free_block(cur_grp, block_bitmap);

  read_free_inode_bitmap(cur_grp, inode_bitmap, inode_table);
}

static void read_free_block(unsigned cur_grp, unsigned block_bitmap) {
  char *buffer, mask;
  unsigned i, j, cur_block;
  unsigned long offset;

  offset = BLOCK_CONF_OFFSET(block_bitmap);
  cur_block = (cur_grp * super_block_ctx.s_blocks_per_group) +
              super_block_ctx.s_first_data_block;

  malloc_check(buffer, block_size);

  pread_check(buffer, block_size, offset);

  for (i = 0; i < block_size; ++i) {
    mask = buffer[i];
    for (j = 0; j < BITMAP_BLOCK_BIT; ++j) {
      if ((1 & mask) == 0) {
        fprintf(stdout, "BFREE,%d\n", cur_block);
      }
      mask >>= 1;
      ++cur_block;
    }
  }
  free(buffer);
}

static void read_free_inode_bitmap(unsigned cur_grp, unsigned inode_bitmap,
                                   unsigned inode_table) {
  char *buffer, mask;
  unsigned long offset;
  unsigned i, j, cur, start, len;

  len = super_block_ctx.s_inodes_per_group / 8;

  malloc_check(buffer, len);

  offset = BLOCK_CONF_OFFSET(inode_bitmap);
  start = cur = super_block_ctx.s_inodes_per_group * cur_grp + 1;

  pread_check(buffer, len, offset);

  for (i = 0; i < len; ++i) {
    mask = buffer[i];
    for (j = 0; j < 8; j++) {
      //int used = 1 & mask;
      if (1 & mask) {
        read_used_inode(inode_table, cur - start, cur);
      } else {
        fprintf(stdout, "IFREE,%d\n", cur);
      }
      mask >>= 1;
      ++cur;
    }
  }

  free(buffer);
}

static void read_used_inode(unsigned inode_table, unsigned idx,
                            unsigned inode_idx) {

  unsigned i, len, tmp, blk_cnt;
  char file_type, atime_buf[32], ctime_buf[32], mtime_buf[32];
  unsigned long offset;

  struct tm *ptm;
  struct ext2_inode inode_ctx;

  len = sizeof(inode_ctx);
  offset = BLOCK_CONF_OFFSET(inode_table) + idx * len;
  pread_check(&inode_ctx, len, offset);

  if (inode_ctx.i_mode == 0 || inode_ctx.i_links_count == 0) {
    return;
  }

  tmp = (inode_ctx.i_mode >> 12) << 12;
  switch (tmp) {
  case DIR_VAL:
    file_type = 'd';
    break;

  case FILE_VAL:
    file_type = 'f';
    break;

  case SYMBOL_VAL:
    file_type = 's';
    break;

  default:
    return;
  }

  blk_cnt = 2 * (inode_ctx.i_blocks / (2 << super_block_ctx.s_log_block_size));
  time_t i_atime = inode_ctx.i_atime, i_ctime = inode_ctx.i_ctime,
         i_mtime = inode_ctx.i_mtime;
  read_time(&i_atime, atime_buf);
  read_time(&i_ctime, ctime_buf);
  read_time(&i_mtime, mtime_buf);

  fprintf(stdout, "INODE,%d,%c,%o,%d,%d,%d,%s,%s,%s,%d,%d", inode_idx,
          file_type, inode_ctx.i_mode & 0xFFF, inode_ctx.i_uid, inode_ctx.i_gid,
          inode_ctx.i_links_count, ctime_buf, mtime_buf, atime_buf,
          inode_ctx.i_size, blk_cnt);

  for (i = 0; i < 15; ++i) {
    fprintf(stdout, ",%d", inode_ctx.i_block[i]);
  }

  fprintf(stdout, "\n");

  for (i = 0; i < EXT2_NDIR_BLOCKS; i++) {
    if (inode_ctx.i_block[i] != 0 && file_type == 'd') {
      read_directory_entry(inode_idx, inode_ctx.i_block[i]);
    }
  }

  if (inode_ctx.i_block[EXT2_IND_BLOCK] != 0) {
    read_indirect_entry(inode_ctx.i_block[EXT2_IND_BLOCK], inode_idx,
                        EXT2_IND_BLOCK_LOG_OFFSET, file_type,
                        EXT2_IND_BLOCK_LEVEL);
  }

  if (inode_ctx.i_block[EXT2_DIND_BLOCK] != 0) {
    read_indirect_entry(inode_ctx.i_block[EXT2_DIND_BLOCK], inode_idx,
                        EXT2_DIND_BLOCK_LOG_OFFSET, file_type,
                        EXT2_DIND_BLOCK_LEVEL);
  }

  if (inode_ctx.i_block[EXT2_TIND_BLOCK] != 0) {
    read_indirect_entry(inode_ctx.i_block[EXT2_TIND_BLOCK], inode_idx,
                        EXT2_TIND_BLOCK_LOG_OFFSET, file_type,
                        EXT2_TIND_BLOCK_LEVEL);
  }
}

static void read_indirect_entry(const uint32_t i_block_addr,
                                const unsigned inode_idx,
                                const unsigned logical_offset,
                                const char file_type, const int level) {

  uint32_t buffer[block_size], len = block_size / sizeof(uint32_t), i = 0;
  unsigned long offset = BLOCK_CONF_OFFSET(i_block_addr);

  pread_check(buffer, block_size, offset);

  for (i = 0; i < len; ++i) {
    if (buffer[i] != 0) {

      if (level == 1 && file_type == 'd') {
        read_directory_entry(inode_idx, buffer[i]);
      }

      fprintf(stdout, "INDIRECT,%d,%d,%d,%d,%d\n", inode_idx, level,
              logical_offset + i, i_block_addr, buffer[i]);

      if (level == 2 || level == 3) {
        read_indirect_entry(buffer[i], inode_idx, logical_offset, file_type,
                            level - 1);
      }
    }
  }
}

static void read_directory_entry(unsigned inode_idx, unsigned block_idx) {
  unsigned i = 0;
  unsigned long offset = BLOCK_CONF_OFFSET(block_idx);
  
  struct ext2_dir_entry dir_ctx;

  for (i = 0; i < block_size;) {
    set_memzero(dir_ctx.name, EXT2_NAME_LEN);

    pread_check(&dir_ctx, sizeof(dir_ctx), offset + i);

    if (dir_ctx.inode != 0) {
      set_memzero(&dir_ctx.name[dir_ctx.name_len],
                  EXT2_NAME_LEN - dir_ctx.name_len);

      fprintf(stdout, "DIRENT,%d,%d,%d,%d,%d,'%s'\n", inode_idx, i,
              dir_ctx.inode, dir_ctx.rec_len, dir_ctx.name_len, dir_ctx.name);
    }

    i += dir_ctx.rec_len;
  }
}
