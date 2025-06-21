#pragma once

#include "../superblock/superblock.hpp"

class FileSystemManager {
private:
  FILE *image = NULL;
  Superblock *superblock = NULL;

public:
  FileSystemManager(FILE *image); //construtor
  void info(); // imprime dados do sistema de arquivos
  void superblock_info(); //imprime dados do superbloco
};