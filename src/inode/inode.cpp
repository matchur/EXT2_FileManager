#include "inode.hpp"
#define BLOCK_SIZE 1024
#define BASE_OFFSET 1024

using namespace std;

void print_array(uint32_t *array, int size)
{
  for (int i = 0; i < size; i++)
    cout << "pointer[" << i << "]:  " << array[i] << endl;
}

void print_inode(Inode *inode) {
    cout << "file format and access rights:  " << "0x" << std::hex << (unsigned)inode->i_mode << std::dec << endl;
    cout << "user_id:  " << (unsigned)inode->i_uid << endl;
    cout << "lower 32-bit file size:  " << (unsigned)inode->i_size << endl;
    cout << "access time:   " << (unsigned)inode->i_atime << endl;
    cout << "creation time:  " << (unsigned)inode->i_ctime << endl;
    cout << "modification time:  " << (unsigned)inode->i_mtime << endl;
    cout << "deletion time:  " << (unsigned)inode->i_dtime << endl;
    cout << "group id:  " << (unsigned)inode->i_gid << endl;
    cout << "link count inode:  " << (unsigned)inode->i_links_count << endl;
    cout << "512-bytes blocks:  " << (unsigned)inode->i_blocks << endl;
    cout << "ext2_flags:  " << (unsigned)inode->i_flags << endl;
    cout << "reserved (Linux):  " << (unsigned)inode->osd1 << endl;
    print_array(inode->i_block, 15);
    cout << "file version (nfs):  " << (unsigned)inode->i_generation << endl;
    cout << "block number extended attributes:  " << (unsigned)inode->i_file_acl << endl;
    cout << "higher 32-bit file size:  " << (unsigned)inode->i_dir_acl << endl;
    cout << "location file fragment:  " << (unsigned)inode->i_faddr << endl;
  }

int get_block_offset(uint32_t block){
  return BASE_OFFSET + (block - 1) * BLOCK_SIZE;
}

Inode *read_inode(FILE *image, BlocksGroupDescriptor *bgd, unsigned int inode_order) {
  Inode *inode = (Inode *)malloc(sizeof(Inode));
  int inode_position =  get_block_offset(bgd->bg_inode_table) + ((inode_order - 1) * sizeof(Inode));
  fseek(image, inode_position, SEEK_SET);
  fread(inode, 1, sizeof(Inode), image);
  return inode;
}

unsigned int inode_order(Superblock *superblock, uint32_t inode) {
  return (unsigned int)inode % superblock->s_inodes_per_group;
}