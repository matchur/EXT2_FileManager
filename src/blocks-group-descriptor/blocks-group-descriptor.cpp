#include "blocks-group-descriptor.hpp"

using namespace std;

// Imprime informações do descritor de grupo de blocos EXT2
void print_blocks_group_descriptor(BlocksGroupDescriptor* bgd) {
    cout << "block bitmap:\t\t"    << bgd->bg_block_bitmap << endl;
    cout << "inode bitmap:\t\t"    << bgd->bg_inode_bitmap << endl;
    cout << "inode table:\t\t"     << bgd->bg_inode_table << endl;
    cout << "free blocks count:\t" << bgd->bg_free_blocks_count << endl;
    cout << "free inodes count:\t" << bgd->bg_free_inodes_count << endl;
    cout << "used dirs count:\t"   << bgd->bg_used_dirs_count << endl;
}

// Lê um descritor de grupo de blocos da imagem EXT2 e retorna um ponteiro alocado dinamicamente
// O ponteiro retornado deve ser liberado com free após o uso
BlocksGroupDescriptor *read_blocks_group_descriptor(FILE *image, uint32_t position) {
    BlocksGroupDescriptor *bgd = (BlocksGroupDescriptor *)malloc(sizeof(BlocksGroupDescriptor));
    fseek(image, position, SEEK_SET);
    fread(bgd, 1, sizeof(BlocksGroupDescriptor), image);
    return bgd;
}

// Retorna o endereço do descritor de grupo de blocos de acordo com o índice
uint32_t block_group_descriptor_address(int bgd_index) {
    return (unsigned int) 2048 + (sizeof(BlocksGroupDescriptor) * (bgd_index));
}

// Calcula o grupo de blocos ao qual pertence um determinado inode
unsigned int block_group_from_inode(Superblock* superblock, unsigned int inode) {
    unsigned int inodes_per_group = superblock->s_inodes_per_group;
    return ((inode - 1) / inodes_per_group);
}

// Escreve um descritor de grupo de blocos na imagem EXT2 na posição especificada
void write_blocks_group_descriptor(BlocksGroupDescriptor* bgd , FILE *image, uint32_t position) {
    fseek(image, position, SEEK_SET);
    fwrite(bgd, 1, sizeof(BlocksGroupDescriptor), image);
}