#include <iostream>
#include <cstdint>
#include <linux/types.h>

#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

/*
  Estrutura do superbloco
  Referência:  https://www.science.smith.edu/~nhowe/262/oldlabs/kernel/ext2_fs.h
*/ 
typedef struct superblock
{
  uint32_t s_inodes_count; /* Inodes count */
  uint32_t s_blocks_count; /* Blocks count */
  uint32_t s_r_blocks_count; /* Reserved blocks count */
  uint32_t s_free_blocks_count; /* Free blocks count */
  uint32_t s_free_inodes_count; /* Free inodes count */
  uint32_t s_first_data_block; /* First Data Block */
  uint32_t s_log_block_size; /* Block size */
  int32_t s_log_frag_size; /* Fragment size */       /*** __s32 ***/
  uint32_t s_blocks_per_group; /* # Blocks per group */
  uint32_t s_frags_per_group; /* # Fragments per group */
  uint32_t s_inodes_per_group; /* # Inodes per group */
  uint32_t s_mtime; /* Mount time */
  uint32_t s_wtime; /* Write time */
  uint16_t s_mnt_count; /* Mount count */
  int16_t	s_max_mnt_count; /* Maximal mount count */ /*** __s16 ***/
  uint16_t s_magic; /* Magic signature */
  uint16_t s_state; /* File system state */
  uint16_t s_errors; /* Behaviour when detecting errors */
  uint16_t s_minor_rev_level; /* minor revision level */
  uint32_t s_lastcheck; /* time of last check */
  uint32_t s_checkinterval; /* max. time between checks */
  uint32_t s_creator_os; /* OS */
  uint32_t s_rev_level; /* Revision level */
  uint16_t s_def_resuid; /* Default uid for reserved blocks */
  uint16_t s_def_resgid; /* Default gid for reserved blocks */
  uint32_t s_first_ino; /* First non-reserved inode */
  uint16_t s_inode_size; /* size of inode structure */
  uint16_t s_block_group_nr; /* block group # of this superblock */
  uint32_t s_feature_compat; /* compatible feature set */
  uint32_t s_feature_incompat; /* incompatible feature set */
  uint32_t s_feature_ro_compat; /* readonly-compatible feature set */
  uint8_t s_uuid[16]; /* 128-bit uuid for volume */
  char s_volume_name[16]; /* volume name */
  char s_last_mounted[64]; /* directory where last mounted */
  uint32_t s_algorithm_usage_bitmap; /* For compression */
  uint8_t s_prealloc_blocks; /* Nr of blocks to try to preallocate*/
  uint8_t s_prealloc_dir_blocks; /* Nr to preallocate for dirs */
  uint16_t s_padding1;
  uint32_t s_reserved[204]; /* Padding to the end of the block */
} Superblock;

void print_superblock(Superblock* superblock); //imprime dados do superbloco
void write_ext2_superblock(Superblock* superblock, FILE* image); // escreve o superbloco na imagem
Superblock* read_ext2_superblock(FILE* image); // lê o superbloco do sistema de arquivos e mapeia para Superblock

#endif