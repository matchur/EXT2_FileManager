#pragma once

#define EXT2_NAME_LEN 255
#include <linux/types.h>
#include "../inode/inode.hpp"
#include <vector>

using namespace std;
/*
  Estrutura do diretório
  Referência:  https://www.science.smith.edu/~nhowe/262/oldlabs/kernel/ext2_fs.h
*/ 
typedef struct directory
{
     uint32_t	inode;			/* Inode number */
     uint16_t	rec_len;		/* Directory entry length */
     uint8_t	name_len;		/* Name length */
     uint8_t	file_type;
     char	name[EXT2_NAME_LEN];	/* File name */
} Directory;

// procura pelo diretório com o nome especificado entre os diretórios contidos dentro do diretório identificado pelo inode
Directory* search_directory(FILE* image, Inode* inode, const char* name);

// realiza a leitura dos diretórios contidos dentro do diretório identificado pelo inode
std::vector<Directory> read_directories(FILE* image, Inode* inode);
void print_directory(Directory directory); // imprime informações úteis do diretório
void print_directories(vector<Directory> directories); // imprime informações úteis do diretório