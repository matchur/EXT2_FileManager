#pragma once

#include "../superblock/superblock.hpp"
#include <linux/types.h>

/*
  Estrutura do blocks group descriptor
  Referência:  https://www.science.smith.edu/~nhowe/262/oldlabs/kernel/ext2_fs.h
*/ 
typedef struct blocks_group_descriptor {
    uint32_t	bg_block_bitmap;		/* Blocks bitmap block */
    uint32_t	bg_inode_bitmap;		/* Inodes bitmap block */
    uint32_t	bg_inode_table;		/* Inodes table block */
    uint16_t	bg_free_blocks_count;	/* Free blocks count */
    uint16_t	bg_free_inodes_count;	/* Free inodes count */
    uint16_t	bg_used_dirs_count;	/* Directories count */
    uint16_t	bg_pad;
    uint32_t	bg_reserved[3];
} BlocksGroupDescriptor;

BlocksGroupDescriptor *read_blocks_group_descriptor(FILE *image, uint32_t position); // lê e retorna um descritor de grupo da imagem
void print_blocks_group_descriptor(BlocksGroupDescriptor* bgd); // imprime dados do descritor de grupo 
uint32_t block_group_descriptor_address(int bgd_index); // retorna o endereço do descritor de grupo na imagem
unsigned int block_group_from_inode(Superblock* superblock, unsigned int inode); // retorna o índice do descritor de grupo ao qual o 'inode' pertence
void write_blocks_group_descriptor(BlocksGroupDescriptor* bgd , FILE *image, uint32_t position); //escreve o descritor de grupo na imagem em uma posição posição