#include "inode.hpp"
#include "../utils/utils.hpp"
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

Inode *read_inode(FILE *image, BlocksGroupDescriptor *bgd, unsigned int inode_order) {
  Inode *inode = (Inode *)malloc(sizeof(Inode)); //aloca memória para um Inode

  // localiza a tabela de inodes do grupo e soma à posição relativa do inode para obter o offset do inode na tabela
  int inode_position =  get_block_offset(bgd->bg_inode_table, BASE_OFFSET, BLOCK_SIZE) + ((inode_order - 1) * sizeof(Inode));
  fseek(image, inode_position, SEEK_SET); //posiciona o ponteiro de leitura no offset encontrado
  fread(inode, 1, sizeof(Inode), image); // lê os bytes para a estrutura Inode
  return inode;
}

unsigned int inode_order(Superblock *superblock, uint32_t inode) {
  return (unsigned int)inode % superblock->s_inodes_per_group; //operador módulo devolve a posição relativa do inode no grupo
}

static unsigned int last_position_of_content_on_block(unsigned int bytes_to_read)
{
  return bytes_to_read >= BLOCK_SIZE ? BLOCK_SIZE : bytes_to_read;
}

// imprime conteúdos dos blocos em 'indexes' e informa se existe mais conteúdo a ser lido
bool _print_array_of_blocks(FILE *image, uint32_t *indexes, int qtd_indexes, unsigned int *bytes_to_read, unsigned int *blocks_read, unsigned int total)
{
  char *content = (char *)malloc(sizeof(char) * (BLOCK_SIZE + 1));
  int position;

  bool exit = false;

  for (int i = 0; i < qtd_indexes; i++)
  {
    if ((*bytes_to_read) <= BLOCK_SIZE)
      exit = true;

    position = get_block_offset(indexes[i], BASE_OFFSET, BLOCK_SIZE);
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

// imprime o conteúdo dos blocos de dados do inode
void print_inode_blocks_content(FILE *image, Inode *inode)
{
  unsigned int bytes_to_read = inode->i_size;
  unsigned int blocks_read = 0;
  bool exit = false;
  int position;

  // Aloca buffers para os níveis indiretos de blocos
  uint32_t *indexes_level_1 = (uint32_t *)malloc(sizeof(uint32_t) * 256);
  uint32_t *indexes_level_2 = (uint32_t *)malloc(sizeof(uint32_t) * 256);
  uint32_t *indexes_level_3 = (uint32_t *)malloc(sizeof(uint32_t) * 256);

  // Primeiro processa os blocos diretos (12 primeiros)
  if (!_print_array_of_blocks(image, inode->i_block, 12, &bytes_to_read, &blocks_read, inode->i_size))
    return;

   // Processa blocos indiretos simples (nível 1)
  position = get_block_offset((inode->i_block)[12], BASE_OFFSET, BLOCK_SIZE);
  fseek(image, position, SEEK_SET);
  fread(indexes_level_1, 1, BLOCK_SIZE, image);
  if (!_print_array_of_blocks(image, indexes_level_1, 256, &bytes_to_read, &blocks_read, inode->i_size))
    return;

  // Processa blocos indiretos duplos (nível 2)
  position = get_block_offset((inode->i_block)[13], BASE_OFFSET, BLOCK_SIZE);
  fseek(image, position, SEEK_SET);
  fread(indexes_level_2, 1, BLOCK_SIZE, image);

  for (int i = 0; i < 256; i++)
  {
    position = get_block_offset(indexes_level_2[i], BASE_OFFSET, BLOCK_SIZE);
    fseek(image, position, SEEK_SET);
    fread(indexes_level_1, 1, BLOCK_SIZE, image);
    if (!_print_array_of_blocks(image, indexes_level_1, 256, &bytes_to_read, &blocks_read, inode->i_size))
      return;
  }

  // Processa blocos indiretos triplos (nível 3) 
  position = get_block_offset((inode->i_block)[14], BASE_OFFSET, BLOCK_SIZE);
  fseek(image, position, SEEK_SET);
  fread(indexes_level_3, 1, BLOCK_SIZE, image);

  for (int i = 0; i < 256; i++)
  {
    position = get_block_offset(indexes_level_3[i], BASE_OFFSET, BLOCK_SIZE);
    fseek(image, position, SEEK_SET);
    fread(indexes_level_2, 1, BLOCK_SIZE, image);

    for (int j = 0; j < 256; j++)
    {
      position = get_block_offset(indexes_level_2[j], BASE_OFFSET, BLOCK_SIZE);
      fseek(image, position, SEEK_SET);
      fread(indexes_level_1, 1, BLOCK_SIZE, image);
      if (!_print_array_of_blocks(image, indexes_level_1, 256, &bytes_to_read, &blocks_read, inode->i_size))
        return;
    }
  }
}

string get_i_mode_permissions(uint32_t i_mode) {
  string str;

  if (i_mode & (0x4000))
  str = str.append("d"); // Diretório
  else if (i_mode & (0x8000))
    str = str.append("f"); // Arquivo regular
  else
    str = str.append("-"); // outro tipo

  // Permissões do user
  str += (i_mode & 0x0100) ? "r" : "-";
  str += (i_mode & 0x0080) ? "w" : "-";
  str += (i_mode & 0x0040) ? "x" : "-";

  // Permissões do grupo
  str += (i_mode & 0x0020) ? "r" : "-";
  str += (i_mode & 0x0010) ? "w" : "-";
  str += (i_mode & 0x0008) ? "x" : "-";

  // Permissões de outros
  str += (i_mode & 0x0004) ? "r" : "-";
  str += (i_mode & 0x0002) ? "w" : "-";
  str += (i_mode & 0x0001) ? "x" : "-";

  return str;
}
