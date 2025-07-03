#include "directory.hpp"
#include <sys/stat.h>
#include <cstring>
#include "../utils/utils.hpp"
#define BLOCK_SIZE 1024
#define BASE_OFFSET 1024

typedef char Block[BLOCK_SIZE];

// Lê e retorna todas as entradas de diretório contidas em um inode de diretório
vector<Directory> read_directories(FILE* image, Inode* inode) {

    vector<Directory> directories;
  
    if(S_ISDIR(inode->i_mode)) {
  
      for(int i = 0; i < 12; i++){
        Block* block = (Block*)malloc(sizeof(Block));
  
        if(!inode->i_block[i]) break;
  
        fseek(image, get_block_offset(inode->i_block[i], BASE_OFFSET, BLOCK_SIZE), SEEK_SET);
        fread(block, 1, sizeof(Block), image);
        
        Directory* worked_directory = (Directory *) block;
        int block_posiion = 0;

        // Processa cada entrada de diretório no bloco
        do{
          Directory* directory = (Directory*) malloc(sizeof(Directory));
          memcpy(directory, worked_directory, sizeof(Directory));
          (directory->name)[directory->name_len] = '\0';
          directories.push_back(*directory);
          worked_directory = (Directory *) ((char*) worked_directory + worked_directory->rec_len);
          block_posiion += worked_directory->rec_len;
        }while((block_posiion < inode->i_size) && worked_directory->inode);
  
        free(block);
      }
    }
  
    return directories;
}

// Busca por uma entrada específica em um diretório
Directory* search_directory(FILE* image, Inode* inode, const char* name){
    Directory* directory = NULL;
  
    vector<Directory> directories = read_directories(image, inode); // Obtém todas as entradas do diretório
    
    // Itera pelas entradas procurando pelo nome
    for(vector<Directory>::iterator i = directories.begin(); i != directories.end(); i++){
      const char* iterator_directory_name = (const char*) (*i).name;
      if(!strcmp(name, iterator_directory_name)){
        directory = &(*i);
        break;
      }
    }
  
    return directory;
}

void print_directory(Directory directory){
  cout <<  directory.name << endl;
  cout << "inode:\t\t\t" << (unsigned) directory.inode << endl;
  cout << "record lenght:\t\t" << (unsigned) directory.rec_len << endl;
  cout << "name lenght:\t\t" << (unsigned) directory.name_len << endl;
  cout << "file type:\t\t" << (unsigned) directory.file_type << endl;
}

void print_directories(vector<Directory> directories) {
  for(vector<Directory>::iterator i = directories.begin(); i != directories.end(); i++){
    print_directory(*i);
    cout << endl;
  }
}