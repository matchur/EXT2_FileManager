#pragma once

#include "../superblock/superblock.hpp"
#include "../blocks-group-descriptor/blocks-group-descriptor.hpp"
#include "../directory/directory.hpp"

class FileSystemManager {
private:
  FILE *image = NULL;
  BlocksGroupDescriptor *bgd = NULL;
  Superblock *superblock = NULL;
  vector<Directory> navigation;

public:
  FileSystemManager(FILE *image); //construtor
  void info(); // imprime dados do sistema de arquivos
  void superblock_info(); //imprime dados do superbloco
  void blocks_group_descriptor_info(int index); // imprime dados do descritor de grupo de índice especificado
  void inode_info(unsigned int inode); // imprime dados do inode
  void cat(const char *directory_name); //exibe o conteúdo de um arquivo no formato texto
};