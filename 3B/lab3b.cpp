// NAME: CHEN CHEN
// EMAIL: chenchenstudent@gmail.com
// ID: 004710308
#include "lab3b.h"

int main(int argc, char **argv) {

  struct option long_opts[] = {{0, 0, 0, 0}};

  if (argc != 2) {
    err_exit("Usage: ./lab3a CSV");
  }

  if (getopt_long(argc, argv, "", long_opts, NULL) != -1) {
    err_exit("Usage: ./lab3a CSV");
  }

  fd_t fd;

  open_RDONLY_file_check(fd, argv[1]);

  string input_csv(argv[1]);

  CSV_file file_ctx(input_csv);

  if (file_ctx.is_damged())
    return 2;
  else
    return 0;
}

CSV_file::CSV_file(string file_name) : damged(false) {

  ifstream cvs_ctx(file_name, ios::in);
  string line_ctx;

  while (getline(cvs_ctx, line_ctx)) {
    stringstream ss(line_ctx);
    string str;
    vector<string> tmp;

    while (getline(ss, str, ',')) {
      tmp.push_back(str);
    }

    csv_file.push_back(tmp);
  }

  rows = csv_file.size();

  read_ext2_summ();
}

bool inline CSV_file::is_damged() { return damged; }

unsigned int inline CSV_file::getlines() { return rows; }

