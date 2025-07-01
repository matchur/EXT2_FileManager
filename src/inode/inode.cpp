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

/* retorna a ultima posição da porção de bytes para ler (primeira posição não utilizado com dado util) */
static unsigned int last_position_of_content_on_block(unsigned int bytes_to_read)
{
  return bytes_to_read >= BLOCK_SIZE ? BLOCK_SIZE : bytes_to_read;
}

/* realiza a impressão dos conteudos dos blocos em 'indexes' e informar se existe mais conteudo a ser lido */
bool _print_array_of_blocks(FILE *image, uint32_t *indexes, int qtd_indexes, unsigned int *bytes_to_read, unsigned int *blocks_read, unsigned int total)
{
  char *content = (char *)malloc(sizeof(char) * (BLOCK_SIZE + 1));
  int position;

  bool exit = false;

  for (int i = 0; i < qtd_indexes; i++)
  {
    if ((*bytes_to_read) <= BLOCK_SIZE)
      exit = true;

    position = get_block_offset(indexes[i]);
    fseek(image, position, SEEK_SET);
    fread(content, 1, BLOCK_SIZE, image);

    content[last_position_of_content_on_block(*bytes_to_read)] = '\0';

    cout << content;

    if (exit) return false;
    (*bytes_to_read) -= 1024;
    (*blocks_read)++;
  }

  return true;
}

/* realiza a impressão do conteudo dos blocos de dados do 'inode' */
void print_inode_blocks_content(FILE *image, Inode *inode)
{
  unsigned int bytes_to_read = inode->i_size;
  unsigned int blocks_read = 0;
  bool exit = false;
  int position;

  uint32_t *indexes_level_1 = (uint32_t *)malloc(sizeof(uint32_t) * 256);
  uint32_t *indexes_level_2 = (uint32_t *)malloc(sizeof(uint32_t) * 256);
  uint32_t *indexes_level_3 = (uint32_t *)malloc(sizeof(uint32_t) * 256);

  /* impressão niveis de acesso direto */
  if (!_print_array_of_blocks(image, inode->i_block, 12, &bytes_to_read, &blocks_read, inode->i_size))
    return;

  /* impressão niveis simples de indexes */
  position = get_block_offset((inode->i_block)[12]);
  fseek(image, position, SEEK_SET);
  fread(indexes_level_1, 1, BLOCK_SIZE, image);
  if (!_print_array_of_blocks(image, indexes_level_1, 256, &bytes_to_read, &blocks_read, inode->i_size))
    return;

  /* impressão niveis duplos de indexes */
  position = get_block_offset((inode->i_block)[13]);
  fseek(image, position, SEEK_SET);
  fread(indexes_level_2, 1, BLOCK_SIZE, image);

  for (int i = 0; i < 256; i++)
  {
    position = get_block_offset(indexes_level_2[i]);
    fseek(image, position, SEEK_SET);
    fread(indexes_level_1, 1, BLOCK_SIZE, image);
    if (!_print_array_of_blocks(image, indexes_level_1, 256, &bytes_to_read, &blocks_read, inode->i_size))
      return;
  }

  /* impressão niveis triplos de indexes */
  position = get_block_offset((inode->i_block)[14]);
  fseek(image, position, SEEK_SET);
  fread(indexes_level_3, 1, BLOCK_SIZE, image);

  for (int i = 0; i < 256; i++)
  {
    position = get_block_offset(indexes_level_3[i]);
    fseek(image, position, SEEK_SET);
    fread(indexes_level_2, 1, BLOCK_SIZE, image);

    for (int j = 0; j < 256; j++)
    {
      position = get_block_offset(indexes_level_2[j]);
      fseek(image, position, SEEK_SET);
      fread(indexes_level_1, 1, BLOCK_SIZE, image);
      if (!_print_array_of_blocks(image, indexes_level_1, 256, &bytes_to_read, &blocks_read, inode->i_size))
        return;
    }
  }
}

string get_i_mode_permissions(uint32_t i_mode) {
  string str;

  switch (i_mode & 0xF000) {
      case 0x4000: str += "d"; break; // directory
      case 0x8000: str += "f"; break; // regular file
      default:     str += "-"; break;
  }

  // user
  str += (i_mode & 0x0100) ? "r" : "-";
  str += (i_mode & 0x0080) ? "w" : "-";
  str += (i_mode & 0x0040) ? "x" : "-";

  // group
  str += (i_mode & 0x0020) ? "r" : "-";
  str += (i_mode & 0x0010) ? "w" : "-";
  str += (i_mode & 0x0008) ? "x" : "-";

  // other
  str += (i_mode & 0x0004) ? "r" : "-";
  str += (i_mode & 0x0002) ? "w" : "-";
  str += (i_mode & 0x0001) ? "x" : "-";

  return str;
}
