#include "../superblock/superblock.hpp"
#include "file-system-manager.hpp"
#define BLOCK_SIZE 1024

using namespace std;

FileSystemManager::FileSystemManager(FILE *image){
  this->image = image;
  this->superblock = read_ext2_superblock(this->image);
}

void FileSystemManager::info(){
  uint32_t group_count = this->superblock->s_blocks_per_group/BLOCK_SIZE;
  uint32_t inode_table_blocks = this->superblock->s_inodes_per_group/group_count;

  cout << "Volume name.....: " << this->superblock->s_volume_name << endl;
  cout << "Image size......: " << this->superblock->s_blocks_count << " KiB" << endl;
  cout << "Free space......: " << this->superblock->s_free_blocks_count << " KiB" << endl;
  cout << "Free inodes.....: " << this->superblock->s_free_inodes_count << endl;
  cout << "Free blocks.....: " << this->superblock->s_free_blocks_count << endl;
  cout << "Blocks size.....: " << BLOCK_SIZE << " bytes" << endl;
  cout << "Inode size......: " << this->superblock->s_inode_size << " bytes" << endl;
  cout << "Group count.....: " << group_count << endl;
  cout << "Group size......: " << this->superblock->s_blocks_per_group << " blocks" << endl;
  cout << "Group inodes....: " << this->superblock->s_inodes_per_group << " inodes" << endl;
  cout << "Inodetable size.: " << inode_table_blocks << " blocks" << endl;
}

void FileSystemManager::superblock_info(){
  print_superblock(this->superblock);
}