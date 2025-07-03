#include "superblock.hpp"
#include "../utils/utils.hpp"

using namespace std;

Superblock *read_ext2_superblock(FILE *image){

    Superblock *superblock = (Superblock *)malloc(sizeof(Superblock)); // aloca memória para armazenar o superbloco 
    fseek(image, 1024, SEEK_SET); // Posiciona o ponteiro de leitura no byte 1024 do arquivo (onde começa o superbloco)
    fread(superblock, 1, sizeof(Superblock), image); //Lê os bytes do superbloco do arquivo e os armazena na struct alocada

    return superblock;
}

void print_superblock(Superblock *superblock)
{
    cout << "inodes count:  " << (unsigned)superblock->s_inodes_count << endl;
    cout << "blocks count:  " << (unsigned)superblock->s_blocks_count << endl;
    cout << "reserved blocks count:  " << (unsigned)superblock->s_r_blocks_count << endl;
    cout << "free blocks count:  " << (unsigned)superblock->s_free_blocks_count << endl;
    cout << "free inodes count:  " << (unsigned)superblock->s_free_inodes_count << endl;
    cout << "first data block:  " << (unsigned)superblock->s_first_data_block << endl;
    cout << "block size:  " << (unsigned)superblock->s_log_block_size << endl;
    cout << "frag size:  " << superblock->s_log_frag_size << endl;
    cout << "blocks per group:  " << (unsigned)superblock->s_blocks_per_group << endl;
    cout << "frags per group:  " << (unsigned)superblock->s_frags_per_group << endl;
    cout << "inodes per group:  " << (unsigned)superblock->s_inodes_per_group << endl;
    cout << "mount time:  " << (unsigned)superblock->s_mtime << endl;
    cout << "write time:  " << (unsigned)superblock->s_wtime << endl;
    cout << "mount count:  " << (unsigned)superblock->s_mnt_count << endl;
    cout << "max mount count:  " << (unsigned)superblock->s_max_mnt_count << endl;
    cout << "magic signature:  " << std::hex << std::uppercase << superblock->s_magic << std::dec << endl;
    cout << "file system state:  " << (unsigned)superblock->s_state << endl;
    cout << "errors:  " << (unsigned)superblock->s_errors << endl;
    cout << "minor revision level:  " << (unsigned)superblock->s_minor_rev_level << endl;
    cout << "time of last check:  ";
    print_time((unsigned)superblock->s_lastcheck);
    cout << "max check interval:  " << (unsigned)superblock->s_checkinterval << endl;
    cout << "creator OS:  " << (unsigned)superblock->s_creator_os << endl;
    cout << "revision level:  " << (unsigned)superblock->s_rev_level << endl;
    cout << "default uid reserved blocks:  " << (unsigned)superblock->s_def_resuid << endl;
    cout << "default gid reserved blocks:  " << (unsigned)superblock->s_def_resgid << endl;
    cout << "first non-reserved inode:  " << (unsigned)superblock->s_first_ino << endl;
    cout << "inode size:  " << (unsigned)superblock->s_inode_size << endl;
    cout << "block group number:  " << (unsigned)superblock->s_block_group_nr << endl;
    cout << "compatible feature set:  " << (unsigned)superblock->s_feature_compat << endl;
    cout << "incompatible feature set:  " << (unsigned)superblock->s_feature_incompat << endl;
    cout << "read only compatible feature set:  " << (unsigned)superblock->s_feature_ro_compat << endl;
    cout << "volume UUID:  " << superblock->s_uuid << endl;
    cout << "volume name:  " << superblock->s_volume_name << endl;
    cout << "volume last mounted:  " << superblock->s_last_mounted << endl;
    cout << "algorithm usage bitmap:  " << (unsigned)superblock->s_algorithm_usage_bitmap << endl;
    cout << "blocks to try to preallocate:  " << (unsigned)superblock->s_prealloc_blocks << endl;
    cout << "blocks preallocate dir:  " << (unsigned)superblock->s_prealloc_dir_blocks << endl;
}