void inline CSV_file::read_ext2_summ() {

  const set<int> reserved_blocks = {0, 1, 2, 3, 4, 5, 6, 7, 64};
  const set<int> reserved_inode = {1, 3, 4, 5, 6, 7, 8, 9, 10};

  set<int> bfree, ifree;

  // inode_idx -> links_count
  map<int, int> m_inode_to_link_cnt;
  map<int, int> m_inode_to_parent;

  // block idx -> {inode_idx, offset, level};
  map<int, vector<vector<int>>> m_block_idx_to_info;

  map<int, string> m_inode_idx_to_dirname;

  map<int, int> m_inode_to_ref_link_cnt;

  map<int, int> m_parent_to_inode;

  for (unsigned int x = 0; x < csv_file.size(); ++x) {
    vector<string> &line_ctx = csv_file[x];

    const string &category = line_ctx[0];

    int inode_idx;

    // super block info
    if (category == "SUPERBLOCK") {

      block_cnt = stoi(line_ctx[1]);
      inodes_cnt = stoi(line_ctx[2]);
    }
    // free block set
    else if (category == "BFREE") {

      bfree.insert(stoi(line_ctx[1]));
    }
    // free inodes set
    else if (category == "IFREE") {

      ifree.insert(stoi(line_ctx[1]));
    }
    // inode process from data to mem str
    else if (category == "INODE") {

      inode_idx = stoi(line_ctx[1]);
      m_inode_to_link_cnt[inode_idx] = stoi(line_ctx[6]);

      for (int i = 12; i < 27; ++i) {

        int block_idx = stoi(line_ctx[i]);

        // unused block
        if (block_idx == 0) {
          continue;
        }

        string indir_type;
        int level, logical_offset;

        if (i == 24) {

          indir_type = "INDIRECT ";

          level = 1;
          logical_offset = 12;
        } else if (i == 25) {

          indir_type = "DOUBLE INDIRECT ";

          level = 2;
          logical_offset = 268;
        } else if (i == 26) {

          indir_type = "TRIPLE INDIRECT ";

          level = 3;
          logical_offset = 65804;
        } else {
          indir_type = "";

          level = 0;
          logical_offset = 0;
        }

        if (block_idx < 0 || (unsigned int)block_idx > block_cnt) {
          printf("INVALID %sBLOCK %d IN INODE %d AT OFFSET %d\n",
                 indir_type.c_str(), block_idx, inode_idx, logical_offset);

          damged = true;
        }
        ////reserved block
        else if (reserved_blocks.find(block_idx) != reserved_blocks.end() &&
                 block_idx != 0) {
          printf("RESERVED %sBLOCK %d IN INODE %d AT OFFSET %d\n",
                 indir_type.c_str(), block_idx, inode_idx, logical_offset);

          damged = true;
        }
        //
        else {
          vector<int> tmp;

          tmp.push_back(inode_idx);
          tmp.push_back(logical_offset);
          tmp.push_back(level);

          m_block_idx_to_info[block_idx].push_back(tmp);
        }
      }
    } //
    else if (category == "INDIRECT") {

      string indir_type;
      int offset = 0;

      int inode_idx = stoi(line_ctx[1]), block_idx = stoi(line_ctx[5]);

      int level = stoi(line_ctx[2]);

      if (level == 1) {

        indir_type = "INDIRECT ";
        offset = 12;
      } //
      else if (level == 2) {

        indir_type = "DOUBLE INDIRECT ";
        offset = 268;
      } //
      else if (level == 3) {
        indir_type = "TRIPLE INDIRECT ";
        offset = 65804;
      }

      if (block_idx < 0 || (unsigned int)block_idx > block_cnt) {
        printf("INVALID %sBLOCK %d IN INODE %d AT OFFSET %d\n",
               indir_type.c_str(), block_idx, inode_idx, offset);

        damged = true;
      }
      ////reserved block
      else if (reserved_blocks.find(block_idx) != reserved_blocks.end() &&
               block_idx != 0) {
        printf("RESERVED %sBLOCK %d IN INODE %d AT OFFSET %d\n",
               indir_type.c_str(), block_idx, inode_idx, offset);

        damged = true;
      }
      //
      else {
        vector<int> tmp;

        tmp.push_back(inode_idx);
        tmp.push_back(offset);
        tmp.push_back(level);

        m_block_idx_to_info[block_idx].push_back(tmp);
      }

    }
    // dirent
    else if (category == "DIRENT") {

      string dir_name = line_ctx[6];
      int parent_inode = stoi(line_ctx[1]), inode_idx = stoi(line_ctx[3]);

      m_inode_idx_to_dirname[inode_idx] = dir_name;

      m_inode_to_ref_link_cnt[inode_idx] += 1;

      if (inode_idx < 1 || (unsigned int)inode_idx > inodes_cnt) {

        printf("DIRECTORY INODE %d NAME '%s' INVALID INODE %d\n", parent_inode,
               dir_name.substr(1, dir_name.size() - 2).c_str(), inode_idx);

        damged = true;
      }
      //
      else if (dir_name.substr(0, 3) == "'.'" && parent_inode != inode_idx) {

        printf("DIRECTORY INODE %d NAME '.' LINK TO INODE %d SHOULD BE %d\n",
               parent_inode, inode_idx, parent_inode);

        damged = true;
      }
      //
      else if (dir_name.substr(0, 3) == "'.'") {
        continue;
      }
      //
      else if (dir_name.substr(0, 4) == "'..'") {
        m_parent_to_inode[parent_inode] = inode_idx;
      }
      //
      else {
        m_inode_to_parent[inode_idx] = parent_inode;
      }
    }
  }

  for (unsigned int i = 1; i < block_cnt + 1; ++i) {
    if (bfree.find(i) == bfree.end() &&
        m_block_idx_to_info.find(i) == m_block_idx_to_info.end() &&
        reserved_blocks.find(i) == reserved_blocks.end()) {

      printf("UNREFERENCED BLOCK %d\n", i);

      damged = true;
    }
    //
    else if (bfree.find(i) != bfree.end() &&
             m_block_idx_to_info.find(i) != m_block_idx_to_info.end()) {

      printf("ALLOCATED BLOCK %d ON FREELIST\n", i);

      damged = true;
    }
  }

  for (unsigned int i = 1; i < inodes_cnt + 1; ++i) {
    if (ifree.find(i) == ifree.end() &&
        m_inode_to_link_cnt.find(i) == m_inode_to_link_cnt.end() &&
        m_inode_to_parent.find(i) == m_inode_to_parent.end() &&
        reserved_inode.find(i) == reserved_inode.end()) {

      printf("UNALLOCATED INODE %d NOT ON FREELIST\n", i);

      damged = true;
    } //
    else if (m_inode_to_link_cnt.find(i) != m_inode_to_link_cnt.end() &&
             ifree.find(i) != ifree.end()) {

      printf("ALLOCATED INODE %d ON FREELIST\n", i);

      damged = true;
    }
  }

  for (auto it = m_block_idx_to_info.begin(); it != m_block_idx_to_info.end();
       ++it) {
         
    int block_idx = it->first;
    vector<vector<int>> buffer = it->second;

    if (buffer.size() > 1) {

      string indir_type;

      for (unsigned int i = 0; i < buffer.size(); ++i) {

        vector<int> &tmp = buffer[i];
        int level = tmp[2];

        switch (level) {
        case 1:
          indir_type = "INDIRECT ";
          break;

        case 2:
          indir_type = "DOUBLE INDIRECT ";
          break;

        case 3:
          indir_type = "TRIPLE INDIRECT ";
          break;

        default:
          break;
        }

        printf("DUPLICATE %sBLOCK %d IN INODE %d AT OFFSET %d\n",
               indir_type.c_str(), block_idx, tmp[0], tmp[1]);
      }

      damged = true;
    }
  }

  for (auto it = m_parent_to_inode.begin(); it != m_parent_to_inode.end();
       ++it) {

    if (it->first == 2 && it->second == 2) {
      continue;
    }

    if (it->first == 2) {
      printf("DIRECTORY INODE 2 NAME '..' LINK TO INODE %d SHOULD BE 2\n",
             it->second);

      damged = true;
    } //
    else if (m_inode_to_parent.find(it->first) == m_inode_to_parent.end()) {
      printf("DIRECTORY INODE %d NAME '..' LINK TO INODE %d SHOULD BE %d\n",
             it->second, it->first, it->second);

      damged = true;
    } //
    else if (it->second != m_inode_to_parent[it->first]) {
      printf("DIRECTORY INODE %d NAME '..' LINK TO INODE %d SHOULD BE %d\n",
             it->first, it->first, m_inode_to_parent[it->first]);

      damged = true;
    }
  }

  for (auto it = m_inode_to_link_cnt.begin(); it != m_inode_to_link_cnt.end();
       ++it) {
    int cur_cnt = it->second, dict_cnt = 0;

    auto tmp_it = m_inode_to_ref_link_cnt.find(it->first);

    if (tmp_it != m_inode_to_parent.end()) {
      dict_cnt = tmp_it->second;
    }

    if (cur_cnt != dict_cnt) {
      printf("INODE %d HAS %d LINKS BUT LINKCOUNT IS %d\n", it->first, dict_cnt,
             cur_cnt);

      damged = true;
    }
  }

  for (auto it = m_inode_idx_to_dirname.begin();
       it != m_inode_idx_to_dirname.end(); ++it) {
    auto if_it = ifree.find(it->first);
    auto in_it = m_inode_to_parent.find(it->first);
    if (if_it != ifree.end() && in_it != m_inode_to_parent.end()) {

      string &tmp = it->second;

      printf("DIRECTORY INODE %d NAME '%s' UNALLOCATED INODE %d\n",
             in_it->second, tmp.substr(1, tmp.size() - 2).c_str(), it->first);

      damged = true;
    }
  }
}