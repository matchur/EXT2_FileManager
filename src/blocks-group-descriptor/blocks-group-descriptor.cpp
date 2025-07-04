#include "blocks-group-descriptor.hpp"

using namespace std;

void print_blocks_group_descriptor(BlocksGroupDescriptor* bgd) {
    cout << "block bitmap:\t\t"    << bgd->bg_block_bitmap << endl;
    cout << "inode bitmap:\t\t"    << bgd->bg_inode_bitmap << endl;
    cout << "inode table:\t\t"     << bgd->bg_inode_table << endl;
    cout << "free blocks count:\t" << bgd->bg_free_blocks_count << endl;
    cout << "free inodes count:\t" << bgd->bg_free_inodes_count << endl;
    cout << "used dirs count:\t"   << bgd->bg_used_dirs_count << endl;
}

BlocksGroupDescriptor *read_blocks_group_descriptor(FILE *image, uint32_t position) {

  BlocksGroupDescriptor *bgd = (BlocksGroupDescriptor *)malloc(sizeof(BlocksGroupDescriptor)); // aloca memória para um bgd
  fseek(image, position, SEEK_SET); //posiciona o ponteiro no offset onde o bgd está localizado
  fread(bgd, 1, sizeof(BlocksGroupDescriptor), image); //copia os bytes do bgd para a estrutura em memória

  return bgd;
}

uint32_t block_group_descriptor_address(int bgd_index) {
    return (unsigned int) 2048 + (sizeof(BlocksGroupDescriptor) * (bgd_index));// localiza onde está localizado o bgd na imagem
}

unsigned int block_group_from_inode(Superblock* superblock, unsigned int inode) {
  unsigned int inodes_per_group = superblock->s_inodes_per_group;
  return ((inode - 1) / inodes_per_group);
}

void write_blocks_group_descriptor(BlocksGroupDescriptor* bgd , FILE *image, uint32_t position) {
    fseek(image, position, SEEK_SET);
    fwrite(bgd, 1, sizeof(BlocksGroupDescriptor), image);
}