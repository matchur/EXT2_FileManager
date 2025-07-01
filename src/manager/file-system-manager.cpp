#include "../superblock/superblock.hpp"
#include "file-system-manager.hpp"
#include "../blocks-group-descriptor/blocks-group-descriptor.hpp"
#include "../inode/inode.hpp"
#include "../error/error.hpp"
#include <cstring>
#define BLOCK_SIZE 1024

using namespace std;

FileSystemManager::FileSystemManager(FILE *image){
  this->image = image;
  this->superblock = read_ext2_superblock(this->image);
  this->bgd = read_blocks_group_descriptor(this->image, block_group_descriptor_address(0));
  Inode* first_inode = read_inode(this->image, this->bgd, 2);
  vector<Directory> directories = read_directories(this->image, first_inode);
  this->navigation.push_back(directories.at(0));
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

void FileSystemManager::blocks_group_descriptor_info(int index) {
  uint32_t address = block_group_descriptor_address(index);
  BlocksGroupDescriptor *bgd = read_blocks_group_descriptor(this->image, address);
  print_blocks_group_descriptor(bgd);
}

void FileSystemManager::inode_info(unsigned int inode) {
  unsigned int inode_bgd = block_group_from_inode(this->superblock, inode);
  BlocksGroupDescriptor *bgd = read_blocks_group_descriptor(this->image, block_group_descriptor_address(inode_bgd));
  Inode *found_inode = read_inode(this->image, bgd, inode_order(this->superblock, inode));
  print_inode(found_inode);
}

void FileSystemManager::cat(const char *directory_name) {

  Directory actual_directory = this->navigation.at(this->navigation.size() - 1);
  Inode *actual_inode = read_inode(this->image, this->bgd, inode_order(this->superblock, actual_directory.inode));

  Directory *directory = search_directory(this->image, actual_inode, directory_name);

  if(!directory) throw new Error("file not found.");
  if(directory->file_type != 1) throw new Error("it's not a file.");

  unsigned int directory_inode_block_group = block_group_from_inode(this->superblock, directory->inode);
  BlocksGroupDescriptor *bgd_of_inode = read_blocks_group_descriptor(this->image, block_group_descriptor_address(directory_inode_block_group));

  Inode *directory_inode = read_inode(this->image, bgd_of_inode, inode_order(this->superblock, directory->inode));
  print_inode_blocks_content(this->image, directory_inode);
}

void FileSystemManager::ls() {
  Directory actual_directory = this->navigation.at(this->navigation.size() - 1);
  Inode *actual_inode = read_inode(this->image, this->bgd, inode_order(this->superblock, actual_directory.inode));
  vector<Directory> directories = read_directories(this->image, actual_inode);
  print_directories(directories);
}

string FileSystemManager::pwd() {
  string str;
  for (vector<Directory>::iterator i = this->navigation.begin(); i != this->navigation.end(); i++)
  {
    if (std::strcmp((*i).name, "."))
      str = str.append((*i).name);
    str = str.append("/");
  }

  return str;
}

void FileSystemManager::cd(const char *directory_name) {

  if (!std::strcmp(directory_name, ".")) {
    print_directory(this->navigation.back());
    return;
  }
  if (!std::strcmp(directory_name, "..") && this->navigation.size() == 1)
    throw new Error("no directories to go back.");

  if (!std::strcmp(directory_name, "..")) {
    this->navigation.pop_back();
    Directory actual_directory = this->navigation.back();
    unsigned int directory_inode_block_group = block_group_from_inode(this->superblock, actual_directory.inode);
    this->bgd = read_blocks_group_descriptor(this->image, block_group_descriptor_address(directory_inode_block_group));
    print_directory(this->navigation.back());
    return;
  }

  Directory actual_directory = this->navigation.at(this->navigation.size() - 1);
  Inode *actual_inode = read_inode(this->image, this->bgd, inode_order(this->superblock, actual_directory.inode));
  Directory *directory = search_directory(this->image, actual_inode, directory_name);

  if(!directory) throw new Error("directory not found.");
  if (directory->file_type != 2)  throw new Error("it's not a directory.");

  unsigned int directory_inode_block_group = block_group_from_inode(this->superblock, directory->inode);
  this->bgd = read_blocks_group_descriptor(this->image, block_group_descriptor_address(directory_inode_block_group));

  this->navigation.push_back(*directory);
  print_directory(this->navigation.back());
}